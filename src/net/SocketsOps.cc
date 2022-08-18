#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "Types.h"
#include "Endian.h"
#include "Logging.h"
#include "SocketsOps.h"

namespace mrpc
{
namespace net
{

#if VALGRIND || defined(NO_ACCEPT4)
static void SetNonBlockAndCloseOnExec(int socketFd)
{
    /// non-block
    int flags = ::fcntl(socketFd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(socketFd, F_SETFL, flags);

    /// close-on-exec
    flags = ::fcntl(socketFd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(socketFd, F_SETFD, flags);

    /// FIXME check
    (void)ret;
}
#endif

namespace sockets
{

int CreateNonblockingSocketFdOrDie(sa_family_t family)
{
#if VALGRIND
    int socketFd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == socketFd)
    {
        LOG_SYSFATAL << "sockets::CreateNonblockingOrDie";
    }
    SetNonBlockAndCloseOnExec(socketFd);
#else
    int socketFd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(-1 == socketFd)
    {
        LOG_SYSFATAL << "sockets::CreateNonblockingOrDie";
    }
#endif
    return socketFd;
}

int Connect(int socketFd, const struct sockaddr* addr)
{
    return ::connect(socketFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void BindOrDie(int socketFd, const struct sockaddr* addr)
{
    int ret = ::bind(socketFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if(-1 == ret)
    {
        LOG_SYSFATAL << "sockets::BindOrDie";
    }
}

void ListenOrDie(int socketFd)
{
    int ret = ::listen(socketFd, SOMAXCONN);
    if(-1 == ret)
    {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int Accept(int socketFd, struct sockaddr_in6* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = :: accept(socketFd, sockaddr_cast(addr), &addrlen);
    SetNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(socketFd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if(-1 == connfd)
    {
        int savedErrno = errno;
        LOG_SYSERR << "sockets::Accept";
        switch(savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

void ShutdownWrite(int socketFd)
{
    if(-1 == ::shutdown(socketFd, SHUT_WR))
    {
        LOG_SYSERR << "sockets::ShutdownWrite";
    }
}

void Close(int fd)
{
    if(-1 == ::close(fd))
    {
        LOG_SYSERR << "sockets::Close";
    }
}

ssize_t Read(int socketFd, void* buf, size_t bufSize)
{
    return ::read(socketFd, buf, bufSize);
}
ssize_t Readv(int socketFd, const struct iovec * iov, int iovcnt)
{
    return ::readv(socketFd, iov, iovcnt);
}

ssize_t Write(int socketFd, const void* buf, size_t bufSize)
{
    return ::write(socketFd, buf, bufSize);
}
void ToIp(char* buf, size_t size, const struct sockaddr* addr)
{
    if(addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if(addr->sa_family == AF_INET6)
    {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void ToIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
    if(addr->sa_family == AF_INET)
    {
        ToIp(buf, size, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        uint16_t port = NetworkToHost16(addr4->sin_port);
        assert(size > end);
        snprintf(buf + end, size - end, ":%u", port);
    }
    else if(addr->sa_family == AF_INET6)
    {
        buf[0] = '[';
        ToIp(buf + 1, size - 1, addr);
        size_t end = ::strlen(buf);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        uint16_t port = NetworkToHost16(addr6->sin6_port);
        assert(size > end);
        snprintf(buf + end, size - end, "]:%u", port);
    }
}

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = HostToNetwork16(port);
    if(-1 == ::inet_pton(AF_INET, ip, &addr->sin_addr))
    {
        LOG_SYSERR << "sockets::fromIpPort";
    }
}

void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
    addr->sin6_family = AF_INET6;
    addr->sin6_port = HostToNetwork16(port);
    if(-1 == ::inet_pton(AF_INET6, ip, &addr->sin6_addr))
    {
        LOG_SYSERR << "sockets::fromIpPort";
    }

}

struct sockaddr_in6 GetLocalAddr(int socketFd)
{
    struct sockaddr_in6 localAddr;
    memZero(&localAddr, sizeof(localAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localAddr));
    if(-1 == ::getsockname(socketFd, sockaddr_cast(&localAddr), &addrlen))
    {
        LOG_SYSERR << "sockets::GetLocalAddr";
    }
    return localAddr;
}

struct sockaddr_in6 GetPeerAddr(int socketFd)
{
    struct sockaddr_in6 peerAddr;
    memZero(&peerAddr, sizeof(peerAddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peerAddr));
    if (-1 == ::getpeername(socketFd, sockaddr_cast(&peerAddr), &addrlen))
    {
        LOG_SYSERR << "sockets::GetPeerAddr";
    }
    return peerAddr;
}

int GetSocketError(int socketFd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if(-1 == ::getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

bool IsSelfConnect(int socketFd)
{
    struct sockaddr_in6 localaddr = GetLocalAddr(socketFd);
    struct sockaddr_in6 peeraddr = GetPeerAddr(socketFd);
    if(AF_INET == localaddr.sin6_family)
    {
        const struct sockaddr_in* laddr4 = 
            reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = 
            reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        
        return laddr4->sin_port == raddr4->sin_port
            && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    }
    else if(AF_INET6 == localaddr.sin6_family)
    {
        return localaddr.sin6_port == peeraddr.sin6_port
            && 0 == memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof(localaddr.sin6_addr));
    }
    else
    {
        return false;
    }
}

/**
 * 下列是通用 sockaddr 指针与 sockaddr_in、sockaddr_in6 指针之间的转化
 * 
 * 无关系指针类型的转换可以使用 reinterpret_cast 运算符
 * 但 reinterpret_cast 运算符并不会改变括号中运算对象的值
 * 而是对该对象从位模式上进行重新诠释，这种转化并不安全
 * 下列用法先将 src 指针类型 implicit_cast （向上转换）成 void*,
 * 再用 static_cast 将 void* 转化为 dst 指针类型
 */

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
{   
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr)
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

}  // namespace sockets
}  // namespace net
}  // namespace mrpc