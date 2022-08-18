#include <stdio.h>

#include "Logging.h"
#include "Acceptor.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "EventLoopThreadPool.h"

namespace mrpc
{
namespace net
{

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     StringArg name,
                     Option option)
    : m_loop(CHECK_NOTNULL(loop)),
      m_name(name.c_str()),
      m_ipPort(listenAddr.toIpPort()),
      m_nextConnId(0),
      m_connectionCallback(DefaultConnectionCallback),
      m_messageCallback(DefaultMessageCallback),
      m_acceptor(new Acceptor(loop, listenAddr, option == kReusePort)),
      m_threadPool(new EventLoopThreadPool(loop, m_name))
{
    /// 设置新 TCP 连接建立时的处理回调，若不设置则默认由服务器关闭连接
    m_acceptor->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer()
{
    m_loop->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << m_name << "] destructing";

    for(auto& item : m_connections)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(numThreads >= 0);
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(m_started.store(1) == 0)
    {
        /// 创建并启动 EventLoop 线程池中的线程
        m_threadPool->start(m_threadInitCallback);

        /// 监听服务器的端口
        assert(!m_acceptor->listening());
        m_loop->runInLoop(
            std::bind(&Acceptor::listen, get_pointer(m_acceptor))
        );
    }
}

void TcpServer::newConnection(int socketFd, const InetAddress& peerAddr)
{
    /// 受理客户端发起的连接必须在 Acceptor 注册的 I/O EventLoop 线程
    m_loop->assertInLoopThread();
    
    /// 从线程池中通过轮询的方式获取一个 EventLoop 来受理客户请求
    EventLoop* ioLoop = m_threadPool->getNextLoop();

    /// 生成 newConnection 的名称并记录日志信息
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", m_ipPort.c_str(), m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + buf;
    LOG_INFO << "TcpServer::newConnection [" << m_name
             << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();
    
    /// 创建一个新的 TcpConnection 并放入 ConnectionMap
    InetAddress localAddr(sockets::GetLocalAddr(socketFd));
    TcpConnectionPtr conn = 
        std::make_shared<TcpConnection>(ioLoop, connName, socketFd, localAddr, peerAddr);
    // TcpConnectionPtr conn(
    //     new TcpConnection(ioLoop, connName, socketFd, localAddr, peerAddr);
    // );
    m_connections[connName] = conn;
    
    /**
     * 设置 TCP 连接建立和断开时触发的用户回调，如果用户不设置 ConnectionCallback
     * 则在默认函数 DefaultConnectionCallback 中用 LOG_TRACE 记录连接的上下线信息
     */
    conn->setConnectionCallback(m_connectionCallback);

    /**
     * 设置当监听到 TcpConnection 管理的 socketFd 上用户数据到达时触发的数据处理用户回调
     * 如果用户不设置 MessageCallback，则默认调用 DefaultMessageCallback 清除所有的数据
     */
    conn->setMessageCallback(m_messageCallback);

    /**
     * 对于非阻塞网络编程的发送数据，如果发送数据的速度远高于对方接受数据的数度
     * 则会造成数据在本地内存堆积。针对该问题网络库提供了 HighWaterMarkCallback
     * 和 WriteCompleteCallback 这一对回调，也被称为高水位回调和低水位回调。当
     * TcpConnectoin 的 ouputBuffer 中积累的数据超过用户指定的大小，则会触发高
     * 水位回调（只在上升沿触发一次）。当 OutputBuffer 中积累的数据全部发送完毕
     * 则会触发 WriteCompleteCallback 回调，此处正是设置 WriteCompleteCallback
     */
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    /**
     * 设置 TCP 连接关闭时执行的回调，如果 Acceptor 线程与 TCP 连接建立后的受理
     * 线程是同一个线程，则 removeConnection 中 bind 的 removeConnectionInLoop
     * 会立即执行，如果 Acceptor 线程与 TCP 连接的受理线程分离，则该回调函数会在
     * 受理 TCP 连接的线程中被调用，但实际执行函数 removeConnectionInLoop 会被压
     * 入 Acceptor 线程的待执行任务队列并在 Loop 循环的最后统一执行，此处理方法一
     * 方面可以使用非常小的多线程临界区，提高程序性能；另一方面可以解决多线程调用导
     * 致的安全性问题。对于一个 TcpConnection，除了建立和关闭时需要使用 EventLoop
     * 的 PendingFunctors 队列进行跨线程的调用和同步外，所有的业务数据处理始终在其
     * 所属的 I/O EventLoop，而 I/O EventLoop 中使用 I/O 多路复用来监听和管理多个
     * 已建立的 TcpConnections 上的业务数据，所以一个 TcpConnection 的业务处理过程
     * 是完全无锁且线程安全的，这种设计可以完全发挥多核 CPU 的优势，性能非常可观。
     */
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );
    
    /// 在被分发的 I/O Loop 中注册 TcpConnection 监听的事件并执行用户设置的 TCP 连接建立回调
    ioLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn)
    );
}   

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn)
    );
}

/// Not thread safe but only run in loop thread.
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    m_loop->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" 
             << m_name << "] - connection " << conn->name();

    /// 从 ConnectionMap 中删除 TcpConnection（TcpConnection 对象实际不会被析构）
    size_t n = m_connections.erase(conn->name());
    assert(n == 1); (void)n;

    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}

}  // namespace net
}  // namespace mrpc
