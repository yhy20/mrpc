#include "TcpServer.h"

// RFC 863
class DiscardServer
{
public:
    DiscardServer(mrpc::net::EventLoop* loop,
                  const mrpc::net::InetAddress& listenAddr);
    
    void start();

private:
    void onConnection(const mrpc::net::TcpConnectionPtr& conn);
    void onMessage(const mrpc::net::TcpConnectionPtr& conn,
                   mrpc::net::Buffer* buf,
                   mrpc::TimeStamp time);

private:
    mrpc::net::TcpServer m_server;
};