#include <errno.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "Channel.h"
#include "Logging.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "WeakCallback.h"
#include "TcpConnection.h"

namespace mrpc
{
namespace net
{
/**
 * @brief 默认的 TCP 连接建立和断开时的回调函数，LOG_TRACE 打印相关信息
 */
void DefaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

/**
 * @brief 默认的 MessageCallback，直接回收 buffer 的空间，清除从客户端收到的数据
 * @param[in] conn 收到数据的 TCP 连接
 * @param[in] buffer 暂存数据的 buffer 缓存
 * @param[in] receiveTime 收到数据的时间
 */
void DefaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            TimeStamp receiveTime)
{
    buffer->retrieveAll();
}

/**
 * 基于创建一个 TcpConnection、执行准备工作、断开连接、执行清理工作、
 * 销毁一个 TcpConnection 的复杂性，该网络库适用于长连接通信
 */
TcpConnection::TcpConnection(EventLoop* loop,
                             StringArg name,
                             int socketFd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
      /// 传入的 loop 必须非 nullptr 
    : m_loop(CHECK_NOTNULL(loop)),     
      /**
       * 创建 TcpConnection 时传入的 socketFd 实际是从内核已
       * 经完成三次握手的连接队列中取出来的，而此时的 m_state
       * 处于 kConnecting 状态表示该 socketFd 还未在被分发到
       * 的 I/O Loop 线程中注册到 I/O 多路复用的监听 fd 上。
       */
      m_state(kConnecting),    
      /// 初始化时默认在 connectEstablished 中开启监听读事件 
      m_reading(true),                  
      m_name(name.c_str()),  
      m_localAddr(localAddr),
      m_peerAddr(peerAddr),
      m_highWaterMark(64 * 1024 * 1024),
      m_socket(new Socket(socketFd)),
      m_channel(new Channel(loop, socketFd))
{
    /**
     * 设置 socketFd 上读事件发生时触发的回调。
     * 读事件一般发生在对端数据到达或对端正常关闭 TCP 连接时
     */
    m_channel->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1)
    );
    /**
     * 设置 socketFd 上写事件发生时触发的回调。
     * 写事件仅在 TcpConnection 的 outputBuffer 中有数据未发送完毕
     * 时开启监听，开启后若 socketFd 的内核写缓冲区空闲，则写事件会在
     * 水平触发的 Poller 上不断触发，尽可能快的将 outputBuffer 中数据
     * 发送给客户端，若 outputBuffer 中数据全部发送完毕，则停止监听 TCP
     * 连接 socketFd 上的写事件，也不会再触发 WriteCallback 回调
     */
    m_channel->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this)
    );
    /**
     * 设置 socketFd 断开连接时触发的回调。
     * 只有在 POLLHUB 事件发生且 POLLIN 事件不发生时会触发该回调来
     * 关闭 Tcp 连接， 
     * 一般在对端正常关闭 TCP 连接或由服务器主动关闭连接，即 read(2)
     * 返回 0 时调用该回调
     */
    m_channel->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this)
    );
    /// 设置 socketFd 上错误发生时触发的回调。
    m_channel->setErrorCallback(
        std::bind(&TcpConnection::handleError, this)
    );

    LOG_DEBUG << "TcpConnection::ctor[" << m_name << "] at " << this << " fd = " << socketFd;

    /// 这个 setKeepAlive 意义不大，应当在应用层设计心跳包进行 "保活" 和 "判死"
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
              << " fd = " << m_channel->fd() << " state = " << stateToString();
    
    /// 必须在 kDisconnected 状态析构 TcpConnection 对象
    /// 在执行完 connectDestroyed 函数中相关清除操作后置为 kDisconnected 状态
    assert(m_state == kDisconnected);
}

/// 获取 TCP 连接的相关信息
bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const
{
    return m_socket->getTcpInfo(tcpi);
    // return true;
}

/// 获取字符串表示的 TCP 连接的相关信息
std::string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    m_socket->getTcpInfoString(buf, sizeof(buf));
    return buf;
}

/// 可以用于发送二进制数据
void TcpConnection::send(const void* data, int len)
{
    send(StringPiece(static_cast<const char*>(data), len));
}

