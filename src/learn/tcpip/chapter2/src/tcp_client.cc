#include "../../header.h"

int main(int argc, char* argv[])
{
    struct sockaddr_in serv_addr;
    char message[30] = { 0 };

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

    int str_len = 0;
    int read_len = 0, idx = 0;
    while((read_len = read(sock, &message[idx++], 1)) > 0)
    {
        if(-1 == read_len) LOG_ERROR("read() error!");

        str_len += read_len;
    }

    printf("message from server: %s\n", message);
    printf("function read() call count = %d\n", str_len);
    close(sock);
    return 0;
}