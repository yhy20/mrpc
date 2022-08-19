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
 * 关于 TcpConnectoin 的线程安全问题非常复杂，由于对象会被
 * 直接传递给用户使用，用户的行为是未知的，如果在用户代码中
 * 非法的销毁了 TcpConnectoin 对象的某些资源，会导致错误，
 * 所以哪些函数能够提供给用户需要仔细思考。其次，对于某些业
 * 务，需要另外开一个线程处理，这种情况下会向可能向线程中传
 * 入 conn，在新开线程中对 conn 的非线程安全函数的调用都是 
 * 危险的，将直接导致 race condition。之所以这样设计，本质
 * 上是性能对安全性的妥协，它要求使用者清楚的知道自己的行为。
 */

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @brief 构造函数，使用一个已经连接的 socketFd 创建一个 TcpConnection 处理各种事件
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
     * @brief 非线程安全，提供给用户判断 TcpConnection 是否处于连接状态
     */
    bool connected() const { return m_state == kConnected; }
    /**
     * @brief 非线程安全，提供给用户判断 TcpConnection 是否处于断开状态
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

    /**
     * @brief 线程安全，使用 shutdown(fd, SHUT_WR) 关闭 socketFd
     * @details 使用 shutdown(fd, SHUT_WR) 会使得 socketFd 处于
     *          半关闭状态，如果客户端恶意连接以占用服务器资源，那么
     *          TcpConnection 对象永远不会析构，此时正确的处理方法
     *          是设置定时器计时，超时后调用 forceClose() 强制关闭
     *          连接或使用 forceCloseWithDelay 来关闭连接
     */
    void shutdown(); 

    /**
     * @brief 服务器主动强制关闭连接
     */
    void forceClose();
    /**
     * @brief 延迟一定时间后强制关闭连接
     * @param[in] seconds 延迟时间
     */
    void forceCloseWithDelay(double seconds);

    /**
     * @brief 非线程安全，控制是否启用 nagly 算法 
     */
    void setTcpNoDelay(bool on);

    /**
     * @brief 线程安全，开启监听读事件
     */
    void startRead();

    /**
     * @brief 线程安全，停止监听读事件
     */
    void stopRead();

    /**
     * @brief 是否开启监听读事件
     */
    bool isReading() const { return m_reading; }

    
    /// 下面一组函数作为冗余，是提供给用户存储需要的数据

    void setContext(const boost::any& context) { m_context = context; }
    const boost::any& getContext() const { return m_context; } 
    boost::any* getMutableContext() { return &m_context; }

    /// 下列一组设置回调函数都是非线程安全的

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

    /**
     * @brief 非线程安全，返回 TcpConnection 的 inputBuffer
     */
    Buffer* inputBuffer()
    { return &m_inputBuffer; }

    /**
     * @brief 非线程安全，返回 TcpConnection 的 outputBuffer
     */
    Buffer* outputBuffer()
    { return &m_outputBuffer; }

    /**
     * @brief 当 TcpServer 受理一个新的客户端 TCP 请求时调用，主要
     *        将 TcpConnection 的状态设置为 kConnected、该函数应当
     *        被调用一次，且只能在被分发到的 I/O Loop 线程调用。
     */
    void connectEstablished(); 
    /**
     * @brief 当 TcpConnection 被 TcpServer 从 ConnectionMap 中移除后调用。
     *        非线程安全，但是仅在 TcpConnection 被分发的 I/O Loop 线程调用。
     *        主要完成 Poller 中监听事件的清理、记录连接断开的信息以及从 Poller
     *        中移除 channel 等工作，对于每一个 TcpConnection，该函数应该仅被
     *        调用一次
     */
    void connectDestroyed();

private:
    /// TcpConnect 可能的状态
    enum StateE { kDisconnected, kConnecting, kDisconnecting, kConnected };
    
    /// 提供给 Channel 注册的一组回调函数
    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    /// Some functions, not thread safe but only run in loop thread.
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    void forceCloseInLoop();
    void startReadInLoop();
    void stopReadInLoop();

    /// 辅助函数
    void setState(StateE s) { m_state = s; }
    const char* stateToString() const;
   
private:
    EventLoop*          m_loop;             // TcpConnection 所属的 I/O Loop Thread
    StateE              m_state;            // TcpConnection 的当前状态
    bool                m_reading;          // 是否监听读事件
    const std::string   m_name;             // TcpConnection 名称
    const InetAddress   m_localAddr;        // 本端地址
    const InetAddress   m_peerAddr;         // 对端地址
    size_t              m_highWaterMark;    // 高水位回调
    Buffer              m_inputBuffer;      // Tcp 输入缓冲  
    Buffer              m_outputBuffer;     // Tcp 输出缓冲
    boost::any          m_context;          // 用于数据冗余

    std::unique_ptr<Socket>     m_socket;   // socketFd 包装类     
    std::unique_ptr<Channel>    m_channel;  // socketFd Channel

    ConnectionCallback          m_connectionCallback;       
    MessageCallback             m_messageCallback;         
    WriteCompleteCallback       m_writeCompleteCallback;    
    HighWaterMarkCallback       m_highWaterMarkCallback;    
    CloseCallback               m_closeCallback;            
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TCPCONNECTION_H__