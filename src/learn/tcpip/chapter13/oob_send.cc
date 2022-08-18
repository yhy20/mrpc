#include "../header.h"

int main(int argc, char* argv[])
{

    struct sockaddr_in recv_addr;

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
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == sock) LOG_ERROR("socket() error!");
    bzero(&recv_addr, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;

#ifdef USE_DEFAULT_ADDRESS
    recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    recv_addr.sin_port = htons(2001);
#else
    recv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    recv_addr.sin_port = htons(atoi(argv[2]));
#endif

    if(-1 == connect(sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)))
        LOG_ERROR("connect() error!");
    
    sleep(3);
    write(sock, "123", strlen("123"));
    send(sock, "4", strlen("4"), MSG_OOB);
    write(sock, "567", strlen("567"));
    send(sock, "890", strlen("890"), MSG_OOB);
    close(sock);

    exit(EXIT_SUCCESS);
}