/**
 * thread safe
 * 考虑数据数据发送实际发生在 loop thread 中的 sendInLoop 函数，而
 * sendInLoop 通常会被延迟调用，在 sendInLoop 调用发生前，需要保证
 * message 的生命周期，这只能生成一份 message 拷贝，效率较低。
 */
void TcpConnection::send(const StringPiece& message)
{
    if(m_state == kConnected)
    {
        /// 在 loop thread，则直接调用 sendInLoop，此时不用
        /// 生成一份 message 的 copy，效率最高。
        if(m_loop->isInLoopThread())
        {
            sendInLoop(message);
        }
        else  // 进入 loop 的待执行任务队列，等待执行
        {
            void (TcpConnection::*func)(const StringPiece&) =
                &TcpConnection::sendInLoop;

            m_loop->runInLoop(
                std::bind(func, this, message.as_string()));
        }
    }
}

/// thread safe
void TcpConnection::send(Buffer* buf)
{
    if(m_state == kConnected)
    {
        if(m_loop->isInLoopThread())
        {
            sendInLoop(buf->readBegin(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*func)(const StringPiece&) =
                &TcpConnection::sendInLoop;

            m_loop->runInLoop(
                std::bind(func, this, buf->retrieveAllAsString()));
        }
    }
}

/// not thread safe but only in loop.
void TcpConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(), message.size());
}

