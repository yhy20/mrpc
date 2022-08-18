#include <sys/epoll.h>

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

int sock;
int serv_sock, clnt_sock;

int main()
{
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
        char msg[] = "Hello!";
        printf("sock == %d\n", sock);
        write(sock, msg, strlen(msg));
        close(sock);
        
        sleep(5);
        close(clnt_sock);
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

        socklen_t clnt_addr_len = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

        sleep(3);
        int epfd = epoll_create1(EPOLL_CLOEXEC);

        struct epoll_event event;
        event.events = EPOLLIN | EPOLLRDHUP;
        event.data.fd = clnt_sock;
        struct epoll_event* events = (struct epoll_event*)malloc(sizeof(event) * EPOLL_EVENTS_SIZE);

        if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event))
            LOG_FATAL("epoll_ctl error!");
        
        int m = 0;
        char buf[BUF_SIZE];

        // close(serv_sock);

        while(true)
        {
            int count = epoll_wait(epfd, events, EPOLL_EVENTS_SIZE, -1);
            if(-1 == count) LOG_FATAL("epoll_wait error!");
            else
            {
                for(int i = 0; i < count; ++i)
                {
                    PrintEvents(events[i].events);
                    if(5 == ++m)
                    {
                        int readBytes = read(events[i].data.fd, buf, sizeof(buf));
                        if(-1 == readBytes) 
                        {
                            break;
                        }
                        else
                        {
                            buf[readBytes] = '\0';
                            printf("%s\n", buf);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                
                
            }   
        }

        close(serv_sock);
        close(clnt_sock);
        free(events);
    }

    return 0;
}