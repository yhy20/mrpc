#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    char msg1[] = "Hi!";
    char msg2[] = "I'm another UDP host!";
    char msg3[] = "Nice to meet you!";

    struct sockaddr_in your_addr;
    
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
    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(-1 == sock) LOG_ERROR("socket error!");

    bzero(&your_addr, sizeof(your_addr));
    your_addr.sin_family = AF_INET;

#ifdef USE_DEFAULT_ADDRESS
    your_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    your_addr.sin_port = htons(2001);
#else
    your_addr.sin_addr.s_addr = inet_addr(argv[1]);
    your_addr.sin_port = htons(atoi(argv[2]));
#endif

    sendto(sock, msg1, sizeof(msg1), 0,
        (struct sockaddr*)&your_addr, sizeof(your_addr));
    
    sendto(sock, msg2, sizeof(msg2), 0,
        (struct sockaddr*)&your_addr, sizeof(your_addr));

    sendto(sock, msg3, sizeof(msg3), 0,
        (struct sockaddr*)&your_addr, sizeof(your_addr));

    close(sock);

    exit(EXIT_SUCCESS);
}