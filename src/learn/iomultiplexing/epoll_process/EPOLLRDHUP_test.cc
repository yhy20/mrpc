#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <thread>

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

int main()
{
    int sock;
    int serv_sock, clnt_sock;   

    pid_t pid = fork();
    if(0 == pid)
    { 
        struct sockaddr_in serv_addr;
        sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serv_addr.sin_port = htons(2001);

        sleep(1);

        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        char msg[] = "hello!";
        write(sock, msg, strlen(msg));
        // uint64_t wake = 1;
        // write(wakeupFd, &wake, sizeof(wake));
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        // std::this_thread::sleep_for(std::chrono::milliseconds(6000));
        // close(sock);
        shutdown(sock, SHUT_WR);
    }
    else
    {  
        struct sockaddr_in serv_addr;
        struct sockaddr_in clnt_addr;

        serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(2001);

        bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        listen(serv_sock, 5);

        int epfd = epoll_create1(EPOLL_CLOEXEC);
        
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = serv_sock;
        struct epoll_event* events = (struct epoll_event*)malloc(sizeof(event) * EPOLL_EVENTS_SIZE);

        if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event))
            LOG_FATAL("epoll_ctl error!");

        // event.events = EPOLLIN;
        // event.data.fd = wakeupFd;
        // if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, wakeupFd, &event))
        //     LOG_FATAL("epoll_ctl error!");

        int m = 0;
        char buf[BUF_SIZE];

        // close(serv_sock);
        int loopTime = 0;
        while(true)
        {
            printf("loopTime: %d\n", ++loopTime);
            int count = epoll_wait(epfd, events, EPOLL_EVENTS_SIZE, -1);
            if(-1 == count) LOG_FATAL("epoll_wait error!");
            else if(0 == count) printf("nothing happened!");
            else
            {
                m++;
                for(int i = 0; i < count; ++i)
                {   
                    printf("fd = %d, ", events[i].data.fd);
                    PrintEvents(events[i].events);
                    if(5 == m && serv_sock == events[i].data.fd)
                    {
                        
                        socklen_t clnt_addr_len = sizeof(clnt_addr);
                        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
                        if(-1 == clnt_sock) LOG_FATAL("accept error!");
                        event.events = EPOLLIN | EPOLLRDHUP | EPOLLPRI;
                        event.data.fd = clnt_sock;
                        if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event))
                            LOG_FATAL("epoll_ctl error!");

                        printf("client: %d connected!\n", clnt_sock);
                    }
                    else
                    {
                        if(10 == m)
                        {
                            ssize_t len = read(events[i].data.fd, buf, sizeof(buf));
                            if(-1 == len) LOG_FATAL("read error!");
                            buf[len] = '\0';
                            printf("msg: %s\n", buf);
                        }
                        if(15 == m)
                        {
                            ssize_t len = read(events[i].data.fd, buf, sizeof(buf));
                            if(-1 == len) LOG_FATAL("read error!");
                            if(0 == len)
                            {
                                if(-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr))
                                    LOG_FATAL("epoll_ctl error!");
                                close(events[i].data.fd);
                                printf("client: %d disconnected!\n", clnt_sock);
                            }
                        }
                    }
                }
                if(20 == m) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            // shutdown(serv_sock, SHUT_WR);
            close(serv_sock);
        }

        close(serv_sock);
        close(clnt_sock);
        free(events);
    }

    return 0;
}