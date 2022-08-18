#include <sys/epoll.h>
#include "../header.h"

#define BUF_SIZE 30
#define EPOLL_SIZE 50

std::string EventsToString(uint32_t events);

int main(int argc, char* argv[])
{
    char buf[BUF_SIZE];
    ssize_t str_len = 0;
    int serv_sock, clnt_sock, ret;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
       
#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        fprintf(stderr, "Doesn't accept any args!");
    }
#else
    if(argc != 2)
    {
        fprintf(stderr, ("Usage: ./%s <port>", basename(argv[0])));
        exit(EXIT_FAILURE);
    }
#endif

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) LOG_FATAL("socket() fail!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_port = htons(atoi(argv[1]));
#endif
    
#ifdef SOL_SOCKET_REUSEADDR
    int opt_reuseaddr = true;
    socklen_t opt_reuseaddr_len = sizeof(opt_reuseaddr);
    ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR,
                    (void*)&opt_reuseaddr, opt_reuseaddr_len);
    if(-1 == ret) LOG_FATAL("setsockopt() failed!");
#endif

    if(-1 == bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_FATAL("bind() fail!");

    if(-1 == listen(serv_sock, 5))
        LOG_FATAL("listen() fail!");

    /**
     * #include <sys/epoll.h>
     * int epoll_create(int size)
     * int epoll_create1(int flags)
     * 
     */
    int epfd = epoll_create(EPOLL_SIZE);
    // int epollfd = epoll_create1(EPOLL_CLOEXEC);
    if(-1 == epfd) LOG_FATAL("epoll_create() fail!");

    struct epoll_event* ep_events;
    ep_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    /// typedef union epoll_data 
    /// {
    ///     void        *ptr;
    ///     int          fd;
    ///     uint32_t     u32;
    ///     uint64_t     u64;
    /// } epoll_data_t;
    ///
    /// struct epoll_event {
    ///     uint32_t     events;      /* Epoll events */
    ///     epoll_data_t data;        /* User data variable */
    /// };
    
    /// The events member is a bit set composed using the following available event types:
    /// EPOLLIN: The associated file is available for read(2) operations.
    /// EPOLLOUT: The associated file is available for write(2) operations.
    /// EPOLLRDHUP: Stream socket peer closed connection, or shut down writing half of connection.
    ///             This flag is especially useful for writing simple code to 
    ///             detect peer shutdown when using Edge Triggered monitoring.
    /// EPOLLPRI: There is urgent data available for read(2) operations.
    /// EPOLLERR: Error  condition  happened on the associated file descriptor.
    ///           epoll_wait(2) will always wait for this event; it is not necessary to set it in events.  
    /// EPOLLHUP: Hang up happened on the associated file descriptor.
    ///           epoll_wait(2) will always wait for this event; it is notnecessary to set it in events.
    /// EPOLLET: Sets the Edge Triggered behavior for the associated file
    ///          descriptor.  The default behavior for epoll is Level Triggered.
    /// EPOLLONESHOT: Sets the one-shot behavior for the associated file descriptor.
    ///               This means that after an event is pulled out with epoll_wait(2) 
    ///               the associated file descriptor is internally disabled and no other events
    ///               will be reported by the epoll interface. The user must call epoll_ctl() with EPOLL_CTL_MOD
    ///               o rearm the file descriptor with a new event mask.

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serv_sock;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
    if(-1 == ret)
    {      
        switch(errno)
        {
            case EBADF: 
                LOG_FATAL("epfd or fd is not a valid file descriptor!");
            case EEXIST:
                LOG_FATAL("op was EPOLL_CTL_ADD, but the supplied file descriptor fd" \
                          " is already registered with this epoll instance.");
            case EINVAL:
                LOG_FATAL("epfd is not an epoll file descriptor" \
                          " or fd is the same as epfd or the requested op is not supported.");
            case ENOENT:
                LOG_FATAL("op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, but the fd is not" \
                          " registered with this epoll instance.");
            case ENOMEM:
                LOG_FATAL("There was insufficient memory to handle the requested op control operation.");
            case ENOSPC:
                LOG_FATAL("The limit imposed by /proc/sys/fs/epoll/max_user_watches" \
                          " was encountered while trying to register (EPOLL_CTL_ADD)" \
                          " a new file descriptor on an epoll  instance.");
            case EPERM:
                LOG_FATAL("The target file fd does not support epoll."); 
            default:
                LOG_FATAL("unknown error of epoll_ctl()");
        }     
    }

    while(true)
    {
        int event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, 10000);
        
        if(-1 == event_cnt)
        {
            LOG_FATAL("epoll_wait() error!");
        }
        if(0 == event_cnt)
        {
            printf("timeout!\n");
            continue;
        }
        for(int i = 0; i < event_cnt; ++i)
        {
            if(ep_events[i].data.fd == serv_sock)
            {   
                printf("ep_events = %u, serv_sock %s", ep_events[i].events, EventsToString(ep_events[i].events).c_str());
                clnt_addr_len = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
                event.events = EPOLLIN | EPOLLRDHUP;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("Connected to clinet: %d\n", clnt_sock);
            }
            else
            {   
                printf("ep_events = %u, clnt_sock %s", ep_events[i].events, EventsToString(ep_events[i].events).c_str());
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if(0 == str_len)
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, nullptr);
                    close(ep_events[i].data.fd);
                    printf("Closed client: %d\n", ep_events[i].data.fd);
                }
                else
                {
                    write(ep_events[i].data.fd, buf, str_len);
                }
            }
        }

    }

    free(ep_events);
    close(serv_sock);
    exit(EXIT_SUCCESS);
}

std::string EventsToString(uint32_t events)
{
    std::string str = "events:\n";
    if(events & EPOLLIN) str += "EPOLLIN\n";
    if(events & EPOLLOUT) str += "EPOLLOUT\n";
    if(events & EPOLLRDHUP) str += "EPOLLRDHUP\n";
    if(events & EPOLLPRI) str += "EPOLLPRI\n";
    if(events & EPOLLERR) str += "EPOLLERR\n";
    if(events & EPOLLHUP) str += "EPOLLHUP\n";
    if(events & EPOLLET) str += "EPOLLET\n";
    if(events & EPOLLONESHOT) str += "  EPOLLONESHOT\n";
    
    return str;
}