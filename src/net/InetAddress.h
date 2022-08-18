#ifndef __MRPC_NET_INETADDRESS_H__
#define __MRPC_NET_INETADDRESS_H__

#include <netinet/in.h>

#include "copyable.h"
#include "StringPiece.h"

namespace mrpc
{
namespace net
{
namespace sockets
{

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

}  // namespace sockets

/**
 * @brief   Wrapper of sockaddr_in and sockaddr_in6
 * @details This is an POD(Plain old data structure) interface class.
 *          POD 是 Plain Old Data 的缩写，是 C++ 定义的一类数据结构概念，
 *          比如 int、float 等都是 POD 类型的。Plain 代表它是一个普通类型，
 *          Old 代表它是旧的，与几十年前的 C 语言兼容，那么就意味着可以使用
 *          memcpy() 这种最原始的函数进行操作, 两个系统进行交换数据，如果没
 *          有办法对数据进行语义检查和解释，那就只能以非常底层的数据形式进行
 *          交互，而拥有 POD 特征的类或者结构体通过二进制拷贝后依然能保持数据
 *          结构不变。也就是说，能用 C 的 memcpy() 等函数进行操作的类、结构
 *          体就是 POD 类型的数据
 */
class InetAddress : public copyable
{
public:
    /**
     * @brief 构造函数，创建一个网络地址对象
     * @param[in] port 端口号
     * @param[in] loopback 是否使用本地循环（127.0.0.1)，默认使用 INADDR_ANY（0.0.0.0）
     * @param[in] ipv6 是否使用 IPv6 协议
     */
    explicit InetAddress(uint16_t port = 0,
                         bool loopback = false,
                         bool ipv6 = false);

    /**
     * @brief 构造函数，创建一个网络地址对象
     * @param[in] ip IP 地址
     * @param[in] prot 端口号
     * @param[in] ipv6 是否使用 IPv6 协议
     */
    InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);

    /**
     * @brief 构造函数
     * @param[in] addr 描述 IPv4 套接字的结构体
     */
    explicit InetAddress(const struct sockaddr_in& addr)
        : m_addr(addr) { }

    /**
     * @brief 构造函数
     * @param[in] addr 描述 IPv6 套接字的结构体
     */
    explicit InetAddress(const struct sockaddr_in6& addr6)
        : m_addr6(addr6) { }

    /// default copy constructor/copy assignment operator are okay
    /// default move constructor/move assignment operator are okay

public:
    /**
     * @brief 返回指向通用套接字结构体指针，实际指向的结构体可能是 sockaddr_in 或者 sockaddr_in6
     */
    const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&m_addr6); }

    /**
     * @brief 设置 IPv6 套接字结构体信息
     */
    void setSockAddrInet6(const struct sockaddr_in6& addr6) { m_addr6 = addr6; }

    /**
     * @brief 设置 IPv6 ScopeID，
     */
    void setScopeId(uint32_t scopeId);

    /**
     * @brief 返回字符串表示的 ip 地址（IPv4 or IPv6)
     */
    std::string toIp() const;

    /**
     * @brief 返回字符串表示的 IP 和 port 地址（IPv4:port or [IPv6]:port)
     */
    std::string toIpPort() const;

    /**
     * @brief 返回当前套接字使用的协议族（AF_INET or AF_INET6）
     */
    sa_family_t family() const { return m_addr.sin_family; }

    /**
     * @brief 主机序表示 IPv4 地址
     */
    uint32_t ipv4();

    /**
     * @brief 网络序表示的 IPv4 地址 
     */
    uint32_t ipv4NetEndian() const;

    /**
     * @brief 主机序表示的端口号
     */
    uint16_t port() const;

    /**
     * @brief 网络序表示的端口号
     */
    uint16_t portNetEndian() const;

public:

    /**
     * @brief 将主机名解析为 IP 地址（线程安全）
     * @return 成功返回 true，失败返回 false
     */
    static bool Resolve(StringArg hostname, InetAddress* result);

private:
    union 
    {
        struct sockaddr_in m_addr;      // ipv4
        struct sockaddr_in6 m_addr6;    // ipv6
    };
};

}  // namespace net
}  // namespace mrcp

#endif  // __MRPC_NET_INETADDRESS_H__