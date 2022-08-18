#ifndef __MRPC_NET_TCPCONNECTION_H__
#define __MRPC_NET_TCPCONNECTION_H__

#include <memory>

#include <boost/any.hpp>

#include "Types.h"
#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "StringPiece.h"

struct tcp_info;

namespace mrpc
{
namespace net
{

class Socket;
class Channel;
class EventLoop;

/**
 *  
 */
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @brief 构造函数，使用一个已经连接的 socketFd 创建一个 TTcpConnection 处理各种事件
     * @param[in] loop TcpConnection 所属的 I/O loop
     * @param[in] name TCP 连接的名称
     * @param[in] socketFd 完成三次握手的 socketFd
     * @param[in] localAddr 本端地址
     * @param[in] peerAddr 对端地址
     */
    TcpConnection(EventLoop* loop,
                  StringArg name,
                  int socketFd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);

    ~TcpConnection();

    /**
     * @brief 线程安全，返回当前 TcpConnection 所属的 I/O Loop
     */
    EventLoop* getLoop() const { return m_loop; }
    /**
     * @brief 线程安全，返回当前 TcpConnection 的名称
     */
    const std::string& name() const { return m_name; }

    /**
     * @brief 线程安全，返回本端网络地址
     */
    const InetAddress& localAddress() const { return m_localAddr; }
    /**
     * @brief 线程安全，返回对端网络地址
     */
    const InetAddress& peerAddress() const { return m_peerAddr; }

    /**
     * @brief 
     */
    bool connected() const { return m_state == kConnected; }
    /**
     * @brief 
     */
    bool disconnected() const { return m_state == kDisconnected; }

    /**
     *@brief 获取 TCP 连接的相关信息
     */
    bool getTcpInfo(struct tcp_info* tcpi) const;
    /**
     *@brief 获取字符串表示的 TCP 连接的相关信息
     */
    std::string getTcpInfoString() const;

    /**
     * @brief send 重载，一般用于二进制数据
     * @param[in] message 字节数组
     * @param[in] len 发送字节数目
     */
    void send(const void* message, int len);
    /**
     * @brief send 重载，发送 StringPiece 类型数据
     * @param[in] message 兼容 const char* 和 std::string
     */
    void send(const StringPiece& message);
    /**
     * @brief send 重载，发送 Buffer 中的数据
     * @param[in] message Buffer 缓冲中的数据
     */
    void send(Buffer* message);

    void shutdown();
    void forceClose();
    void forceCloseWithDelay(double seconds);
    void setTcpNoDelay(bool on);

    void startRead();
    void stopRead();
    bool isReading() const { return m_reading; }

    void setContext(const boost::any& context) { m_context = context; }
    const boost::any& getContext() const { return m_context; } 
    boost::any* getMutableContext() { return &m_context; }

    void setConnectionCallback(const ConnectionCallback& cb) 
    { m_connectionCallback = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { m_messageCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { m_writeCompleteCallback = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { m_highWaterMarkCallback = cb; m_highWaterMark = highWaterMark; }

    void setCloseCallback(const CloseCallback& cb)
    { m_closeCallback = cb; }

    Buffer* inputBuffer()
    { return &m_inputBuffer; }

    Buffer* outputBuffer()
    { return &m_outputBuffer; }

    /**
     * @brief 当 TcpServer 受理一个新的客户端 TCP 请求时调用，主要
     *        将 TcpConnection 的状态设置为 kConnected、
     * 
     *      该函数
     *        只能被调用一次，且只能在被分发到的 I/O Loop 线程调用
     *        
     */
    // called when TcpServer accepts a new connection
    /**
     * 当 TcpServer 接收到一个新的连接时调用
     */
    void connectEstablished();   // should be called only once
    /**
     * @brief 当 TcpConnection 被 TcpServer 从 ConnectionMap 中移除后调用。
     *        非线程安全，但是仅在 TcpConnection 被分发的 I/O Loop 线程调用。
     *        主要完成 Poller 中监听事件的清理、记录连接断开的信息以及从 Poller
     *        中移除 channel 等工作，对于每一个 TcpConnection，该函数应该仅被
     *        调用一次
     */
    void connectDestroyed();

private:
    enum StateE { kDisconnected, kConnecting, kDisconnecting, kConnected };
    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();

    void setState(StateE s) { m_state = s; }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();

private:
    EventLoop* m_loop;
    StateE m_state;
    bool m_reading;

    std::unique_ptr<Socket> m_socket;
    std::unique_ptr<Channel> m_channel;

    const std::string m_name;
    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;

    ConnectionCallback      m_connectionCallback;
    MessageCallback         m_messageCallback;
    WriteCompleteCallback   m_writeCompleteCallback;
    HighWaterMarkCallback   m_highWaterMarkCallback;
    CloseCallback           m_closeCallback;

    size_t m_highWaterMark;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
    boost::any m_context;

};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TCPCONNECTION_H__