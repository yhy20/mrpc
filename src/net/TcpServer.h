#ifndef __MRPC_NET_TCPSERVER_H__
#define __MRPC_NET_TCPSERVER_H__

#include <map>

#include "Types.h"
#include "Atomic.h"
#include "TcpConnection.h"

namespace mrpc
{
namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
    /// 线程创建时调用的初始化回调
    typedef std::function<void(EventLoop*)> ThreadInitCallback;

    /**
     * 是否开启 ReusePort 选项，使用 ReusePort 可以在单机上开多个
     * 服务器进程，依靠内核来做简单的负载均衡，将连接分发到不同的服
     * 务器进程，一般情况下不这样做，而是跑一个 Nginx 来做负载均衡
     */
    enum Option 
    {
        kNoReusePort, 
        kReusePort 
    };

    /**
     * @brief 构造函数，构造一个单线程或多线程的 TcpServer
     * @param[in] loop Acceptor Loop，运行在 TcpServer 的创建线程
     * @param[in] name TcpServer 的名称
     * @param[in] option 是否开启 SO_REUSEPORT 选项
     */
    TcpServer(EventLoop* loop,
              const InetAddress& listenAddr,
              StringArg name,
              Option option = kNoReusePort);
    
    ~TcpServer();
    /**
     * @brief 线程安全、返回 TcpServer 的 Accepotr EventLoop
     */
    EventLoop* getLoop() const { return m_loop; }
    /**
     * @brief 线程安全、返回 TcpServer 的名称
     */
    const std::string& name() const { return m_name; }
    /**
     * @brief 线程安全、返回 Listening IP 和 Port
     */
    const std::string& ipPort() const { return m_ipPort; }
    
    /**
     * @brief 非线程安全、设置 TcpServer 的 TcpConnection I/O 处理线程数目，仅允
     *        许在 TcpServer 调用 start() 之前调用，numThreads 为 0 表示所有处理
     *        在 Acceptor Loop 中进行，不会创建任何其他线程; numThreads 为 1 表示
     *        所有 TcpConnection I/O 处理在额外创建的一个线程中进行; numThreads 为
     *        N(N > 1) 表示所有 TcpConnection I/O 处理通过轮询的方式分发到 N 个额外
     *        创建的 LoopThread 进行处理。  
     */
    void setThreadNum(int numThreads);
   
    /**
     * @brief 返回 start() 后容量固定的 EventLoop 线程池
     */
    std::shared_ptr<EventLoopThreadPool> threadPool()
    { return m_threadPool; }

    /**
     * @brief 线程安全函数，允许多线程重复多次调用无副作用，会首先初始化
     *        TcpServer 的 EventLoopThreadPool 线程池，然后启动服务器监听
     */
    void start();

    /// 下列所有的 Callback 都是非线程安全的，需要在 TcpServer start 之前调用

    /**
     * @brief 设置线程池中线程创建时触发的初始化回调函数，由用户根据具体情况设置
     */
    void setThreadInitCallback(const ThreadInitCallback& cb)
    { m_threadInitCallback = cb; }
    
    /**
     * @brief 设置客户端发起的新连接到达时触发的回调函数，由用户根据具体情况设置
     *        一般会用 LOG_INFO 打印 TCP 连接的四元组信息，或获取客户端的相关信息
     */
    void setConnectionCallback(const ConnectionCallback& cb)
    { m_connectionCallback = cb; }

    /**
     * @brief 设置客户端数据到达时触发的服务器处理回调，由用户根据业务情况来设置
     */
    void setMessageCallback(const MessageCallback& cb)
    { m_messageCallback = cb; }

    /**
     * @brief 设置 TcpConnection 的 outputBufeer 中数据全部发送完毕时触发的回调
     */
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { m_writeCompleteCallback = cb; }

private:
    /**
     * @brief Accepotr 监听到客户端发起新连接时触发的回调。
     *        非线程安全，但只会在 Acceptor Loop 线程调用
     */
    void newConnection(int socketFd, const InetAddress& peerAddr);

    /**
     * @brief 线程安全函数，该函数会被注册到每一个 TcpConnection 中，
     *        当 TcpConnection 连接关闭时在其他的 I/O 线程触发，触发
     *        后向 Acceptor Loop 线程的待执行队列中压入 removeConne-
     *        ctionInLoop 任务，等待一轮 Loop 的最后统一执行关闭 TCP
     *        连接的清理工作。
     */
    void removeConnection(const TcpConnectionPtr& conn);

    /**
     * @brief 非线程安全，仅在 Accepotr Loop 线程执行的关闭 TCP 连接的清理工作
     */
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    /// TcpConnectionPtr map, 用于管理 TcpConnection 的生命周期
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop*              m_loop;                   // Acceptor Loop
    const std::string       m_name;                   // TcpServer 名称
    const std::string       m_ipPort;                 // 服务器监听的 IP 和 Port
    int                     m_nextConnId;             // 自增的 TcpConnection Id
    AtomicInt32             m_started;                // 用于保证多次 start() 调用的线程安全
    ConnectionMap           m_connections;            // 所有建立连接的 TCP 连接

    ThreadInitCallback      m_threadInitCallback;     // 线程池中线程创建时触发的初始化回调
    ConnectionCallback      m_connectionCallback;     // 新 TCP 连接建立时触发的回调
    MessageCallback         m_messageCallback;        // 已建立的 TCP 上数据到达时触发的回调
    WriteCompleteCallback   m_writeCompleteCallback;  // TCP ouputBuffer 数据发送完毕触发的回调

    std::unique_ptr<Acceptor>               m_acceptor;     // 新 TCP 连接到达接收处理器 
    std::shared_ptr<EventLoopThreadPool>    m_threadPool;   // TcpConnection I/O 线程池
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TCPSERVER_H__