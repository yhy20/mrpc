#include "../header.h"

#define BUF_SIZE 60

void read_routine(int sock, char* buf)
{
    ssize_t str_len = 0;
    while(true)
    {
        str_len = read(sock, buf, BUF_SIZE);
        if(-1 == str_len) LOG_FATAL("read() error!");
        if(0 == str_len) return;

        buf[str_len] = '\0';
        printf("Message from server: %s", buf);
    }
}

void write_routine(int sock, char* buf)
{
    while(true)
    {
        // fputs("Input message(Q or q to quit): \n", stdout);
        fgets(buf, BUF_SIZE, stdin);
        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
        {
            shutdown(sock, SHUT_WR);
            return;
        }
        write(sock, buf, strlen(buf));
    }
}

int main(int argc, char* argv[])
{
    int sock;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_addr;

#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args");
    }
#else
    if(argc != 3)
    {
        LOG_ERROR("Usage: %s <IP> <port>", basename(argv[0]));
    }
#endif

    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1) LOG_ERROR("socket() error!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
   
#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
#endif

    if(-1 == connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_FATAL("connect() error!");
    
    pid_t pid = fork();
    if(0 == pid)
    {   
        write_routine(sock, buf);
    }
    else
    {
        read_routine(sock, buf);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}