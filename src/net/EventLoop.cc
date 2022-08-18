#include <signal.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <algorithm>

#include "Mutex.h"
#include "Poller.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TimerQueue.h"
#include "SocketsOps.h"

namespace
{

/// 当前线程的 Loop 循环，一个线程仅能创建一个 EventLoop 对象
thread_local mrpc::net::EventLoop* t_loopInThisThread = nullptr;

/// Poll 或者 Epoll 的调用超时时间（单位毫秒）
const int kPollTimeMs = 10 * 1000;

/**
 * @brief 创建一个 eventfd，用于 weakup Poller
 */
int CreateEventFd()
{
    int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(-1 == eventFd)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return eventFd;
}

/// ::signal 函数使用了 old-style-cast
#pragma GCC diagnostic ignored "-Wold-style-cast"
/**
 * socket 网络编程中 SIGPIPE 信号处理详解
 * 
 * 一、SIGPIPE 信号产生原因以及为什么需要处理该信号
 * (1) 产生原因：当程序向一个接收了 RST 的套接字执行写操作时，会触发 SIGPIPE 信号
 * (2) 处理原因：SIGPIPE 信号的默认行为是终止进程，会导致整个服务器意外退出。
 * 
 * 二、导致 TCP 发送 RST 报文的原因
 * (1) 发送 SYN 报文时指定的目的端口没有接收进程监听
 *     这种情况下常见的例子是客户端访问服务器未监听的端口，服务器回复 RST 报文
 *     比如，访问 Web 服务器的 21 端口（FTP），如果该端口服务器未开放或者阻断
 *     了到该端口的请求报文，则服务器很可能会给客户端 SYN 报文回应一个 RST 报文。
 *     因此，服务器对终端的 SYN 报文响应 RST 报文在很多时候可以作为判断目标端口
 *     是否开放的一个可靠依据。当然，在大多数场景下，服务器对到达自身未监听端口
 *     的报文进行丢弃而不响应是一种更为安全的实现。
 * 
 * (2) 客户端尝试连接服务端的一个端口，其处于 TIME_WAIT 状态时，服务端会向客户端发 RST
 * 
 * (3) TCP 的一端主动发送 RST, 丢弃发送缓冲区数据，异常终止连接
 *     正常情况下结束一个已有 TCP 连接的方式是发送 FIN，FIN 报文会在所有排队数据都
 *     发出后才会发送，正常情况下不会有数据丢失，因此这也被称为是有序释放。另外一种
 *     拆除已有 TCP 连接的方式就是发送 RST，这种方式的优点在于无需等待数据传输完毕，
 *     可以立即终结连接，这种通过 RST 结束连接的方式被称为异常释放。
 * (4) 向特殊的半连接状态套接字发送数据会收到回复的 RST 报文（此处指的不是 shutdown 调用导致的半关闭状态）
 *     正常情况下 TCP 通过四元组标识一个已经创建的连接，当服务器或客户端收到一个新四
 *     元组（服务器或客户端本地没有这个连接）的非 SYN 首包就会丢弃该报文并回复一个 RST 报文
 *     举个例子：位于不同机器上的用户端和服务器在正常连接的情况下，突然拔掉网线，再重
 *     启服务端。在这个过程中客户端感受不到服务端的异常，还保持着连接（此时就是半连接
 *     状态）客户端向该连接写数据，会收到服务端回复的 RST 报文，如果客户端再收到 RST
 *     包文后继续向该连接写数据，会触发 SIGPIPE 信号，默认情况下会导致客户端异常退出
 * 
 * 三、在网络库中可能触发 SIGPIPE 信号的情况
 *     假设服务器繁忙，没有及时处理对方断开连接的事件，就有可能出现在连接断开后继续发
 *     送数据的情况，下面的例子模拟了这种情况。
 *     void onConnection(const TcpConnectionPtr& conn)
 *     {
 *         if(conn->connected())
 *         {
 *             ::sleep(5);
 *             conn->send(message1); // 会收到 RST 报文
 *             conn->send(message2); // 触发 SIGPIPE 信号
 *         }        
 *     }
 * 
 * 思考：close(socketFd) or shutdown(socketFd, SHUT_WR) 会阻塞吗？
 * 首先 TCP 连接的一端关闭连接会发送 FIN 报文，并至少等待另一端的内核
 * 返回 ACK 报文，所以当一直收不到 ACK 报文时这两个函数调用会阻塞吗？
 * 测试设计：在本地电脑和服务器上建立 TCP 连接，客户端 sleep(60), 在
 * 这段时间内拔掉网线，观察 60s 后客户端是否正常退出。
 * 下面是对结果的预测:
 * (1) 客户端正常退出了，使用 netstat 可以观察到客户端处于 FIN_WAIT1 
 *     状态，这说明应用程序正常退出了，由内核接管 close 或 shutdown 
 *     调用来不断重发 FIN 报文。
 * 
 * (2) 客户端没有正常退出，说明在应用程序上发生了阻塞，此时用 netstat
 *     也可以观察到客户端处于 FIN_WAIT1 状态，不过证明在应用程序上发生
 *     了阻塞。
 * 
 * 上述思考只是突发奇想，暂未测试，不过我认为会发生情况 (1)。
 * TODO: 实验测试上述思考
 */

/**
 * @brief SIGPIPE 信号处理类
 */
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

/// 利用 C++ 的全局对象在程序开始运行时忽略 SIGPIPE 信号
IgnoreSigPipe initObj;

}  // namespace

