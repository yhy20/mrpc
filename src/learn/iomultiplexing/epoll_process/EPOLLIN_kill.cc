#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <thread>
#include <exception>

#include "../header.h"

#define BUF_SIZE 100
#define EPOLL_EVENTS_SIZE 10

void PrintEvents(uint32_t events)
{   
    printf("events: ");
    if(events & EPOLLIN) printf("IN ");
    if(events & EPOLLOUT) printf("OUT ");
    if(events & EPOLLRDHUP) printf("RDHUP ");
    if(events & EPOLLPRI) printf("PRI ");
    if(events & EPOLLHUP) printf("HUP ");    
    if(events & EPOLLET) printf("ET ");
    if(events & EPOLLONESHOT) printf("ONESHOT ");
    printf("\n");
    fflush(stdout);
}

/// 关于 EPOLLIN 何时触发
/// 


/// The program displays when the EPOLLIN, EPOLLRDHUP, EPOLLHUP events will be triggered.
/// "///" means ordinary comments.
/// "//" means temporary comments on the code in order to test different situations.

/// TODO: 知道进程之间的影响，fork 后一个进程退出对其他进程的影响
int main()
{
    pid_t pid = fork();
    if(0 == pid)
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

        /// wait for server listening.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        /// wait for epoll_ctl and epoll_wait.
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        /// send msg to triggers EPOLLIN.
        char msg[] = "hello!";
        write(sock, msg, strlen(msg));
        
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        /// close or shutdown to triggers EPOLLIN and EPOLLRDHUP.
        // shutdown(sock, SHUT_WR);
        throw std::logic_error("unknown");
        // close(sock);
    }
    else
    {  
        int serv_sock, clnt_sock;   
        struct sockaddr_in serv_addr;
        struct sockaddr_in clnt_addr;

        serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(2001);

        bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        listen(serv_sock, 5);

        socklen_t clnt_addr_len = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

        // int opt_nodelay = true;
        // socklen_t opt_nodelay_len = sizeof(opt_nodelay);
        // setsockopt(clnt_sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_nodelay, opt_nodelay_len);

        int epfd = epoll_create1(EPOLL_CLOEXEC);
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLRDHUP;
        event.data.fd = clnt_sock;
        struct epoll_event* events = (struct epoll_event*)malloc(sizeof(event) * EPOLL_EVENTS_SIZE);

        // shutdown(clnt_sock, SHUT_WR);

        /// close(clnt_sock) before epoll_ctl will lead to EINVAL error.
        // close(clnt_sock);

        if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event))
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

        int m = 0;
        int loopTime = 0;
        while(true)
        {
            m++;
            printf("loopTime: %d, m = %d\n", ++loopTime, m);
            int count = epoll_wait(epfd, events, EPOLL_EVENTS_SIZE, 0);
            if(-1 == count) LOG_FATAL("epoll_wait error!");
            else if( 0 == count) printf("nothing happened!\n");
            else
            {
                for(int i = 0; i < count; ++i)
                {
                    printf("client socketFd = %d, ", events[i].data.fd);
                    PrintEvents(events[i].events);          
                }
                if(10 == m) break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        close(serv_sock);
        close(clnt_sock);
        free(events);
    }

    return 0;
}