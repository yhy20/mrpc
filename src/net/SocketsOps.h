#ifndef __MRPC_NET_SOCKETSOPS_H__
#define __MRPC_NET_SOCKETSOPS_H__

#include "arpa/inet.h"

namespace mrpc
{
namespace net
{
namespace sockets
{
/**
 * @brief 创建一个非阻塞的 IPv4(IPv6) 的 socketFd，创建失败则结束进程
 * @param[in] family 创建套接字使用的协议族（AF_INET or AF_INET6)
 */
int CreateNonblockingSocketFdOrDie(sa_family_t family);

/**
 * @brief 绑定服务器地址，绑定失败则结束进程
 * @param[in] socketFd 服务器套接字
 * @param[in] addr 绑定的服务器地址
 */
void BindOrDie(int socketFd, const struct sockaddr* addr);

/**
 * @brief 监听服务器服务器端口，监听失败则结束进程
 * @param[in] socketFd 服务器套接字
 */
void ListenOrDie(int socketFd);

int Connect(int socketFd, const struct sockaddr* addr);
int Accept(int socketFd, struct sockaddr_in6* addr);
void ShutdownWrite(int socketFd);
void Close(int fd);

ssize_t Read(int socketFd, void* buf, size_t bufSize);
ssize_t Readv(int socketFd, const struct iovec* iov, int iovcnt);
ssize_t Write(int socketFd, const void* buf, size_t bufSize);

void ToIp(char* buf, size_t size, const struct sockaddr* addr);
void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);

struct sockaddr_in6 GetLocalAddr(int socketFd); 
struct sockaddr_in6 GetPeerAddr(int socketFd);
int GetSocketError(int socketFd);
bool IsSelfConnect(int socketFd);

/// 下列一组函数在 sockaddr_in* 、sockaddr_in6* 和 sockaddr* 之间进行安全的类型转换

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

}  // namespace sockets
}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_SOCKETSOPS_H__

