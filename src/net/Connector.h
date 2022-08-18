#ifndef __MRPC_NET_CONNECTOR_H__
#define __MRPC_NET_CONNECTOR_H__

#include <memory>
#include <functional>

#include "noncopyable.h"
#include "InetAddress.h"

namespace mrpc
{
namespace net
{

class Channel;
class EventLoop;

class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void(int socketFd)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        m_newConnectionCallback = cb;
    }

    /**
     * @brief 线程安全函数，开始尝试非阻塞 connect 连接，连接失败会不断重试
     */
    void start();

    /**
     * @brief 非线程安全，尝试重连
     * TODO:
     * 
     */
    void restart();

    /**
     * @brief 线程安全函数，
     * 
     * 
     */
    void stop();

    /**
     * @brief 
     *
     * 
     * @return const InetAddress& 
     */
    const InetAddress& serverAddress() const { return m_serverAddr; }


private:
    enum States { kDisconnected, kConnecting, kConnected };
    static std::string stateToString(States s);

    static const int kMaxRetryDelayMs = 30 * 1000;
    static const int kInitRetryDelayMs = 500;
    
    /**
     * @brief 设置状态迁移
     * @param[in] s kDisconnected, kConnecting or kConnected
     */
    void setState(States s) { m_state = s; }

    /**
     * @brief 在 Loop 线程开始非阻塞连接
     */
    void startInLoop();
    /**
     * @brief 
     */
    void stopInLoop();
    /**
     * @brief 
     */
    void connect();
    /**
     * @brief 
     */
    void connecting(int socketFd);
    /**
     * @brief 处理 polling 在 socketFd 上监听到的 POLLOUT 事件
     */
    void handleWrite();
    /**
     * @brief 处理 polling 在 socketFd 上监听到的 POLLERR 事件
     */
    void handleError();
    /**
     * @brief 状态迁移至 kDisconnected 并尝试重连
     */
    void retry(int socketFd);
    /**
     * @brief 清除并销毁 Channel
     */
    int removeAndResetChannel();

    /**
     * @brief 销毁 Channel，
     * @details 之所以将该函数独立出来，是为了用 queueInLoop 运行以保
     *          证 Channel 的生命周期长于 handleEvent 函数的运行时期
     */
    void resetChannel();

private:
    EventLoop*    m_loop;           // Connector 所属的 loop
    InetAddress   m_serverAddr;     // 要连接的服务器地址
    States        m_state;          // 当前连接状态
    bool          m_connect;        // 用于控制 stop connect
    int           m_retryDelayMs;   // 重连延迟时间

    std::unique_ptr<Channel> m_channel;                 // 注册非阻塞 connect 结束触发事件的回调
    NewConnectionCallback    m_newConnectionCallback;   // 上级的新 TCP 连接建立回调
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_CONNECTOR_H__