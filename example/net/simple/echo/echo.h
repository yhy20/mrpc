#include "TcpServer.h"

using namespace mrpc;
using namespace mrpc::net;

/// RFC 862
class EchoServer
{
public:
    EchoServer(mrpc::net::EventLoop* loop, 
               const mrpc::net::InetAddress& listenAddr);
    
    void start();   // call m_server.start();

private:
    void onConnection(const mrpc::net::TcpConnectionPtr& conn);
    void onMessage(const mrpc::net::TcpConnectionPtr& conn,
                   mrpc::net::Buffer* buf,
                   mrpc::TimeStamp time);

private:
    TcpServer m_server;
};