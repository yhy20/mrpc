#include "echo.h"
#include "Logging.h"

using namespace mrpc;
using namespace mrpc::net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

EchoServer::EchoServer(EventLoop* loop, 
                       const InetAddress& listenAddr)
    : m_server(loop, listenAddr, "EchoServer")
{
    m_server.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1)
    );

    m_server.setMessageCallback(
        std::bind(&EchoServer::onMessage, this, _1, _2, _3)
    );
}

void EchoServer::start()
{
    m_server.start();
}

void EchoServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           TimeStamp time)
{
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
             << "data received at " << time.toString();
    conn->send(msg);
}


