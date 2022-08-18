#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "SocketsOps.h"
#include "InetAddress.h"

namespace mrpc
{
namespace net
{

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort)
    : m_loop(loop),
      m_listening(false),
      m_idleFd(open("/dev/null", O_RDONLY | O_CLOEXEC)),
      m_acceptSocket(sockets::CreateNonblockingSocketFdOrDie(listenAddr.family())),
      m_acceptChannel(loop, m_acceptSocket.fd())
{
    assert(m_idleFd != -1);

    /// 允许复用 TIME-WAIT 状态下的地址
    m_acceptSocket.setReuseAddr(true);
    /// 允许复用地址，使用内核进行负载均衡分发
    m_acceptSocket.setReusePort(reusePort);
    /// 绑定服务器监听地址
    m_acceptSocket.bindAddress(listenAddr);

    /// 服务器监听 socketFd 只处理 read event
    m_acceptChannel.setReadCallback(
        std::bind(&Acceptor::handleRead, this)
    );
}

Acceptor::~Acceptor()
{
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
    ::close(m_idleFd);
}

void Acceptor::listen()
{
    m_loop->assertInLoopThread();
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}

void Acceptor::handleRead()
{
    /// 该函数只能在 loop 线程调用
    m_loop->assertInLoopThread();

    /// 从已经完成三次握手的 TCP 连接中获取一个并获取客户端地址和端口号
    InetAddress peerAddr;
    int connfd = m_acceptSocket.accept(&peerAddr);
    
    if(connfd >= 0)
    {
        /**
         * 此处 m_newConnectionCallback 实际是 TcpServer::newConnection() 函数
         * 仔细思考可以知道、这里发生的函数调用包括 handleRead 和 newConnection，以
         * 及这两个函数使用的 Acceptor 类和 TcpServer 类中的资源都在 Acceptor Loop
         * Thread，没有任何安全性的问题,不过层层的 Callback 回调容易造成思维上的混乱
         */
        if(m_newConnectionCallback) 
        {
            m_newConnectionCallback(connfd, peerAddr);
        }
        else
        {
            sockets::Close(connfd);
        }
    }
    else
    {
        LOG_SYSERR << "Acceptor::handleRead";
        /**
         * Read the section named "The special problem of
         * accept()ing when you can't" in libev's doc.
         * By Marc Lehmann, author of libev.
         * 
         * For example, larger servers often run out of file descriptors 
         * (because of resource limits), causing accept to fail with ENFILE 
         * but not rejecting the connection, leading to libev signalling 
         * readinesson the next iteration again (the connection still exists
         * after all), and typically causing the program toloop at 100% CPU 
         * usage.One of the easiest ways to handle this situation is to just
         * ignore it - when the program encounters anoverload, it will just 
         * loop until the situation is over. While this is a form of busy waiting, 
         * no OS offers an event-based way to handle this situation, so it's the
         * best one can do.A better way to handle the situation is to log any
         * errors other than EAGAIN and EWOULDBLOCK,making sure not to flood the
         * log with such messages, and continue as usual, which at least gives
         * the user an idea of what could be wrong ("raise the ulimit!"). 
         * For extra points one could stop the ev_io watcheron the listening fd
         * "for a while", which reduces CPU usage. If your program is single-threaded,
         * then you could also keep a dummy file descriptor for overloadsituations 
         * (e.g. by opening /dev/null), and when you run into ENFILE or EMFILE, 
         * it, run accept,close that fd, and create a new dummy fd. This will 
         * gracefully refuse clients under typical overload conditions.The last
         * way to handle it is to simply log the error and exit, as is often done
         * with malloc failures,but this results in an easy opportunity for a DoS attack.
         * 
         * 对于上面这段作者的分析，内容大致如下：
         * 在大型服务器中，经过会出现描述符用完（通常是 linux 的资源限制导致的）的情况，这会导致
         * accept() 失败，并返还一个 ENFILE 错误，但是内核并没有拒绝这个连接，连接仍然在连接队列
         * 中，这导致在下一次迭代的时候，仍然会触发监听描述符的可读事件，最终造成程序 busy loop。
         * 一种简单的处理方式就是当程序遇到这种问题就直接忽略掉，直到这种情况消失，显然这种方法将
         * 会导致 busy waiting，一种比较好的处理方式就是记录除了 EAGAIN 或 EWOULDBLOCK 其他任何
         * 错误，告诉用户出现了某种错误，并停止监听描述符的可读事件，减少 CPU 的使用。如果程序是单
         * 线程的，我们可以先 open /dev/null，保留一个描述符，当 accept() 出现 ENFILE 或 EMFILE 
         * 错误的时候，close 掉 /dev/null 这个 fd，然后 accept，再 close 掉 accept 产生的 fd，
         * 然后再次 open /dev/null，这是一种比较优雅的方式来拒绝掉客户端的连接。最后一种方式则是遇
         * 到 accept() 的这种错误，直接拒绝并退出，但是显然这种方式很容易受到Dos攻击。
         * 
         * PS：Acceptor 对象仅在 Acceptor 线程中受理客户端连接，符合上述单线程优雅处理方式。
         */
        if(errno == EMFILE)
        {
            ::close(m_idleFd);
            m_idleFd = ::accept(m_acceptSocket.fd(), NULL, NULL);
            ::close(m_idleFd);
            m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

}  // namespace net
}  // namespace mrpc

