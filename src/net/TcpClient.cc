#include <stdio.h>  // for snprintf

#include "Logging.h"
#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "SocketsOps.h"

namespace mrpc
{
namespace net
{
namespace details
{
void RemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}

void RemoveConnector(const ConnectorPtr& connector)
{

}

}  // namespace details

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr,
                     StringArg name)
    : m_loop(CHECK_NOTNULL(loop)),
      m_connector(new Connector(loop, serverAddr)),
      m_name(name.c_str()),
      m_nextConnId(1),
      m_retry(false),
      m_connect(true),
      m_connectionCallback(DefaultConnectionCallback),
      m_messageCallback(DefaultMessageCallback)
{
    m_connector->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, _1)
    );
    
    LOG_INFO << "TcpClient::TcpClient[" << m_name
             << "] - connector " << get_pointer(m_connector);
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient" << m_name
             << "] - connector " << get_pointer(m_connector);
             
    TcpConnectionPtr conn;
    bool unique = false;

    {
        LockGuard<Mutex> lock(m_mutex);
        unique = m_connection.unique();
        conn = m_connection;
    }
    
    if(conn)
    {
        assert(m_loop == conn->getLoop());

        CloseCallback cb = std::bind(&details::RemoveConnection, m_loop, _1);
        m_loop->runInLoop(
            std::bind(&TcpConnection::setCloseCallback, conn, cb)
        );
        
        if(unique)
        {
            conn->forceClose();
        }
    }
    else
    {
        m_connector->stop();
        m_loop->runAfter(1, std::bind(&details::RemoveConnector, m_connector));
    }
}

void TcpClient::connect()
{
    LOG_INFO << "TcpClient::connect[" << m_name << "] - connecting to "
             << m_connector->serverAddress().toIpPort();

    m_connect.store(true, std::memory_order_relaxed);
    m_connector->start();
}

void TcpClient::disconnect()
{
    m_connect.store(false, std::memory_order_relaxed);

    {
        LockGuard<Mutex> lock(m_mutex);
        if(m_connection)
        {
            m_connection->shutdown();
        }
    }
}

void TcpClient::stop()
{
    m_connect.store(false, std::memory_order_relaxed);
    m_connector->stop();
}

/// Not thread safe but only run in loop thread.
void TcpClient::newConnection(int socketFd)
{
    m_loop->assertInLoopThread();
    InetAddress peerAddr(sockets::GetPeerAddr(socketFd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peerAddr.toIpPort().c_str(), m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + buf;
    InetAddress localAddr(sockets::GetLocalAddr(socketFd));

    TcpConnectionPtr conn = 
        std::make_shared<TcpConnection>(m_loop, connName, socketFd, localAddr, peerAddr);

    // TcpConnectionPtr conn(
    //     new TcpConnection(m_loop, connName, socketFd, localAddr, peerAddr)
    // );

    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);

    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, _1)
    );
    
    {
        LockGuard<Mutex> lock(m_mutex);
        m_connection = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
    m_loop->assertInLoopThread();
    assert(m_loop == conn->getLoop());

    {
        LockGuard<Mutex> lock(m_mutex);
        assert(m_connection == conn);
        m_connection.reset();
    }

    m_loop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );

    if(m_retry.load(std::memory_order_relaxed) && m_connect)
    {
        LOG_INFO << "TcpClient::connect[" << m_name << "] - Reconnectin to "
                 << m_connector->serverAddress().toIpPort();
        m_connector->restart();
    }
}

}  // namespace net
}  // namespace mrpc
