#ifndef __MRPC_NET_TCPCLIENT_H__
#define __MRPC_NET_TCPCLIENT_H__

#include <atomic>

#include "Mutex.h"
#include "TcpConnection.h"

namespace mrpc
{
namespace net
{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient : noncopyable
{
public:
    /**
     * @brief 构造函数，构造一个 TCP 客户端对象
     * @param[in] loop 非阻塞 connect 注册的 Connector Loop，
     *                 多个 TcpClient 可以炮在同一个 Loop 上
     * @param[in] serverAddr 服务器网络地址
     * @param[in] name TcpClient 名称
     */
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              StringArg name);
    
    ~TcpClient();
    /**
     * @brief 发起 TCP 连接
     */
    void connect();
    /**
     * @brief 关闭已经建立 TCP 连接
     */
    void disconnect();

    /**
     * @brief 停止尝试连接服务器
     */
    void stop();

    /**
     * @brief 线程安全函数，返回建立的 TCP 连接
     */
    TcpConnectionPtr connection() const
    {
        LockGuard<Mutex> m_lock(m_mutex);
        return m_connection; 
    }

    /**
     * @brief 返回 TcpClient 所属的 Connector Loop
     *
     */
    EventLoop* getLoop() const { return m_loop; }
    /**
     * @brief 线程安全函数，返回是否设置 retry
     * 
     */
    bool retry() const { return m_retry.load(std::memory_order_relaxed); }
    /**
     * @brief 线程安全函数，允许 TCP 连接断开并移除后重新尝试连接
     */
    void enableRetry() { m_retry.store(true, std::memory_order_relaxed); }
    /**
     * @brief 线程安全函数，返回 TcpClient 名称
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief 非线程安全函数，设置连接建立和断开时触发的回调
     */
    void setConnectionCallback(ConnectionCallback cb)
    { m_connectionCallback = std::move(cb); }

    /**
     * @brief 非线程安全函数，设置 TCP 连接上数据到达时触发的回调
     */
    void setMessageCallback(MessageCallback cb)
    { m_messageCallback = std::move(cb); }

    /**
     * @brief 非线程安全函数，设置 outputBuffer 上数据发送完毕触发的回调
     */
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    { m_writeCompleteCallback = std::move(cb); }

private:
    /**
     * @brief 新连接建立和断开时触发的回调
     */
    void newConnection(int socketFd);
    /**
     * @brief TCP 连接断开后触发的清理回调 
     */
    void removeConnection(const TcpConnectionPtr& conn);

private:
    EventLoop*          m_loop;         // TcpClient 所属的 Accepotr Loop
    ConnectorPtr        m_connector;    // Tcp 连接器
    const std::string   m_name;         // TcpClient 名称
    int                 m_nextConnId;   // TcpConnection Id
    mutable Mutex       m_mutex;        // 互斥锁
    TcpConnectionPtr    m_connection;   // 建立的 TCP 连接 
    /**
     * muduo 库作者陈硕大神在此处直接使用 bool 类型并注释为 atomic
     * bool m_retry;    // atomic
     * bool m_connect;  // atomic 
     * 这造成了我的一些疑惑，在我的认识中，即使是 volatile bool
     * 也无法保证 100% 的线程安全，下面是 stackoverflow 上对该
     * 问题的讨论，以及我的一些理解。
     * 原文见：https://stackoverflow.com/questions/29633222/c-stdatomicbool-and-volatile-bool
     * 
     * 经典的利用 bool 类型进行读者写者线程同步的代码如下：
     * std::vector<int> data;
     * std::atomic<bool> data_ready(false);
     * 
     *  void reader_thread()
     *  {
     *      while(!data_ready.load())
     *      {
     *          std::this_thread::sleep(std::milliseconds(1));
     *      }
     *      std::cout << "The answer=" << data[0] << "\n";
     *  }
     * 
     *  void writer_thread()
     *  {
     *      data.push_back(42);
     *      data_ready = true;
     *  }
     * 
     * 首先，如果将 std::atomic<bool> 替换成 bool，那么在 reader_thread
     * 线程中，由于编译器看不到前后上下文中 data_ready 会发生改变，此时很
     * 可能发生优化只从内存中读取一次 data_ready 的值，而 data_ready 的初
     * 始值为 false，这将导致死循环的发生。退一步说，即使没有发生编译器优化，
     * data_ready 的值也有很大的可能从缓存中读取，由于多核 CPU 缓存不一致的
     * 的问题，导致 writer_thread 对 data_ready 的修改不能及时的被 reader
     * _thread 看到，程序也不是完全可靠的。使用 volatile bool 可以保证每一
     * 次都重新加载 data_ready，不会发生仅读取一次内存的优化，其次可以保证每
     * 一次修改变量都写回主存并从主存中获取变量的值，这解决了多核 CPU 缓存不
     * 一致导致的问题。但实际上 volatile bool 的行为任然没有被 C++ 标准定义，
     * 也就是说，访问 volatile bool 变量既不是同步的，也不是原子的。
     * 
     * 相信作者原来的写法一定是基于某种原因（或许与 Linux 平台实现相关，目前我
     * 猜测 Linux 平台上 bool 类型的并发访问也许确实是原子的），所以此问题留待
     * 进一步考证，认真思考代码中可能的问题，既是对原作者的尊重，也是对自己负责，
     * 所以在彻底搞懂本问题前，我暂时将 m_retry 与 m_connect 替换为
     * std::atomic<bool> 类型
     */
    std::atomic<bool>   m_retry;        // 控制 TCP 连接断开后是否重连
    std::atomic<bool>   m_connect;      // 控制是否停止尝试 TCP 连接

    ConnectionCallback      m_connectionCallback;    
    MessageCallback         m_messageCallback;         
    WriteCompleteCallback   m_writeCompleteCallback;   
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TCPCLIENT_H__