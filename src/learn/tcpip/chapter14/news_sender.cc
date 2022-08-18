// #include "../header.h"

// #define TTL 64
// #define BUF_SIZE 100

// int main(int argc, char* argv[])
// {
//     char buf[BUF_SIZE];
//     struct sockaddr_in multi_addr;

// #ifdef USE_DEFAULT_ADDRESS
//     if(argc != 1)
//     {
//         LOG_ERROR("Doesn't accept any args");
//     }
// #else
//     if(argc != 3)
//     {
//         LOG_ERROR("Usage: ./%s <GroupIP> <port>", basename(argv[0]));
//     }
// #endif  
//     int send_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if(-1 == send_sock) LOG_ERROR("socket() error!");

//     bzero(&multi_addr, sizeof(multi_addr));
//     multi_addr.sin_family = AF_INET;
// #ifdef USE_DEFAULT_ADDRESS
//     multi_addr.sin_addr.s_addr = inet_addr("224.1.1.2");
//     multi_addr.sin_port = htons(2001);
// #else
//     multi_addr.sin_addr.s_addr = inet_addr(argv[1]);
//     multi_addr.sin_port = htons(atoi(argv[2]));
// #endif

//     int opt_time_to_live = TTL;
//     int ret = setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL,
//                 (void*)&opt_time_to_live, sizeof(opt_time_to_live));
//     if(-1 == ret) LOG_ERROR("setsockopt() error!");

//     FILE* fp = nullptr;
//     if(nullptr == (fp = fopen("../news.txt", "r")))
//         LOG_ERROR("fopen() error!");
    
//     while(!feof(fp))
//     {
//         printf("hello!\n");
//         fgets(buf, BUF_SIZE, fp);
//         ssize_t str_len = sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr*)&multi_addr, sizeof(multi_addr));
//         if(-1 == str_len) LOG_ERROR("sendto() error!");

//         sleep(2);
//     }   
//     fclose(fp);
//     close(send_sock);
//     exit(EXIT_SUCCESS);
// }


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TTL 64
#define BUF_SIZE 30
void error_handling(const char *message);

int main(int argc, char *argv[])
{
    int send_sock;
    struct sockaddr_in mul_adr;
    int time_live = TTL;
    FILE *fp;
    char buf[BUF_SIZE];
    if (argc != 3)
    {
        printf("Usage : %s <GroupIP> <PORT>\n", argv[0]);
        exit(1);
    }
    send_sock = socket(PF_INET, SOCK_DGRAM, 0); //创建  UDP 套接字
    memset(&mul_adr, 0, sizeof(mul_adr));
    mul_adr.sin_family = AF_INET;
    mul_adr.sin_addr.s_addr = inet_addr(argv[1]); //必须将IP地址设置为多播地址
    mul_adr.sin_port = htons(atoi(argv[2]));
    //指定套接字中 TTL 的信息
    setsockopt(send_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&time_live, sizeof(time_live));
    if ((fp = fopen("../news.txt", "r")) == NULL)
        error_handling("fopen() error");

    while (!feof(fp)) //如果文件没结束就返回0
    {
        fgets(buf, BUF_SIZE, fp);
        sendto(send_sock, buf, strlen(buf), 0, (struct sockaddr *)&mul_adr, sizeof(mul_adr));
        sleep(2);
    }
    fclose(fp);
    close(send_sock);
    return 0;
}

void error_handling(const char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}