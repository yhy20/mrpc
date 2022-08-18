#include "../../header.h"

#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE] = { 0 };
    
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

    int sock = socket(PF_INET, SOCK_STREAM, 0);
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
        LOG_ERROR("connect() error!");

    puts("Connected......");

    while(true)
    {
        fputs("Input message(Q or q to quit): ", stdout);
        fgets(message, BUF_SIZE, stdin);
        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }
        int str_len = write(sock, message, strlen(message));
        int recv_len = 0;
        /// 解决截断和粘包问题
        while(recv_len < str_len)
        {
            int recv_cnt = read(sock, &message[recv_len], BUF_SIZE - 1);
            if(-1 == recv_cnt) LOG_ERROR("read() error!");
            recv_len += recv_cnt;
        }
        message[str_len] = '\0';
        printf("Message from server: %s", message);
    }
    close(sock);

    exit(EXIT_SUCCESS);
}