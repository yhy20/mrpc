#include <errno.h>

#include <iostream>

#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "Connector.h"

namespace mrpc
{
namespace net
{
const int Connector::kMaxRetryDelayMs /* = 500 */;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : m_loop(loop),
      m_serverAddr(serverAddr),
      m_state(kDisconnected),
      m_connect(false),
      m_retryDelayMs(kInitRetryDelayMs)  // 默认为 500 毫秒
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!m_channel);
}

/// 线程安全，可重复调用
void Connector::start()
{
    /// 设置处理连接状态
    m_connect = true;
    m_loop->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop()
{
    /// Not thread safe but only run in loop thread.
    m_loop->assertInLoopThread();

    /// 初始状态为 kDisconnected
    assert(m_state == kDisconnected);

    if(m_connect)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop()
{
    m_connect = false;
    m_loop->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::stopInLoop()
{
    m_loop->assertInLoopThread();

    /**
     * 当 PendingFunctors 中包含 startInLoop 任务和 stopInLoop 任务时
     * 刚执行完 startInLoop 后连接状态处于 kConnecting，此时还没有执行到 
     * polling, 在 stopInLoop 任务中会遇到 kConnecting 状态
     */
    if(m_state == kConnecting)
    {
        setState(kDisconnected);
        int socketFd = removeAndResetChannel();

        /// 设置 state 为 kDisconnection 并 close(socketFd)
        retry(socketFd);
    }
}

void Connector::connect()
{
    int socketFd = sockets::CreateNonblockingSocketFdOrDie(m_serverAddr.family());
    /**
     * If the connection or binding succeeds, zero is returned.
     * On error, -1 is returned, and errno is set appropriately.
     */
    int ret = sockets::Connect(socketFd, m_serverAddr.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;

    switch(savedErrno)
    {
    /**
     * No error.
     */
    case 0:
    /**
     * The socket is nonblocking and the connection cannot be completed 
     * immediately. It is possible to select(2) or poll(2) or epoll(2) for
     * completion by listening the socket for writing. After select(2), 
     * poll(2) or epoll(2) indicates writability, use getsockopt(2) to 
     * read the SO_ERROR option at level SOL_SOCKET to determine whether
     * connect() completed successfully (SO_ERROR is zero) or unsuccessfully
     * (SO_ERROR is one of the usual error codes listed here,explaining the 
     * reason `for the failure)
     * 
     * 非阻塞 connect 编程详解
     * 一、当在非阻塞的 socketFd 上调用 connect 时，connect 有可能返回 -1，表示
     * 失败，但如果 errno 是 EINPROGRESS，则说明这个 connect 是非阻塞的，无法立
     * 即完成，需要用 polling API（select, poll, epoll) 去监听可写事件，当监听
     * 的 fd 上有事件发生时，再使用 getsockopt(2) 查看 connect 的结果
     * 
     * PS: 注意，下列写就绪只代表写时不会发生阻塞，不代表会触发 epoll 的写事件，此
     *     处主要是参考第(3)(4)条描述的非阻塞 connect 的状态
     * 
     * 二、根据《unix 网络编程》 A socket is ready for writing if any of the
     * following four conditions is true:
     * (1) The number of bytes of available space in the socket send buffer
     *     is greater than or equal to the current size of the low-water mark
     *     for the socket send buffer and either: (i) the socket is connected,
     *     or (ii) the socket does not require a connection (e.g., UDP). This
     *     means that if we set the socket to nonblocking, a write operation 
     *     will not block and will return a positive value (e.g., the number 
     *     of bytes accepted by the transport layer).
     * 
     * (2) The write half of the connection is closed. A write operation on 
     *     the socket will generate SIGPIPE.
     * 
     * (3) A socket using a non-blocking connect has completed the connection,
     *     or the connect has failed.
     * 
     * (4) A socket error is pending. A write operation on the socket will not
     *     block and will return an error (–1) with errno set to the specific 
     *     error condition.
     * 
     * 由上述所知非阻塞 connect 调用后如果不能立即完成连接会导致 EINPROGRESS errno
     * 此时只需要在 epoll 上监听该 fd, 直到该 fd 上触发写事件（此处需要强调无论是否
     * 连接成功都一定会触发写事件）实际测试如下：
     * 
     * 连接成功时的 revents 日志信息：
     * [TRACE][2022-08-17 05:56:52:869011Z][19517][EventLoop.cc:419][printActiveCh
     * -annels]{ 6: OUT}
     * 
     * 连接失败时的 revents 日志信息：
     * [TRACE][2022-08-17 05:54:44:003283Z][18547][EventLoop.cc:419][printActiveCh
     * -annels]{ 6: OUT HUP ERR}
     * 
     * 可以看到，连接失败（连接失败存在多种可能，例如：对端无服务进程监听，对端防火墙拦截，
     * 网络不通等）不仅会触发 EPOLLOUT 事件，还有可能触发 EPOLLHUP 和 EPOLLERR 事件。
     * 
     * 根据上述情况 Connector 做出如下处理
     * EPOLLHUP: 打印 WARN 警告信息
     * EPOLLERR: 使用 getsockopt(2) 获取 socketFd 上的具体错误信息并记录 LOG_ERRNO 日
     *           志后使用定时器设定 retry，retry 初始等待时间为 500 毫秒，此后每 retry
     *           一次，等待时间增加 1 倍，需要注意，发生 EPOLLERR 事件后不需要再额外处理
     *           EPOLLOUT 事件，因为连接已经失败了。
     * EPOLLOUT: 但上述 2 个事件没触发时，则处理 EPOLLOUT，调用 handleWrite 回调，在该
     *           回调中
     * 
     */
    case EINPROGRESS:
    /**
     * The system call was interrupted by a signal that was caught.
     */
    case EINTR:
    /**
     * The socket is already connected.
     */
    case EISCONN:
        connecting(socketFd);
        break;
    /**
     * Nonblocking return.
     */
    case EAGAIN:
    /**
     * Local address is already in use.
     */
    case EADDRINUSE:
    /**
     * Non-existent interface was requested or the requested address was not local.
     */
    case EADDRNOTAVAIL:
    /**
     * No-one listening on the remote address.
     */
    case ECONNREFUSED:
    /**
     * Network is unreachable.
     */
    case ENETUNREACH:
        retry(socketFd);
        break;
    /**
     * For UNIX domain sockets, which are identified by path‐name: 
     * Write permission is denied on the socket file, or search 
     * permission is denied for one of the directories in the path prefix.
     */
    case EACCES:
    /**
     * The user tried to connect to a broadcast address without
     * having the socket broadcast flag enabled or the connection
     * request failed because of a local firewall rule.
     */
    case EPERM:
    /**
     * The passed address didn't have the correct address family in
     * its sa_family field.
     */
    case EAFNOSUPPORT:
    /**
     * The socket is nonblocking and a previous connection attempt
     * has not yet been completed.
     */
    case EALREADY:
    /**
     * The file descriptor is not a valid index in the descriptor table.
     */
    case EBADF:
    /**
     * The socket structure address is outside the user's address space.
     */
    case EFAULT:
    /**
     * The file descriptor is not associated with a socket.
     */
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
        sockets::Close(socketFd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
        sockets::Close(socketFd);
        break; 
    }
}

void Connector::restart()
{
    m_loop->assertInLoopThread();
    setState(kDisconnected);
    m_retryDelayMs = kInitRetryDelayMs;
    m_connect = true;
    startInLoop();
}

void Connector::connecting(int socketFd)
{
    /**
     * 设置当前状态为 kConnecting，表示当前正在 polling 等待非
     * 阻塞 connect 调用结束并触发相应的事件，完整整个 connect
     */
    setState(kConnecting);
    assert(!m_channel);

    /// 在 I/O 复用上监听写事件（非阻塞 onnect 返回时触发）
    m_channel.reset(new Channel(m_loop, socketFd));
    m_channel->setWriteCallback(
        std::bind(&Connector::handleWrite, this)
    );
    m_channel->setErrorCallback(
        std::bind(&Connector::handleError, this)
    );
    m_channel->enableWriting();
}

void Connector::resetChannel()
{
    m_channel.reset();
}

int Connector::removeAndResetChannel()
{
    /// channel 相关的清理工作
    m_channel->disableAll();
    m_channel->remove();

    int socketFd = m_channel->fd();
    /**
     * removeAndResetChannel 会在 Channel 的 handleEvent 中调用
     * 如果要 reset 必须保证其生命周期，故使用 queueInLoop 保正在
     * handleEvent 执行完毕后析构 Channel 对象
     */
    m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));

    return socketFd;
}   

void Connector::handleError()
{
    /// socketFd 上发生错误
    LOG_ERROR << "Connector::handleError state = " << stateToString(m_state);

    if(m_state == kConnecting)
    {
        int socketFd = removeAndResetChannel();
        int err = sockets::GetSocketError(socketFd);
        LOG_ERROR << "SO_ERROR = " << err << " " << Strerror_tl(err);
        retry(socketFd);
    }
}

/// 正常情况下 connect 连接成功会在 kConnecting 状态下回调 handleWrite 函数
void Connector::handleWrite()
{
    LOG_TRACE << "Connector::handleWrite " << stateToString(m_state);

    if(m_state == kConnecting)
    {
        /// 首先需要重置 Channel
        int socketFd = removeAndResetChannel();

        /// 获取并清除 socketFd 上的错误，如果无错误（连接成功）则返回 0
        int err = sockets::GetSocketError(socketFd);

        /// err 不为 0，说明连接失败，记录错误信息并 retry
        if(err)
        {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                     << err << " " << Strerror_tl(err);
            retry(socketFd);
        }
        /**
         * 关于 Linux/Unix Socket Self-connection 的讨论
         * When a client try to connect to a server, if client and server 
         * are both localhost, self-connection may happen(source port and 
         * destination port happened to be the same.) But my problem is how
         * can self-connection be possible?
         * 原讨论见：
         * https://stackoverflow.com/questions/16767113/linux-unix-socket-self-connection
         * 
         * 讨论中问题描述比较清楚的博文：
         * http://sgros.blogspot.com/2013/08/tcp-client-self-connect.html
         * 
         * 下面是我对这篇博文的尝试与翻译
         *
         * 如果在 shell 中运行下列代码
         * while true
         * do
         *     telnet 127.0.0.1 50000
         * done
         * 
         * 会持续不断的收到 'Connection refused' message, 但是在某一时刻，连
         * 接将会被建立并且不论输入什么字符串都将被回显（类似于 EchoServer),下
         * 面是尝试的部分输出结果
         * 
         * Trying 127.0.0.1...
         * telnet: connect to address 127.0.0.1: Connection refused
         * Trying 127.0.0.1...
         * telnet: connect to address 127.0.0.1: Connection refused
         * Trying 127.0.0.1...
         * telnet: connect to address 127.0.0.1: Connection refused
         * Trying 127.0.0.1...
         * Connected to 127.0.0.1.
         * Escape character is '^]'.
         * test Socket Self-connection
         * test Socket Self-connection
         * 
         * 需要注意，我们并没有启动任何进程在 localhost 上的端口 50000 上
         * 监听，但使用 netstat 命令输出可以看到连接确实建立了
         * netstat -tn | grep -E '50000|Proto'
         * Proto Recv-Q Send-Q Local Address    Foreign Address  State      
         * tcp        0      0 127.0.0.1:50000  127.0.0.1:50000  ESTABLISHED
         * 
         * 并且使用 tcpdump 监控流量可以观察到发生了三次握手，所以到底发生了
         * 什么，简而言之，client connected to itself，下面是更长的解释
         * 首先，我们从一个事实开始，当客户端（在本例中是 telent 应用程序）创建
         * 套接字并尝试连接到服务器时，内核会为其分配一个随机源端口号，这是因为每
         * 个 TCP 连接都用 4 元组唯一标识
         * (source IP, source port, destination IP, destination port)
         * 其中，三个参数是预先确定的，即 source IP, destination IP and destination 
         * port，剩下的是必须以某种方式分配的源端口，通常情况下，应用程序将该任务
         * 留给内核，内核从临时端口范围内选取它（应用程序也可以使用 bind(2) 系统调
         * 用选择源端口，但很少这样做），那么这些临时端口在什么范围内呢？它们通常是
         * high ports（数值较大的端口），我们可以通过查看 /proc file system 来查看
         * 范围，例如：
         * cat /proc/sys/net/ipv4/ip_local_port_range 
         * 32768   60999
         * 在我的服务器上，临时端口号位于 32768 和 61000 之间
         * 
         * 现在回到 telnet 应用程序示例，当 telnet 启动时，内核从给定的临时端口范围
         * 中选择一些空闲端口并尝试连接到 localhost:5000，由于通常没有进程侦听临时端口
         * 因此会收到内核回复的 RST 响应并在 telnet 客户端给出错误消息 Connection denied
         * 
         * 有趣的是，Linux 是按顺序而不是随机选择临时端口的，这可能导致很容易猜到分配的
         * 端口号，这也许是一个安全问题，留待进一步的调查确认，但无论如何，在多出的迭代中
         * 有一次，telnet 客户端被分配源端口 50000，SYN 请求被发送到端口 50000，即发送
         * 给了它自己，所以它与自己建立了联系！这实际上完全符合 TCP 规范（TCP 规范支持所
         * 谓的同时打开功能）【后文依赖于 TCP 状态转换图，暂不阐述】
         * 
         * 关于 Self Connect 的发生条件
         * (1) 客户端和服务器必须在相同的 IP 地址上运行
         * (2) 使用临时端口来监听服务器
         * (3) 只能在握手阶段发生，如果发现某些客户端使用某个临时端口并尝试连接到它，将被拒绝
         * 
         * 总结：不要为服务器使用临时端口！否则，将遇到临一些非常有趣的行为发生，这些行为通常
         * 是不确定的且难以调试的。
         */

        /// 检查是否发生 Self-connection，如果发生则 retry
        else if(sockets::IsSelfConnect(socketFd))
        {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(socketFd);
        }
        /// 完成 TCP 连接的建立
        else
        {
            setState(kConnected);
            if(m_connect)
            {
                m_newConnectionCallback(socketFd);
            }
            else
            {
                sockets::Close(socketFd);
            }
        }
    }
    else
    {
        assert(m_state == kDisconnected);
    }
}


void Connector::retry(int socketFd)
{
    /// 由多种可能导致的连接失败，关闭当前 socketFd 并设置为 kDisconnected 状态
    sockets::Close(socketFd);
    setState(kDisconnected);

    if(m_connect)
    {
        LOG_INFO << "Connector::retry - Retry connecting to " << m_serverAddr.toIpPort()
                 << " in " << m_retryDelayMs << " milliseconds.";
        
        m_loop->runAfter(
            m_retryDelayMs / 1000.0,
            std::bind(&Connector::startInLoop, shared_from_this())
        );
        m_retryDelayMs = std::min(m_retryDelayMs * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

std::string Connector::stateToString(States s)
{
    enum States { kDisconnected, kConnecting, kConnected };
    switch(s)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        default:
            return "Unknown State";
    }
}

}  // namespace net
}  // namespace mrpc