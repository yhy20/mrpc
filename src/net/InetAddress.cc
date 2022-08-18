

#include "Endian.h"
#include "Logging.h"
#include "SocketsOps.h"
#include "InetAddress.h"

#include <netdb.h>
#include <stddef.h>
#include <netinet/in.h>

/// INADDR_ANY and INADDR_LOOPBACK use old-style-cast as shown below.
/// #define	INADDR_ANY		((in_addr_t) 0x00000000) /* Address to accept any incoming messages */
/// #define INADDR_LOOPBACK	((in_addr_t) 0x7f000001) /* Inet 127.0.0.1. */
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

namespace mrpc
{
namespace net
{

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");

static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t port, 
                         bool loopback /*  = false */,
                         bool ipv6 /* = false */)
{
    static_assert(offsetof(InetAddress, m_addr6) == 0, "addr6_ offset 0");
    static_assert(offsetof(InetAddress, m_addr) == 0, "addr_ offset 0");
    if(ipv6)
    {
        memZero(&m_addr6, sizeof(m_addr6));
        m_addr6.sin6_family = AF_INET6;
        in6_addr ip = loopback ? in6addr_loopback : in6addr_any;
        m_addr6.sin6_addr = ip;
        m_addr6.sin6_port = sockets::HostToNetwork16(port);
    }
    else
    {
        memZero(&m_addr, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
        in_addr_t ip = loopback ? kInaddrLoopback : kInaddrAny;
        m_addr.sin_addr.s_addr = sockets::HostToNetwork32(ip);
        m_addr.sin_port = sockets::HostToNetwork16(port);
    }
}

InetAddress::InetAddress(StringArg ip, uint16_t port, bool ipv6)
{
    if(ipv6 || strchr(ip.c_str(), ':'))
    {
        memZero(&m_addr6, sizeof(m_addr6));
        sockets::FromIpPort(ip.c_str(), port, &m_addr6);
    }
    else
    {
        memZero(&m_addr, sizeof(m_addr));
        sockets::FromIpPort(ip.c_str(), port, &m_addr);
    }
}

void InetAddress::setScopeId(uint32_t scopeId)
{
    if(family() == AF_INET6)
    {
        m_addr6.sin6_scope_id = scopeId;
    }
}

std::string InetAddress::toIp() const 
{
    char buf[64] = { 0 };
    sockets::ToIp(buf, sizeof(buf), getSockAddr());
    return buf;
}

std::string InetAddress::toIpPort() const 
{
    char buf[64] = { 0 };
    sockets::ToIpPort(buf, sizeof(buf), getSockAddr());
    return buf; 
}

uint32_t InetAddress::ipv4()
{
    assert(family() == AF_INET);
    return sockets::NetworkToHost32(ipv4NetEndian());
}

uint32_t InetAddress::ipv4NetEndian() const 
{
    assert(family() == AF_INET);
    return m_addr.sin_addr.s_addr;
}

uint16_t InetAddress::port() const
{
    return sockets::NetworkToHost16(portNetEndian());
}

uint16_t InetAddress::portNetEndian() const
{
    return m_addr.sin_port;
}

static thread_local char t_resolveBuffer[64 * 1024];

bool InetAddress::Resolve(StringArg hostname, InetAddress* result)
{
    assert(result != nullptr);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    memZero(&hent, sizeof(hent));

    int ret = gethostbyname_r(hostname.c_str(), &hent, 
        t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herrno);
    
    if(0 == ret && he != nullptr)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        result->m_addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if(ret)
        {
            LOG_SYSERR << "InetAddress::Resolve";
        }
        return false;
    }
}

}  // namespace net
}  // namespace mrpc







