#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <thread>
#include <sstream>
#include <exception>
#include <condition_variable>

#include "../header.h"

#define BUF_SIZE 100
#define EPOLL_EVENTS_SIZE 10

std::mutex g_mutex;
bool g_serverReady = false;
std::condition_variable g_cond;

void Client()
{
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2001);

    int opt_nodelay = true;
    socklen_t opt_nodelay_len = sizeof(opt_nodelay);
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

    std::unique_lock<std::mutex> lock(g_mutex);
    while(!g_serverReady)
    {
        g_cond.wait(lock);
    }
    lock.unlock();

    if(-1 == connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_FATAL("connect() error!");

    close(sock);
    printf("clientThread stop!\n");
}

/**
 * 服务器向关闭的客服端发送数据，即 write 调用 2 次触发 SIGPIPE 信号
 * 如果不忽略该信号，将直接终结服务器进程。至于为何要 write 2 次触发，
 * 原因很简单，因为在服务器第一次 write 之前，客户端的状态是未知的，不 
 * 过发送包到达后会收到客户端回复的 RST 包，此时服务器已经知道客户端断
 * 开连接了，再调用 write 会触发 SIGPIPE 信号  
 */
void Server()
{
    int serv_sock;
    int clnt_sock;   
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // int opt_nodelay = true;
    // socklen_t opt_nodelay_len = sizeof(opt_nodelay);
    // setsockopt(clnt_sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

    int opt_reuseaddr = true;
    socklen_t opt_reuseaddr_len = sizeof(opt_reuseaddr);
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_reuseaddr, opt_reuseaddr_len);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(2001);
    
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    int ret = listen(serv_sock, 5);
    if(-1 == ret) LOG_FATAL("listen() error!");

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_serverReady = true;
        g_cond.notify_one();
    }

    /// 阻塞调用
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

    sleep(1);

    char msg[] = "hello!";
    ret = write(clnt_sock, msg, strlen(msg));
    if(-1 == ret) LOG_FATAL("write() error!");
    printf("ret = %d\n", ret);

    signal(SIGPIPE, SIG_IGN);
    ret = write(clnt_sock, msg, strlen(msg));
    if(-1 == ret)
    {
        switch(errno)
        {
        case EPIPE:
            LOG_ERROR("Fd is connected to a pipe or socket whose"
                      "reading end is closed.");
            break;
        default:
            LOG_FATAL("Unknow Error!");
        };
    }

    close(clnt_sock);
    close(serv_sock);
}



int main()
{
    std::thread clientThread(Client);
    std::thread serverThread(Server);
    clientThread.join();
    serverThread.join();

    return 0;
}