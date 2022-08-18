#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    int serv_sock;
    char message[BUF_SIZE];

    struct sockaddr_in serv_addr, clnt_addr;

#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args!");
    }
#else
    if(argc != 2)
    {
        LOG_ERROR("Usage: %s <port>", basename(argv[0]));
    }
#endif  
    serv_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(-1 == serv_sock) LOG_ERROR("UDP socket creation error!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_port = htons(atoi(argv[1]));
#endif

    if(-1 == bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
    LOG_ERROR("bind() error!");

/// TODO: mode

    while(true)
    {
        socklen_t clnt_addr_len = sizeof(clnt_addr);
        int str_len = recvfrom(serv_sock, message, BUF_SIZE, 0,
                                (struct sockaddr*)&clnt_addr, &clnt_addr_len);
        sendto(serv_sock, message, str_len, 0,
                                (struct sockaddr*)&clnt_addr, sizeof(serv_addr));
    }
    close(serv_sock);

    exit(EXIT_SUCCESS);
}