#include "stdio.h"
#include "unistd.h"

#include <utility>

#include "Thread.h"
#include "Logging.h"
#include "TcpClient.h"
#include "EventLoop.h"
#include "InetAddress.h"

using namespace mrpc;
using namespace mrpc::net;

class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
int numThreads = 0; 
int current = 0;

class EchoClient : noncopyable
{
public:
    EchoClient(EventLoop* loop, const InetAddress& listenAddr, const std::string& id)
        : m_loop(loop),
          m_client(loop, listenAddr, "EchoClient" + id)
    {
        m_client.setConnectionCallback(
            std::bind(&EchoClient::onConnection, this, _1));
        m_client.setMessageCallback(
            std::bind(&EchoClient::onMessage, this, _1, _2, _3));
    }

    void connect()
    {
        m_client.connect();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_TRACE << conn->localAddress().toIpPort() << " -> "
                  << conn->peerAddress().toIpPort() << " is "
                  << (conn->connected() ? "UP" : "DOWN");
        if(conn->connected())
        {
            ++current;
            if (implicit_cast<size_t>(current) < clients.size())
            {
                clients[current]->connect();
            }
            LOG_INFO << "*** connected " << current;
        }
        conn->send("exit\n");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp time)
    {
        std::string msg(buf->retrieveAllAsString());
        LOG_TRACE << "message from server: " << msg;
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
        if (msg == "quit\n")
        {
            conn->send("bye\n");
            conn->shutdown();
        }
        else if (msg == "shutdown\n")
        {
            m_loop->quit();
        }
        else
        {
            conn->send(msg);
        }
    }

private:
    EventLoop* m_loop;
    TcpClient m_client;
};

int main(int argc, char* argv[])
{
    Logger::SetLogLevel(Logger::TRACE);
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::Tid();
    if(argc > 1)
    {
        EventLoop loop;
        bool ipv6 = argc > 3;
        /// IP、Port、useIPv6
        InetAddress serverAddr(argv[1], 2000, ipv6);

        int n = 1;
        if(argc > 2)
        {
            n = atoi(argv[2]);
        }

        clients.reserve(n);
        for(int i = 0; i < n; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", i + 1);
            clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
        }
        clients[current]->connect();
        loop.loop();
    } 
    else
    {
        printf("Usage: %s <IP> <Clnt Cnt> <IPv6>\n", argv[0]);
    }
}