namespace mrpc
{
namespace net
{

/**
 * @brief 获取当前线程的 EventLoop
 */
EventLoop* EventLoop::GetEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

/// 每个线程只能有一个 EventLoop 对象
EventLoop::EventLoop()
    : m_wakeupFd(CreateEventFd()),
      m_iteration(0),
      m_threadId(CurrentThread::Tid()),
      m_currentActiveChannel(nullptr),
      m_quit(false),
      m_looping(false),
      m_eventHandling(false),
      m_callingPendingFunctors(false),
      m_poller(Poller::NewDefaultPoller(this)),
      m_timerQueue(new TimerQueue(this)),
      m_wakeupChannel(new Channel(this, m_wakeupFd))
{
    LOG_DEBUG << "EventLoop "<< this << " created in thread " << m_threadId;

    /// 若当前线程已经创建了 EventLoop 则直接退出
    if(t_loopInThisThread)
    {
        LOG_FATAL << "Another eventLoop " << t_loopInThisThread
                  << " exists in this thread " << m_threadId;
    }
    else
    {
        t_loopInThisThread = this;
    }
    
    /// 设置 wakeup 唤醒时的处理函数，只是简单的清除 wakeupFd 内核缓冲区中的数据
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleWakeup, this));

    /// 始终监听 wakeupFd 上的读事件
    m_wakeupChannel->enableReading();
}

/// TODO: 思考关于 EventLoop 在其他线程析构的问题
EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << m_threadId
              << " destructs in thread " << CurrentThread::Tid();
    
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakeupFd);
    t_loopInThisThread = nullptr;
}

