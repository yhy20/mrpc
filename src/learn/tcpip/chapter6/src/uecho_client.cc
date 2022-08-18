#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    char message[BUF_SIZE] = { 0 };

    struct sockaddr_in serv_addr, from_addr;

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

    while(true)
    {
        fputs("Input message(Q or q to quit): ", stdout);
        fgets(message, sizeof(message), stdin);
        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }

        sendto(sock, message, strlen(message), 0,
                (struct sockaddr*)&serv_addr, sizeof(serv_addr));

        socklen_t from_addr_len = sizeof(from_addr);
        int str_len = recvfrom(sock, message, BUF_SIZE, 0,
                                (struct sockaddr*)&from_addr, &from_addr_len);
        message[str_len] = '\0';
        printf("Message from server: %s", message);
    }
    close(sock);

    exit(EXIT_SUCCESS);
}