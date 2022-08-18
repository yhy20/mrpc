#ifndef __MRPC_NET_SOCKET_H__
#define __MRPC_NET_SOCKET_H__

#include "noncopyable.h"

namespace mrpc
{
namespace net
{

class InetAddress;

/**
 * @brief TCP 套接字类
 */
class Socket : noncopyable
{
public:
    /**
     * @brief 使用已创建的 socketFd 构造一个 Socket 类   
     */
    explicit Socket(int socketFd) : m_socketFd(socketFd) { }
    
    /**
     * @brief 析构函数，Socket 负责管理 socketFd 的连接生命周
     *        期在 Socket 对象析构时会负责关闭持有的 socketFd 
     */
    ~Socket();

    /**
     * @brief 线程安全函数，返回 Socket 持有的 socketFd 文件描述符
     */
    int fd() const { return m_socketFd; }

    /**
     * @brief 线程安全函数，返回 socketFd 上的 TCP 连接信息，用于调试
     * @details 该函数使用用户提供的 tcpi，所以线程安全
     * @param[in] tcpi tcp_info 结构体
     */
    bool getTcpInfo(struct tcp_info* tcpi) const;

    /**
     * @brief 线程安全函数，返回字符串表示的 TCP 连接信息，用于调试
     * @details 该函数使用用户提供的 buffer，所以线程安全
     * @param[in] buf 用户提供的 buffer
     * @param[in] len 用户 buffer 的最大容量
     */
    bool getTcpInfoString(char* buf, int len) const;

    /**
     * @brief 为 socketFd 绑定监听的服务器网络地址，如果绑定
     *        失败则记录 LOG_SYSFATAL 日志信息并结束进程 
     */
    void bindAddress(const InetAddress& addr);

    /**
     * @brief 开启服务器监听 socketFd，如果监听失败
     *        则记录 LOG_SYSFATAL 日志信息并结束进程 
     */
    void listen();

    /**
     * @brief 从内核的已连接 TCP 队列中获取一个连接
     * @param[out] peerAddr 保存对端的网络地址信息
     */
    int accept(InetAddress* peerAddr);

    /**
     * @brief 使用 shutdown(socketFd, SHUT_WR) 半关闭连接
     */
    void shutdownWrite();

    /**
     * @brief 是否使用 Nagle 算法（操作系统默认开启 Nagle 算法）
     */
    void setTcpNoDelay(bool on);

    /**
     * @brief 是否设置 SO_REUSEADDR 选项，用于忽略 TIME_WAIT 状态立即启动服务器
     */
    void setReuseAddr(bool on);

    /**
     * @brief 是否设置 SO_REUSEPORT 选项，用于内核负载均衡分发 TCP 连接
     * @details 在单机上进行负载均衡意义不大，应当使用 Nginx 等负载均衡器
     *          在服务器集群上进行 TCP 连接分发才充分发挥服务器的性能
     */
    void setReusePort(bool on);

    /**
     * @brief 设置 SO_KEEPALIVE 选项，默认每 2 小时发送一次 "保活" 信息
     * @details 该选项意义不大，应当在应用层设计心跳包来 keep alive
     */
    void setKeepAlive(bool on);

private:
    const int m_socketFd;   // 网络套接字文件描述符
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_SOCKET_H__