/// Not thread safe but only in loop thread
void EventLoop::loop()
{   
    assert(!m_looping);
    assertInLoopThread();
    m_looping = true;
    m_quit = false;

    LOG_TRACE << "EventLoop " << this << " start looping";

    while(!m_quit)
    {
        /// 清除活跃 Channel 数组
        m_activeChannels.clear();
        /// 获取 Poller 上监听到的所有 Channel
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
        /// 增加 Loop 循环迭代次数
        ++m_iteration;
        /// 日志打印活跃 Channel 上的所有 revents，用于调式
        if(Logger::LogLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }
        /// 进入事件回调处理状态
        m_eventHandling = true;
        for(Channel* channel : m_activeChannels)
        {
            m_currentActiveChannel = channel;
            m_currentActiveChannel->handleEvent(m_pollReturnTime);
        }
        m_currentActiveChannel = nullptr;
        /// 退出事件回调处理状态
        m_eventHandling = false;

        /// 执行所有的待执行任务
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping" ;
    m_looping = false;
}

void EventLoop::quit()
{   
    /**
     * quit() 函数的调用时机有以下 3 种情况：
     * (1) loop 循环开始之前，此后也不会再进入 loop 循环（这种用法一般是错误的）
     * (2) 如果在 loop 线程，则发生在 m_poller->poll() 调用之后，此时无需 wakeup 唤醒
     * (3) 如果在 loop 线程以外的线程，则可能在 loop 循环的任意时刻被调用，若发生在 
     *  m_poller->poll() 调用之前且 m_poller 没有监听到任何事件发生，则会阻塞 10s，所以需要唤醒
     * 
     * PS: 如果创建一个局部的 EventLoop 对象，且多个线程使用裸指针访问 quit() 函数，则有可能访问到已
     *     经析构了的 EventLoop 对象并导致错误，使用 shared_ptr 管理 EventLoop 对象可以避免该问题
     */
    m_quit = true;
    
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    /// 如果 runInLoop 函数调用时在 loop 的创建线程则立即执行任务
    if(isInLoopThread())
    {
        cb();
    }
    /// 将任务加入调用 loop 对象的待办任务队列
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{   
    /// queueInLoop 可以被 loop 循环所在线程之外的其他线程调用，故 push_back 需要加锁
    {
        LockGuard<Mutex> lock(m_mutex);
        m_pendingFunctors.push_back(std::move(cb));
    }

    /**
     * queueInLoop() 函数被调用时有以下三种情况：
     * (1) 如果不在 loop 线程，则唤醒可能阻塞的 loop 循环，执行待办的任务
     * (2) 如果在 loop 线程且 m_callingPendingFunctors 为 false 状态，则说明 queueInLoop 函数
     * 调用在 m_poller->poll() 执行之后和 doPendingFunctors() 执行之前，此时无需 wakeup，loop 循环
     * 最后自然会在 doPendingFunctors() 内部处理待办的任务
     * (3) 如果在 loop 线程且 m_callingPendingFunctors 为 true 状态则说明在 PendingFunctors 待办任务队列
     * 内部直接或间接调用了queueInLoop 函数，此时任务会进入 swap 后的队列，不会被执行，此时也需要 wakeup 唤醒
     * 
     * PS:关于 doPendingFunctors() 函数中 functors.swap(m_pendingFunctors) 操作的讨论
     * swap 操作一方面减小了临界区，另一方面如果用户在 PendingFunctors 任务中调用了 queueInLoop 函数，则会
     * 导致一种奇怪且错误的情况，即在 foreach 循环中修改了循环的容器本身，类似如下代码的情况
     * 
     * typedef std::function<void()> Functor;
     * std::vector<Functor> functors;
     * Functor obj = { ... }
     * 
     * for(Functor& functor : functors)
     * {
     *     functors.push_back(obj);
     * }
     */
    if(!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    LockGuard<Mutex> lock(m_mutex);
    return m_pendingFunctors.size();
}

TimerId EventLoop::runAt(TimeStamp time, TimerCallback cb)
{
    return m_timerQueue->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    TimeStamp time(AddTime(TimeStamp::Now(), delay));
    return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
    TimeStamp time(AddTime(TimeStamp::Now(), interval));
    return m_timerQueue->addTimer(std::move(cb), time, interval);
}

TimerId EventLoop::runEveryAt(TimeStamp startTime, double interval, TimerCallback cb)
{
    TimeStamp time(AddTime(startTime, interval));
    return m_timerQueue->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
    return m_timerQueue->cancel(timerId);
}

/// Not thread safe but only called in loop thread.
void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    m_poller->updateChannel(channel);
}

/// Not thread safe but only called in loop thread.
void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();

    /**
     * 只有在 Channel 的各种回调函数中调用 removeChannel 函数，
     * 才有可能处于 m_eventHandling 状态，在该状态下，应当只能
     * 移除当前正在执行的 Channel 或当前 activeChannels 中没有
     * 的 Channel
     * 
     * 实际上在 Channel 的各种回调函数中主要关系业务相关的代码，
     * 调用 removeChannel 函数的可能性很小。一种可能是业务代码
     * 在 Poller 上注册了 signalFd，需要移除
     */
    if(m_eventHandling)
    {
        assert(m_currentActiveChannel == channel ||
               std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) == m_activeChannels.end());
    }

    /**
     * 调用 Poller 真正移除 Channel，
     */
    m_poller->removeChannel(channel);
}

/// Not thread safe but only called in loop thread.
bool EventLoop::hasChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return m_poller->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in thread " << m_threadId 
              << ", current thread is " << CurrentThread::Tid();
}

// Thread safe
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::Write(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::handleWakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::Read(m_wakeupFd, &one, sizeof(one));
    if(n != sizeof(one))
    {
        LOG_ERROR << "EventLooop::handleWakeup() reads " << n << " bytes instrad of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;

    /// m_pendingFunctors 可能被其他线程调用以压入任务
    {
        LockGuard<Mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for(const Functor& functor : functors)
    {
        functor();
    }
    m_callingPendingFunctors = false;
}

void EventLoop::printActiveChannels() const
{
    for(const Channel* channel : m_activeChannels)
    {
        LOG_TRACE << "{ " << channel->reventsToString() << "}";
    }
}

}  // namespace net
}  // namespace mrpc