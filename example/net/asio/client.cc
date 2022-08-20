#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "Util.h"
#include "codec.h"
#include "Mutex.h"
#include "Logging.h"
#include "TcpClient.h"
#include "EventLoopThread.h"

using namespace mrpc;
using namespace mrpc::net;

class ChatClient : noncopyable
{
public:
    ChatClient(EventLoop* loop, const InetAddress& serverAddr)
        : m_client(loop, serverAddr, "ChatClient"),
          m_codec(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
    {
        m_client.setConnectionCallback(
            std::bind(&ChatClient::onConnection, this, _1)
        );
        m_client.setMessageCallback(
            std::bind(&LengthHeaderCodec::onMessage, &m_codec, _1, _2, _3)
        );
        m_client.enableRetry();
    }

    void connect()
    {
        m_client.connect();
    }

    void disconnect()
    {
        m_client.disconnect();
    }

    void write(const StringPiece& message)
    {
        printf("in write!\n");
        LockGuard<Mutex> lock(m_mutex);
        if(m_connectoin)
        {
            m_codec.send(get_pointer(m_connectoin), message);
        }
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");
        
        LockGuard<Mutex> lock(m_mutex);
        if(conn->connected())
        {
            m_connectoin = conn;
        }
        else
        {
            m_connectoin.reset();
        }
    }

    void onStringMessage(const TcpConnectionPtr&,
                         const std::string message,
                         TimeStamp)
    {
        printf("<<< %s\n", message.c_str());
    }

private:
    Mutex               m_mutex;  
    TcpClient           m_client;  
    LengthHeaderCodec   m_codec;
    TcpConnectionPtr    m_connectoin;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if(argc > 2)
    {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while(std::getline(std::cin, line))
        {
            client.write(line);
        }
        client.disconnect();
        Util::SleepSec(1);
    }
    else
    {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
    }
}