/// not thread safe but only in loop.
void TcpConnection::sendInLoop(const void* data, size_t len)
{
    /// 仅在 TcpConnection 被分发的 loop thread 发送数据
    m_loop->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(m_state == kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }

    /// if nothing in ouputbuffer, try writing directly
    /// 如果 outputBuffer 中无数据，则表明 TCP socketFd 的发送窗口有空闲
    /// 空间，则可以在当前的 LoopThread 尝试性的发送一次数据
    if(!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0)
    {
        nwrote = sockets::Write(m_channel->fd(), data, len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            /// 若一次 wrtie 将 buffer 中数据写完了，则证明 TCP socketFd 的
            /// 发送窗口有空闲，此时触发 m_writeCompleteCallback 回调，一般
            /// 会重新开启接收数据（停止接受数据由 HighWaterMarkCallback 触发）
            if(remaining == 0 && m_writeCompleteCallback)
            {
                m_loop->queueInLoop(
                    std::bind(m_writeCompleteCallback, shared_from_this())
                );
            }
        }
        else  // nwrote < 0
        {
            nwrote = 0;
            /**
             * EAGAIN or EWOULDBLOCK
             * The file descriptor fd refers to a socket and has been marked
             * nonblocking (O_NONBLOCK), and the write would block. POSIX.1-2001
             * allows either error to be returned for this case, and does not
             * require these constants to have the same value.so portable application
             * should check for both possibilities.
             * 
             * 当 nonblocking socket fd 遇到内核缓冲区满时（write 阻塞时），POSIX.1-2001
             * 标准允许返回宏 EAGAIN 和 EWOULDBLOCK 中的任意一个，并且允许这两个宏有不同的值，
             * 对于一个可移植的应用程序，应该同时检查两种可能性
             */
            if(errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                /**
                 * EPIPE
                 * fd is connected to a pipe or socket（客户端 shutdown(SHUT_RD)） whose 
                 * reading end is closed. When this happens the writing process will also
                 * receive a SIGPIPE signal.
                 * 
                 * ECONNRESET
                 * Connection reset by peer.
                 */
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    
    assert(remaining <= len);
    if(!faultError && remaining > 0)
    {
        size_t oldLen = m_outputBuffer.readableBytes();
        /**
         * 如果 outputbuffer 积累的数据超过 highWaterMark（默认是 64 MB)
         * 则触发用户注册的 m_highWaterMarkCallback，一般会停止接收数据
         * 而 WriteCompleteCallback 可以重新开启接收数据
         */
        if(oldLen + remaining >= m_highWaterMark
           && oldLen < m_highWaterMark
           && m_highWaterMarkCallback)
        {
            m_loop->queueInLoop(
                std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
        }
        
        /// 由 outputBuffer 接管剩余数据
        m_outputBuffer.append(static_cast<const char*>(data) + nwrote, remaining);

        /// 开启写事件监听，当 socketFd 内核缓冲区空闲时，继续发送 outputBuffer 中的数据
        if(!m_channel->isWriting())
        {
            m_channel->enableWriting();
        }
    }
}

/// thread safe.
void TcpConnection::shutdown()
{   
    if(m_state == kConnected)
    {
        setState(kDisconnected);
        m_loop->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

/// not thread safe but in loop.
void TcpConnection::shutdownInLoop()
{
    m_loop->assertInLoopThread();
    /// 如果 outputBuffer 中没有待发送的数据，则直接 shutdown(SHUT_WR)。
    if(!m_channel->isWriting())
    {
        m_socket->shutdownWrite();
    }
}

/// thread safe.
void TcpConnection::forceClose()
{
    if(m_state == kConnected || m_state == kDisconnecting)
    {
        setState(kDisconnecting);
        m_loop->queueInLoop(
            std::bind(&TcpConnection::forceCloseInLoop, shared_from_this())
        );
    }   
}

/// 使用 MakeWeakCallback 是因为到时间后，TcpConnection 对象可能已经析构了
void TcpConnection::forceCloseWithDelay(double seconds)
{
    if(m_state == kConnected || m_state == kDisconnecting)
    {
        setState(kDisconnecting);
        m_loop->runAfter(
            seconds,
            /// not forceCloseInLoop to avoid race condition
            MakeWeakCallback(shared_from_this(), &TcpConnection::forceClose)
        );
    }
}

/// not thread safe but in loop.
void TcpConnection::forceCloseInLoop()
{
    m_loop->assertInLoopThread();
    if(m_state == kConnected || m_state == kDisconnecting)
    {
        handleClose();
    }
}

const char* TcpConnection::stateToString() const
{
  switch (m_state)
  {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "Unknown State";
  }
}

/// 高吞吐量发送数据时，通常要关闭 nagle 算法
void TcpConnection::setTcpNoDelay(bool on)
{
    m_socket->setTcpNoDelay(on);
}

/// 线程安全，开始监听读事件
void TcpConnection::startRead()
{
    m_loop->runInLoop(
        std::bind(&TcpConnection::startReadInLoop, this)
    );
}

/// 非线程安全，仅在 loop 线程调用
void TcpConnection::startReadInLoop()
{
    m_loop->assertInLoopThread();
    if(!m_reading || !m_channel->isReading())
    {
        m_channel->enableReading();
        m_reading = true;
    }
}

/// 线程安全，停止监听读事件
void TcpConnection::stopRead()
{
    m_loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

/// 非线程安全，仅在 loop 线程调用
void TcpConnection::stopReadInLoop()
{
    m_loop->assertInLoopThread();
    if(m_reading || m_channel->isReading())
    {
        m_channel->disableReading();
        m_reading = false;
    }
}

/// Not thread safe, but only run in loop thread.
void TcpConnection::connectEstablished()
{
    m_loop->assertInLoopThread();
    /// TcpConnection 对象在 Acceptor 线程建立，初始化状态为 kConnecting
    assert(m_state == kConnecting);
    /// 在 TcpConnection 被分发到的下级 I/O Loop 中将状态重置为 kConnected
    setState(kConnected);

    /**
     * 此处的 tie 用于延长其 owner TcpConnection 的生命周期  
     */
    m_channel->tie(shared_from_this());
    
    /// 将 TcpConnection 管理的 fd 注册到被分
    /// 发的 I/O Loop 的 Poller 并开启读事件监听
    m_channel->enableReading();

    /// TCP 连接建立或断开时触发的回调，默认情况下会用日志记录连接建立或断开的信息
    m_connectionCallback(shared_from_this());
}

/// not thread safe, but only in loop.
void TcpConnection::connectDestroyed()
{
    m_loop->assertInLoopThread();
    if(m_state == kConnected)
    {
        setState(kDisconnected);
        m_channel->disableAll();

        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime)
{
    /// Poller 监听到读事件后调用的 TcpConnection 
    m_loop->assertInLoopThread();
    int savedErrno = 0;

    /// 一次最多读出 128k - 1 字节的数据，若 buffer 容量不足则不会一次性将 fd 内核缓冲区
    /// 中的数据读取完，等待 Poller 水平触发 EPOLLIN or POLLIN 事件后，会再次读取数据。 
    ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &savedErrno);

    /// 调用用户注册的 messageCallback 相关代码，一般会从 inputBuffer 中取出数据，再根据
    /// 数据读取的具体情况进行相关的业务处理，若业务处理时间较长，可以考虑额外开线程处理
    if(n > 0)
    {
        /// 正常情况下 TcpConnection 对象会被 Acceptor I/O 线程中 ConnectionMap 对象持有
        /// 使用 shared_from_this() 将 TcpConnection 对象传递给可能会使用的 MessageCallback
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    }
    /// 对端正常关闭 TCP 连接
    else if(n == 0)
    {
        handleClose();
    }
    /// 发生错误，打印错误信息
    else
    {
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    m_loop->assertInLoopThread();

    /// outputbuffer 中有待发送数据则会在 socketFd 上监听写事件
    if(m_channel->isWriting())
    {
        /// 通过 socketFd 发送数据，具体发送多少数据取决于 TCP 的发送窗口
        ssize_t n = sockets::Write(m_channel->fd(),
                                   m_outputBuffer.readBegin(),
                                   m_outputBuffer.readableBytes());
        if(n > 0)
        {
            /// 从 outputBuffer 中回收已发送数据的空间
            m_outputBuffer.retrieve(n);

            /// 若 outputBuffer 中数据全部发送完毕
            if(m_outputBuffer.readableBytes() == 0)
            {
                /// 必须取消写事件，不然会发生 busy-loop 情况
                m_channel->disableWriting();

                /**
                 * 如果注册了 m_writeCompleteCallback 回调则触发
                 * 该回调对应于 m_highWaterMarkCallback，一般用于在 outputBuffer 
                 * 中积累了大量数据未发送的情况，需要用户介入处理，暂停服务器继续大
                 * 量的发送数据，而 writeCompleteCallback 则发生在 outputBuffer
                 * 中数据全部发送完毕时，可以恢复服务器继续快速发送数据
                 */
                if(m_writeCompleteCallback)
                {
                    m_loop->queueInLoop(
                        std::bind(m_writeCompleteCallback, shared_from_this())
                    );
                }
            
                /// 处于正在关闭连接状态
                if(m_state == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::handleWrite() error!";
        }
    }
    else
    {
        LOG_TRACE << "Connection fd = " << m_channel->fd()
                  << " is down, no more writing";
    }
}

/// Not thread safe, but only in loop.
void TcpConnection::handleClose()
{
    m_loop->assertInLoopThread();
    LOG_TRACE << "fd = " << m_channel->fd() << " state = " << stateToString();
    assert(m_state == kConnected || m_state == kDisconnecting);

    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    m_channel->disableAll();

    /// guardThis 用于延长 TcpConnection 对象的生命周期
    TcpConnectionPtr guardThis(shared_from_this());
    
    /// 调用断开连接时的回调，默认由日志记录客户端断开连接时的信息
    m_connectionCallback(guardThis);

    /**
     * 对 CloseCallback 的调用过程，会依次调用 removeConnection() 和 
     * removeConnectionInLoop() 函数，而 removeConnectionInLoop() 函数
     * 中会调用 m_connections.erase(conn->name())，如果不延长 TcpConnection
     * 对象的生命周期，TcpConnection 析构后调用 connectDestroyed() 函数会
     * 导致 core dump。也就是说 TcpConnection 对象生存期要长于 handleEvent()
     * 函数，直到执行完 connectDestroyed() 后才会析构。
     */
    m_closeCallback(guardThis);
}

/// 线程安全函数
void TcpConnection::handleError()
{
    /// 获取套接字上发生的错误并打印错误信息
    int err = sockets::GetSocketError(m_channel->fd());
    LOG_ERROR << "TcpConnection::handleError [" << m_name
              << "] - SO_ERROR = " << err << " " << Strerror_tl(err);
}

}  // namespace net
}  // namespace mrpc