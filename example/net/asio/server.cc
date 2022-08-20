#include "stdio.h"
#include "unistd.h"

#include <set>

#include "codec.h"
#include "Mutex.h"
#include "Logging.h"
#include "TcpServer.h"
#include "EventLoop.h"

using namespace mrpc;
using namespace mrpc::net;

class ChatServer : noncopyable
{
public:
    ChatServer(EventLoop* loop,
               const InetAddress& listenAddr)
    : m_server(loop, listenAddr, "ChatServer"),
      m_codec(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
    {
        m_server.setConnectionCallback(
            std::bind(&ChatServer::onConnection, this, _1)
        );
        m_server.setMessageCallback(
            std::bind(&LengthHeaderCodec::onMessage, &m_codec, _1, _2, _3)
        );
    }

    void start()
    {
        m_server.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        if(conn->connected())
        {
            m_connectoins.insert(conn);
        }   
        else
        {
            m_connectoins.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                         const std::string& message,
                         TimeStamp)
    {
        for(ConnectionList::iterator itr = m_connectoins.begin();
            itr != m_connectoins.end(); ++itr)
        {
            m_codec.send(get_pointer(*itr), message);
        }
    }

private:
    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer m_server;
    LengthHeaderCodec m_codec;
    ConnectionList m_connectoins;
};

int main(int argc, char* argv[])
{   
    // Logger::SetLogLevel(Logger::TRACE);
    LOG_INFO << "pid = " << getpid();
    if(argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else
    {
        printf("Usage: %s <Port>\n", argv[0]);
    }
}

