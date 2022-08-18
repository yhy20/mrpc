#ifndef __MRPC_NET_TCPCLIENT_H__
#define __MRPC_NET_TCPCLIENT_H__

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
     * @param[in] loop 
     * @param[in] serverAddr 
     * @param[in] name 
     */
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              StringArg name);
    
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        LockGuard<Mutex> m_lock(m_mutex);
        return m_connection; 
    }

    EventLoop* getLoop() const { return m_loop; }
    bool retry() const { return m_retry; }
    void enableRetry() { m_retry = true; }
    const std::string& name() const { return m_name; }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(ConnectionCallback cb)
    { m_connectionCallback = std::move(cb); }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(MessageCallback cb)
    { m_messageCallback = std::move(cb); }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(WriteCompleteCallback cb)
    { m_writeCompleteCallback = std::move(cb); }

private:
    void newConnection(int socketFd);
    void removeConnection(const TcpConnectionPtr& conn);

private:
    EventLoop* m_loop;
    ConnectorPtr m_connector;
    const std::string m_name;

    ConnectionCallback      m_connectionCallback;
    MessageCallback         m_messageCallback;
    WriteCompleteCallback   m_writeCompleteCallback;

    bool m_retry;
    bool m_connect;
    int m_nextConnId;
    mutable Mutex m_mutex;
    TcpConnectionPtr m_connection;
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TCPCLIENT_H__