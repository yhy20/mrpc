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

void Client()
{
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2001);

    // int opt_nodelay = true;
    // socklen_t opt_nodelay_len = sizeof(opt_nodelay);
    // setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

    std::unique_lock<std::mutex> lock(g_mutex);
    while(!g_serverReady)
    {
        g_cond.wait(lock);
    }
    lock.unlock();

    if(-1 == connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_FATAL("connect() error!");

    close(sock);
}

/**
 * ????????? epoll_wait ????????? close(clnt_sock)?????? epoll set ????????? clnt_sock, ???????????????????????? 
 */
void Server()
{
    int serv_sock, clnt_sock;   
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
    listen(serv_sock, 5);

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_serverReady = true;
        g_cond.notify_one();
    }

    /// ????????????
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

    Epoller poller;
    poller.update(EPOLL_CTL_ADD, clnt_sock, EPOLLIN | EPOLLRDHUP);

    /**
     * ??? epoll_ctl ?????? clnt_sock ????????? clnt_sock
     * ????????????????????? close ????????? epollfd ?????????????????? man 7 epoll Q6 A6
     * Will closing a file descriptor cause it to be removed from all epoll sets automatically?
     * 
     * Yes, but be aware of the following point.
     * A file descriptor is a reference to an open file description (see open(2)).
     * Whenever a descriptor is duplicated via dup(2), dup2(2), fcntl(2) F_DUPFD, 
     * or fork(2), a new file descriptor referring to the same open file description
     * is created. An open file description continues to exist until all file descriptors
     * referring to it have been closed. A file descriptor is removed from an epoll
     * set only after all the file descriptors referring to the underlying open file
     * description have been closed (or before if the descriptor is explicitly removed
     * using epoll_ctl(2) EPOLL_CTL_DEL). This means that even after a file descriptor
     * that is part of an epoll set has been closed, events may be reported for that file
     * descriptor if other file descriptors referring to the same underlying file descr
     * -iption remain open.
     * 
     * ?????? clnt_sock ?????????????????? 1????????? close ?????? clnt_sock ????????? epoll set ?????????
     * ????????? epoll ???????????????????????????
     */
    close(clnt_sock); 
    sleep(1);

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