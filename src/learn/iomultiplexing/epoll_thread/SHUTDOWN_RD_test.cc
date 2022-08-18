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

/// 该代码是对 SIGPIPE_test2.cc 中描述情况 (1) 的测试
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

    shutdown(sock, SHUT_RD);
    char buf[BUF_SIZE];
    int ret = read(sock, buf, BUF_SIZE);
    if(0 == ret)
    {
        printf("read return 0\n"); 
    }
    ret = recv(sock, buf, BUF_SIZE, 0);
    if(0 == ret)
    {
        printf("recv return 0\n");
    }
    printf("clientThread stop!\n");
}

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

    sleep(3);

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