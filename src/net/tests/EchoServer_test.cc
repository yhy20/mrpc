#include <stdio.h>
#include <unistd.h>

#include <unistd.h>

#include "Thread.h"
#include "LogFile.h"
#include "Logging.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"

using namespace mrpc;
using namespace mrpc::net;

int numThreads = 0;

std::unique_ptr<LogFile> g_logFile;

void LogOutput(const char* msg, int len)
{
    g_logFile->append(msg, len);
}

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress& listenAddr)
        : m_loop(loop),
          m_server(loop, listenAddr, "EchoServer"/* , TcpServer::kReusePort */)
    {
        m_server.setConnectionCallback(
        std::bind(&EchoServer::onConnection, this, _1));
        m_server.setMessageCallback(
            std::bind(&EchoServer::onMessage, this, _1, _2, _3)
        );
        m_server.setThreadNum(numThreads);
    }

    void start()
    {
        m_server.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
        LOG_INFO << conn->getTcpInfoString();

        conn->send("hello\n");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        LOG_TRACE << "message from client: " << msg;
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
        if (msg == "exit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n")
        {
            m_loop->quit();
        }
        conn->send(msg);
    }

private:
  EventLoop* m_loop;
  TcpServer m_server;
};

/**
 * 
 * 
 */

int main(int argc, char* argv[])
{
    Logger::SetLogLevel(Logger::TRACE);
    // g_logFile.reset(new LogFile("EchoServer.log", 500 * 1000 * 1000, true));
    // Logger::SetOutput(LogOutput);

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::Tid();
    LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
    if (argc > 1)
    {
        numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    EventLoop loop;
    InetAddress listenAddr(2000, false, ipv6);
    EchoServer server(&loop, listenAddr);

    server.start();

    loop.loop();
} 