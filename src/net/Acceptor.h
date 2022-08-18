#ifndef __MRPC_NET_ACCEPTOR_H__
#define __MRPC_NET_ACCEPTOR_H__

#include <functional>

#include "Socket.h"
#include "Channel.h"

namespace mrpc
{
namespace net
{

class EventLoop;
class InetAddress;

/**
 * @brief TCP 连接受理器，用于启动服务器监听并受理 Poller 返回的
 *        客户端连接请求。需要注意 Acceptor 类的函数都是非线程安
 *        全的，它们只会在 Acceptor 所属的 Acceptor LoopThread
 *        线程调用
 */
class Acceptor : noncopyable
{
public:
    typedef std::function<void(int socketFd, const InetAddress&)> NewConnectionCallback;
    /**
     * @brief 构造函数，为 Acceptor LoopThread 构造一个新 TCP 连接受理器
     * @param[in] loop Acceptor 所属的 EventLoop，一般也是 Acceptor Thread
     * @param[in] ListenAddr 服务器监听地址
     * @param[in] reuseport 是否开启 socketFd 的 reusePort 选项 
     */
    Acceptor(EventLoop* loop, const InetAddress& ListenAddr, bool reusePort);

    /**
     * @brief 析构函数，清理 Channel 注册关闭服务器监听 socketFd，这通常意味着服务器停止运行
     */
    ~Acceptor();

    /**
     * @brief 设置受理 TCP 连接时调用的上级回调，此处是 TcpServer::newConnection()，
     *        它会创建一个 TcpConnection 并设置其各种回调函数，最后分发到线程池中的 I/O 线程
     */
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { m_newConnectionCallback = cb; }

    /**
     * @brief 开启服务器监听（由内核自动完成 3 次握手并放入已连接队列，等待 accept 取用）
     */
    void listen();

    /**
     * @brief 返回服务器是否处于监听状态
     */
    bool listening() const { return m_listening; }

private:
    /**
     * @brief 受理 TCP 连接时实际触发的回调
     */
    void handleRead();

private:
    EventLoop*  m_loop;             // Acceptor 所属的 EventLoop
    bool        m_listening;        // 服务器是否处于监听状态
    int         m_idleFd;           // 预先留置的空闲 fd，用于优雅的解决 fd 资源用尽的问题
    Socket      m_acceptSocket;     // 服务器监听 Socket
    Channel     m_acceptChannel;    // 服务器事件处理 Channel
    
    NewConnectionCallback   m_newConnectionCallback;    // 新 TCP 连接建立时的上级回调
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_ACCEPTOR_H__