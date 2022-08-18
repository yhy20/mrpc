#include <stdio.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "Logging.h"
#include "SocketsOps.h"
#include "InetAddress.h"

namespace mrpc
{
namespace net
{

Socket::~Socket()
{
    sockets::Close(m_socketFd);
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const 
{
    assert(tcpi != nullptr);
    socklen_t len = sizeof(*tcpi);
    memZero(tcpi, len);
    return ::getsockopt(m_socketFd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

/// TODO: learn more
bool Socket::getTcpInfoString(char* buf, int len) const
{
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if(ok)
    {
        snprintf(buf, len, "unrecovered=%u "
                 "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                 "lost=%u retrans=%u rtt=%u rttvar=%u "
                 "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits,     // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,             // Retransmit timeout in usec
                 tcpi.tcpi_ato,             // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss,
                 tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,            // Lost packets
                 tcpi.tcpi_retrans,         // Retransmitted packets out
                 tcpi.tcpi_rtt,             // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,          // Medium deviation
                 tcpi.tcpi_snd_ssthresh,
                 tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress& addr)
{
    sockets::BindOrDie(m_socketFd, addr.getSockAddr());
}

void Socket::listen()
{
    sockets::ListenOrDie(m_socketFd);
}

int Socket::accept(InetAddress* peerAddr)
{
    struct sockaddr_in6 addr;
    memZero(&addr, sizeof(addr));
    int connfd = sockets::Accept(m_socketFd, &addr);
    if(connfd >= 0)
    {
        peerAddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    sockets::ShutdownWrite(m_socketFd);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, static_cast<socklen_t>(sizeof(optval)));
    // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof(optval)));
    if (ret < 0 && on)
    {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(m_socketFd, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof(optval)));
  // FIXME CHECK
}

}  // namespace net
}  // namespace mrpc

