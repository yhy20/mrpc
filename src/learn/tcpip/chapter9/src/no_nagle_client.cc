#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
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

    FILE* fp = fopen("receive.txt", "wb");

    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == sock) LOG_ERROR("UDP socket creation error!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
#endif  

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    int read_size = 0;
    while((read_size = read(sock, buf, BUF_SIZE)) > 0)
    {
        fwrite(buf, read_size, 1, fp);
    }

    puts("Received file data");   
    close(sock);
    exit(EXIT_SUCCESS);
}