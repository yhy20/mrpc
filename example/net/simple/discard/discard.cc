#include "discard.h"
#include "Logging.h"

using namespace mrpc;
using namespace mrpc::net;

DiscardServer::DiscardServer(EventLoop* loop,
                             const InetAddress& listenAddr)
    : m_server(loop, listenAddr, "DiscardServer")
{
    m_server.setConnectionCallback(
        std::bind(&DiscardServer::onConnection, this, _1)
    );
    m_server.setMessageCallback(
        std::bind(&DiscardServer::onMessage, this, _1, _2, _3)
    );
}

void DiscardServer::start()
{
    m_server.start();
}

void DiscardServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "DiscardServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void DiscardServer::onMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              TimeStamp time)
{
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " discards " << msg.size()
             << " bytes received at " << time.toString();
}