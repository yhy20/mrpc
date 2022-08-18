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

class Epoller
{
public:
    Epoller()
    {   
        epfd = epoll_create1(EPOLL_CLOEXEC);
        if(-1 == epfd) LOG_FATAL("epoll_create1() errro!");
    }

    int poll(struct epoll_event* activeEvents, int size,int timeoutMs)
    {
        return epoll_wait(epfd, activeEvents, size, timeoutMs);
    }

    void update(int op, int fd, uint32_t events)
    {
        struct epoll_event event;
        event.events = events;
        event.data.fd = fd;
        if(-1 == epoll_ctl(epfd, op, fd, &event))
        {
            switch(errno)
            {
            case EBADF: 
                LOG_FATAL("EBADF: epfd or fd is not a valid file descriptor!");
            case EEXIST:
                LOG_FATAL("EEXIST: op was EPOLL_CTL_ADD, but the supplied file descriptor fd" \
                          " is already registered with this epoll instance.");
            case EINVAL:
                LOG_FATAL("EINVAL: epfd is not an epoll file descriptor" \
                          " or fd is the same as epfd or the requested op is not supported.");
            case ENOENT:
                LOG_FATAL("ENOENT: op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, but the fd is not" \
                          " registered with this epoll instance.");
            case ENOMEM:
                LOG_FATAL("ENOMEM: There was insufficient memory to handle the requested op control operation.");
            case ENOSPC:
                LOG_FATAL("ENOSPC: The limit imposed by /proc/sys/fs/epoll/max_user_watches" \
                          " was encountered while trying to register (EPOLL_CTL_ADD)" \
                          " a new file descriptor on an epoll  instance.");
            case EPERM:
                LOG_FATAL("EPERM: The target file fd does not support epoll."); 
            default:
                LOG_FATAL("Default: unknown error of epoll_ctl()");
            }     
        }
    }

    static std::string EventsToString(uint32_t ev)
    {
        std::ostringstream oss;
        if(ev & EPOLLIN)
            oss << " IN";
        if(ev & EPOLLOUT)
            oss << " OUT";
        if(ev & EPOLLHUP)
            oss << " HUP";
        if(ev & EPOLLRDHUP)
            oss << " RDHUP";
        if(ev & EPOLLERR)
            oss << " ERR";

        return oss.str();
    }

private:   
    int epfd;
};

/**
 * 客户端在 epoll 上监听未 connect 的 socketFd 将只触发 EPOLLHUP 事件
 * 其实道理与服务器未 listening 意义，相当于直接监听没有建立连接的 socketFd
 * 连接都没有建立，就相当于挂断(Hang up)了
 */
void Client()
{
    int sock;
    // struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2001);

    int opt_nodelay = true;
    socklen_t opt_nodelay_len = sizeof(opt_nodelay);
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

    // std::unique_lock<std::mutex> lock(g_mutex);
    // while(!g_serverReady)
    // {
    //     g_cond.wait(lock);
    // }
    // lock.unlock();

    // if(-1 == connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
    //     LOG_FATAL("connect() error!");

    Epoller poller;
    poller.update(EPOLL_CTL_ADD, sock, EPOLLIN | EPOLLRDHUP);

    struct epoll_event* activeEvents = 
        (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_EVENTS_SIZE);

    int count = poller.poll(activeEvents, EPOLL_EVENTS_SIZE, 3000);
    if(-1 == count) LOG_FATAL("epoll_wait() error!");
    if(0 == count) printf("nothing happend!\n");
    else
    {
        for(int i = 0; i < count; ++i)
        {
            printf("fd = %d, events = { %s }\n",
                   activeEvents[i].data.fd,
                   Epoller::EventsToString(activeEvents[i].events).c_str()
            );
        }
    }

    sleep(3);
    close(sock);
    printf("clientThread stop!\n");
}

void Server()
{
    // int serv_sock;
    // int clnt_sock;   
    // struct sockaddr_in serv_addr;
    // struct sockaddr_in clnt_addr;

    // serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    // // int opt_nodelay = true;
    // // socklen_t opt_nodelay_len = sizeof(opt_nodelay);
    // // setsockopt(clnt_sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

    // int opt_reuseaddr = true;
    // socklen_t opt_reuseaddr_len = sizeof(opt_reuseaddr);
    // setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_reuseaddr, opt_reuseaddr_len);

    // bzero(&serv_addr, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // serv_addr.sin_port = htons(2001);
    
    // bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    // int ret = listen(serv_sock, 5);
    // if(-1 == ret) LOG_FATAL("listen() error!");
    // {
    //     std::lock_guard<std::mutex> lock(g_mutex);
    //     g_serverReady = true;
    //     g_cond.notify_one();
    // }

    // /// 阻塞调用
    // socklen_t clnt_addr_len = sizeof(clnt_addr);
    // clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

    // shutdown(clnt_sock, SHUT_RDWR);

    // Epoller poller;
    // poller.update(EPOLL_CTL_ADD, serv_sock, EPOLLIN | EPOLLRDHUP);

    // struct epoll_event* activeEvents = 
    //     (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_EVENTS_SIZE);

    // int count = poller.poll(activeEvents, EPOLL_EVENTS_SIZE, 3000);
    // if(-1 == count) LOG_FATAL("epoll_wait() error!");
    // if(0 == count) printf("nothing happend!");
    // else
    // {
    //     for(int i = 0; i < count; ++i)
    //     {
    //         printf("fd = %d, events = { %s }\n",
    //                activeEvents[i].data.fd,
    //                Epoller::EventsToString(activeEvents[i].events).c_str()
    //         );
    //     }
    // }

    // close(clnt_sock);
    // close(serv_sock);
}



int main()
{
    std::thread clientThread(Client);
    std::thread serverThread(Server);
    clientThread.join();
    serverThread.join();

    return 0;
}