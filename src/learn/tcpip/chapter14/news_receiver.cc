// #include "../header.h"

// #define BUF_SIZE 100

// int main(int argc, char* argv[])
// {
//     char buf[BUF_SIZE];
//     struct sockaddr_in recv_addr;
//     struct ip_mreq join_addr;

// #ifdef USE_DEFAULT_ADDRESS
//     if(argc != 1)
//     {
//         LOG_ERROR("Doesn't accept any args");
//     }
// #else
//     if(argc != 3)
//     {
//         LOG_ERROR("Usage: %s <GroupIP> <port>", basename(argv[0]));
//     }
// #endif  
//     int recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
//     if(-1 == recv_sock) LOG_ERROR("socket() error!");

//     bzero(&recv_addr, sizeof(recv_addr));
//     recv_addr.sin_family = AF_INET;
//     recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//     join_addr.imr_interface.s_addr = htonl(INADDR_ANY);

// #ifdef USE_DEFAULT_ADDRESS
//     recv_addr.sin_port = htons(2001);
//     join_addr.imr_multiaddr.s_addr = inet_addr("224.1.1.2");
// #else
//     recv_addr.sin_port = htons(atoi(argv[2]));
//     join_addr.imr_multiaddr.s_addr = inet_addr(argv[1]);
// #endif

//     int ret = setsockopt(recv_sock, IPPROTO_IP,
//         IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr));
//     if(-1 == ret) LOG_ERROR("setsockopt() error!");
//     while(true)
//     {
//         printf("hello!\n");
//         ssize_t str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, nullptr, 0);
//         if(-1 == str_len) LOG_ERROR("recvfrom() error!");
//         if(0 == str_len) break;
//         buf[str_len] = '\0';
//         fputs(buf, stdout);
//     }

//     close(recv_sock);
//     exit(EXIT_SUCCESS);
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int recv_sock;
    int str_len;
    char buf[BUF_SIZE];
    struct sockaddr_in adr;
    struct ip_mreq join_adr;
    if (argc != 3)
    {
        printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
        exit(1);
    }
    recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(atoi(argv[2]));

    if (bind(recv_sock, (struct sockaddr *)&adr, sizeof(adr)) == -1)
        error_handling("bind() error");
    //初始化结构体
    join_adr.imr_multiaddr.s_addr = inet_addr(argv[1]); //多播组地址
    join_adr.imr_interface.s_addr = htonl(INADDR_ANY);  //待加入的IP地址
    //利用套接字选项 IP_ADD_MEMBERSHIP 加入多播组，完成了接受指定的多播组数据的所有准备
    setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&join_adr, sizeof(join_adr));

    while (1)
    {
        //通过 recvfrom 函数接受多播数据。如果不需要知道传输数据的主机地址信息，可以向recvfrom函数的第5 6参数分贝传入 NULL 0
        str_len = recvfrom(recv_sock, buf, BUF_SIZE - 1, 0, NULL, 0);
        if (str_len < 0)
            break;
        buf[str_len] = 0;
        fputs(buf, stdout);
    }
    close(recv_sock);
    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}