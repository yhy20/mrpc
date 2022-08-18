#include "../header.h"

#define BUF_SIZE 30


int acpt_sock;
int recv_sock;

void urg_hander(int signo)
{
    char buf[BUF_SIZE];
    ssize_t str_len = recv(recv_sock, buf, sizeof(buf) - 1, MSG_OOB);
    buf[str_len] = '\0';
    printf("Urgent message: %s\n", buf);
}

int main(int argc, char* argv[])
{   
    char buf[BUF_SIZE];
    struct sockaddr_in recv_addr, send_addr;
#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args");
    }
#else
    if(argc != 2)
    {
        LOG_ERROR("Usage: ./%s <port>", basename(argv[0]));
    }
#endif 

    struct sigaction act;
    act.sa_handler = urg_hander;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    acpt_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == acpt_sock) LOG_ERROR("socket() error!");

    bzero(&recv_addr, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef USE_DEFAULT_ADDRESS
    recv_addr.sin_port = htons(2001);
#else
    recv_addr.sin_port = inet_addr(argv[1]);
#endif

#ifdef SOL_SOCKET_REUSEADDR
    int opt_reuseaddr = true;
    socklen_t opt_reuseaddr_len = sizeof(opt_reuseaddr);
    setsockopt(acpt_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&opt_reuseaddr, opt_reuseaddr_len);
#endif

    if(-1 == bind(acpt_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)))
        LOG_ERROR("bind() error!");
    if(-1 == listen(acpt_sock, 5))
        LOG_ERROR("listen() error!");
    
    socklen_t send_addr_len = sizeof(send_addr_len);
    recv_sock = accept(acpt_sock, (struct sockaddr*)&send_addr, &send_addr_len);

    
    fcntl(recv_sock, F_SETOWN, getpid());
    int ret = sigaction(SIGURG, &act, 0);
    if(-1 == ret) LOG_ERROR("sigaction() error!");
    
    ssize_t str_len = 0;
    while((str_len = recv(recv_sock, buf, sizeof(buf), 0)) != 0)
    {
        if(-1 == str_len)
        {
            continue;        
        }
        buf[str_len] = '\0';
        puts(buf);
    }

    close(acpt_sock);
    close(recv_sock);

    exit(EXIT_SUCCESS);
}