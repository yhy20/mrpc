#include <poll.h>
#include <sstream>

#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"

namespace mrpc
{
namespace net
{
/**
 * #define POLLIN  0x001    There is data to read.
 * #define POLLPRI 0x002    There is urgent data to read.
 * #define POLLOUT 0x004    Writing now will not block.
 */
const int Channel::kNoneEvent = 0;                  // 表示不监听任何事件或用于清除已监听的事件
const int Channel::kReadEvent = POLLIN | POLLPRI;   // 表示监听可读事件，包括普通数据到达或紧急数据到达
const int Channel::kWriteEvent = POLLOUT;           // 表示监听可写事件，表示 fd 写缓冲区空闲，此时写入数据不会阻塞

Channel::Channel(EventLoop* loop, int fd)
    : m_loop(loop),
      m_fd(fd),
      m_events(0),
      m_revents(0),
      m_index(-1),
      m_logHup(true),
      m_tied(false),
      m_eventHandling(false),
      m_addedToLoop(false) { }

Channel::~Channel()
{
    assert(!m_eventHandling);
    assert(!m_addedToLoop);
    if (m_loop->isInLoopThread())
    {
        assert(!m_loop->hasChannel(this));
    }
}

/// 详解见 handleEvent 函数说明
void Channel::tie(const std::shared_ptr<void>& obj)
{
    m_tie = obj;
    m_tied = true;
}

/// not thread safe but only in loop.
void Channel::update()
{
    m_addedToLoop = true;
    m_loop->updateChannel(this);

}

/// not thread safe but only in loop.
void Channel::remove()
{
    assert(isNoneEvent());
    m_addedToLoop = false;
    m_loop->removeChannel(this);
}

/**
 * Channel::tie() 详解，此处是一个智能指针使用的特定场景之一。
 * 当对端 TCP 连接断开时会触发 Channel::handleEvent() 调用，而
 * handleEvent 中会调用用户提供的 CloseCallback，而用户的代码在
 * CloseCallback 中可能会析构 Channel 对象，此时会造成 Channel::
 * handleEvent() 执行到一半的时候，其所属的Channel对象本身被销毁
 * 了，这时程序会 core dump. 而 tie 就是用于延长某些对象的生命周
 * 期，可以是 Channel 对象，也可以是其 owner 对象。
 * 
 * 实际上通过我的思考发现，目前 TcpServer 和 TcpConnection 的实现
 * 中不存在 TcpConnection 在其他线程意外析构导致正在执行的 Channel
 * ::handleEvent 函数所属 Channel 也被析构，最终导致错误的情况，此
 * 处调用 tie 是冗余的。
 */
void Channel::handleEvent(TimeStamp receiveTime)
{
    std::shared_ptr<void> guard;
    if(m_tied)
    {
        /// 延长特定对象的生命周期，此处是 Channel 的 owner TcpConnection
        guard = m_tie.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}
/**
 * 详解 epoll 事件，关于所有事件的触发条件测试代码，详解项目的 src/learn/iomultiplexing/epoll
 * 
 * 下面是既可用于 epoll_ctl 注册也会被 epoll_wait 作为 revents 返回的事件    
 * 
 * EPOLLIN: The associated file is available for read(2) operations.
 *          读事件，通常意味着 fd 的内核读缓冲区中有数据，此时调用 read 不会陷入阻塞状态
 * 
 * EPOLLPRI: There is urgent data available for read(2) operations.
 *           读事件，但表示紧急数据，例如 tcp socket 的带外数据
 * 
 * EPOLLOUT: The associated file is available for write(2) operations.
 *           写事件，通常意味着 fd 的内核写缓冲区未满，此时调用 write 不会陷入阻塞状态
 * 
 * EPOLLRDHUP: (since Linux 2.6.17) Stream socket peer closed connection, 
 *             or shut down writing half of connection. This flag is especially
 *             useful for writing simple code to detect peer shutdown 
 *             when using Edge Triggered monitoring.
 *             Stream socket 的一端关闭了连接（注意是 stream socket，我们知道还有
 *             raw socket，dgram socket）
 *             PS: 在水平触发模式下要监听对方是否关闭套接字，只需要监听 EPOLLIN 事件
 *             并调用 read 返回 0 即可，但在边缘触发模式下，如果不及时处理对方关闭
 *             套接字事件可能会导致永远丢失处理该事件。在边缘触发模式下使用 EPOLLRDHUP
 *             可以保证 epoll_wait 会向水平触发一样，总是返回 EPOLLRDHUP 事件就绪，
 *             可以方便的进行延迟处理，而不会导致事件处理的丢失

 * EPOLLONESHOT: (since Linux 2.6.2) sets the one-shot behavior for the associated file descriptor. 
 *               This means that after an event is pulled out with epoll_wait(2) the associated file 
 *               descriptor is internally disabled and no other events will be reported by the epoll interface. 
 *               一次性事件，在触发一次后就会失效，需要修改重新注册才能再次生效，通常用于和 EPOLLOUT 写事件一同
 *               使用，在水平触发模式下，fd 内核缓冲区不满，epoll_wait 会始终返回 EPOLLOUT 事件就绪，大量的积累
 *               会导致 busy-loop，白白浪费 CPU 时间片资源
 * 
 * EPOLLET: Sets the Edge Triggered behavior for the associated file descriptor.The default behavior
 *          for epoll is Level Triggered. See epoll(7) for more detailed information about Edge and 
 *          Level Triggered event distribution architectures.
 * 
 * 下面是仅会被 epoll_wait 作为 revents 返回的事件
 * 
 * EPOLLERR: Error condition happened on the associated file descriptor.
 *           epoll_wait(2) will always wait for this event; it is not 
 *           necessary to set it in events.
 *           仅用于内核设置返回事件 revents，表示发生错误。只有采取动作时，才能知道是否对方异常。即对方突然断掉，
 *           是不会也不可能会主动触发 EPOLLERR 事件的。只有服务器采取动作（当然服务器此刻也不知道发生异常）read 
 *           or write 发生错误时，会触发 EPOLLERR 事件，说明对方已经异常断开。比如对一个已经关闭的 socket 进行
 *           读写操作时，也会触发 EPOLLERR 错误。此处再次强调只有在采取行动（比如读一个已经关闭的 socket，或者
 *           写一个已经关闭的 socket）时候，才知道对方是否关闭了，才会触发 EPOLLERR。正常的处理方式即把对方 DEL
 *           掉，再 close 即可。
 *           PS: 当客户端的机器在发送“请求”前，就崩溃了（或者网络断掉了），则服务器一端是无从知晓的。发生这种情
 *           况与是否使用 epoll 无关，必须要服务器主动的去检查才能发现并解决问题。而服务器显然不可能经常的向客户
 *           写数据，通过有没有发生 EPOLLERR 事件来确认客户端是否有问题，因此服务器端的超时检查很重要。即使没有上
 *           述情况发生也应当做超时检查并主动断开连接，比如利用客户端恶意连接（只连接不发送数据）来攻击服务器，恶意
 *           占用服务器资源致使服务器崩溃。
 *              
 * EPOLLHUP: Hang up happened on the associated file descriptor. epoll_wait(2) will always wait
 *           for this event; it is not necessary to set it in events.
 *           目前我所知的
 * 
 * epoll 事件触发情况分析:
 * (1) 有新连接请求到达或对端发送普通数据会触发 EPOLLIN 事件
 * (2) 带外数据，只触发 EPOLLPRI 事件
 * (3) 对端正常断开（程序里 close(fd), shutdown(fd, SHUT_WR), shell 下 kill 或 ctr + c），通常意味着 FIN 到达，
 *     触发 EPOLLIN 和 EPOLLRDHUP（EPOLLRDHUP 必须通过 epoll_ctl 注册过才会触发），但是不触发 EPOLLERR 
 *     和 EPOLLHUP 事件。对于后两种事件，通常在本端（server 端）出错后才触发。
 *     PS: 对端 shutdown(fd, SHUT_RD) 或者本端 shutdown(fd, SHUT_WR)，在本端不会触发任何事件，
 * (4) 对端异常断开（例如拔掉网线），不会主动触发任何事件。其实很好理解，断网后传递不了任何信息，服务器
 *     在主动发起数据传输之前会认为客户端连接正常，只是没有传递任何数据而已，这种情况类似于恶意连接占用服务器
 *     资源的客户端。对于这种情况，服务器的稳定性依赖于服务器的定时器处理，对于超过一定时间没有发送任何数据的
 *     客户端，服务器应当主动关闭连接。
 * 
 * TODO:详解 poll 事件
 * TODO: https://blog.csdn.net/halfclear/article/details/78061771 情况测试（进行了一半）
 * TODO: https://blog.csdn.net/Dontla/article/details/124131873 内容分析
 * 
 * 
 *  On Linux, the constants of poll(2) and epoll(4) are expected to be same.
 *  static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
 *  static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
 *  static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
 *  static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
 *  static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
 *  static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");
 */
void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    /// 开始事件处理
    m_eventHandling = true;

    LOG_TRACE << reventsToString();
    /**
     * 关于 POLLHUP(EPOLLHUP) 事件的触发情况讨论
     * (1) 客户端 polling 在未 connect 成功的 socketFd 上监听
     * (2) 服务器在未 listening 的 socketFd 上监听 
     * (3) TCP 通信的一端同时调用了 shutdown(SHUT_RD) 和 shutdown(SHUT_WR)
     * (4) 通信的一端调用 shutdown(SHUT_RD) 函数发送 FIN 报文给另一端，另一端
     *     正常回复 ACK 报文，并在所有数据发送完毕后继续发送 FIN 报文完全挂断通
     *     信，此时会触发组合事件（POLLIN | POLLHUP) 如果 poller 上注册了事件
     *     POLLRDHUP，则会触发组合事件（POLLIN | POLLRDHUP | POLLHUP)
     * 
     * PS: close 引用计数为 1 的 socketFd 不会触发 POLLHUP，这是由于 close 会
     *     导致自动在 epoll 中清除 socketFd（TODO: 行为待测试）
     * 
     * 在网络库中，非阻塞 connect 连接失败会明确触发 POLLHUP 事件，此时会记录
     * LOG_WARN 信息，但通常没有设置 m_closeCallback 回调，因为连接还尚未建立
     */
    if((m_revents & POLLHUP) && !(m_revents & POLLIN))
    {
        /// 是否在发生 POLLHUP 事件时打印信息
        if(m_logHup)
        {
            LOG_WARN << "fd = " << m_fd << " Channel::handle_event() POLLHUP";
        }
        /// 调用 closeCallback 关闭连接
        if(m_closeCallback) m_closeCallback();
    }

    /// 发生专属于 poll 系统调用的错误事件
    if(m_revents & POLLNVAL)
    {
        LOG_WARN <<"fd = " << m_fd << " Channel::handle_event() POLLNVAL";
    }

    /// 发生 epoll 的 EPOLLER 事件或 poll 的 POLLNVAL 事件调用 ErrorCallback 回调
    if(m_revents & (POLLERR | POLLNVAL))
    {
        if(m_errorCallback) m_errorCallback();
    }

    /// 处理读事件（包括对端正常退出）
    if(m_revents & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if(m_readCallbcak) m_readCallbcak(receiveTime);
    }

    /// 处理写事件，发送 outputBuffer 中的数据
    if(m_revents & POLLOUT)
    {
        if(m_writeCallback)
        {
            m_writeCallback();
        }
         
    }
    
    /// 结束事件处理
    m_eventHandling = false;
}

/// For TRACE and DEBUG
std::string Channel::eventsToString() const
{
    return EventsToString(m_fd, m_events);
}

/// For TRACE、DEBUG
std::string Channel::reventsToString() const
{
    return EventsToString(m_fd, m_revents);
}

/// For TRACE and DEBUG
std::string Channel::EventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ":";
    if(ev & POLLIN)
        oss << " IN";
    if(ev & POLLPRI)
        oss << " PRI";
    if(ev & POLLOUT)
        oss << " OUT";
    if(ev & POLLHUP)
        oss << " HUP";
    if(ev & POLLRDHUP)
        oss << " RDHUP";
    if(ev & POLLERR)
        oss << " ERR";
    if(ev & POLLNVAL)
        oss << " NVAL";

    return oss.str();
}

}  // namespace net
}  // namespace mrpc