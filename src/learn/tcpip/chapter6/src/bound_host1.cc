#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    char message[BUF_SIZE];
    struct sockaddr_in my_addr, your_addr;
    
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

    int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(-1 == sock) LOG_ERROR("socket() error!");

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;

#ifdef USE_DEFAULT_ADDRESS
    my_addr.sin_port = htons(2001);
#else
    my_addr.sin_port = htons(atoi(argv[1]));
#endif

    if(-1 == bind(sock, (struct sockaddr*)&my_addr, sizeof(my_addr)))
    LOG_ERROR("bind() error!");

    for(int i = 0; i < 3; ++i)
    {
        sleep(3);
        socklen_t addr_len = sizeof(your_addr);
        recvfrom(sock, message, BUF_SIZE, 0,
                (struct sockaddr*)&your_addr, &addr_len);
        
        printf("Message %d: %s\n", i + 1, message);
    }
    close(sock);

    exit(EXIT_SUCCESS);
}