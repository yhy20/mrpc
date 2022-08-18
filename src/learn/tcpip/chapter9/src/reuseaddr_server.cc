#include "../../header.h"

#define TRUE 1
#define FALSE 0

int main(int argc, char* argv[])
{
    char message[30];
    
    struct sockaddr_in serv_addr, clnt_addr;
#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args");
    }
#else
    if(argc != 2)
    {
        LOG_ERROR("Usage: ./%s <IP>", basename(argv[0]));
    }
#endif  

    int serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == serv_sock) LOG_ERROR("socket() error!");

    /// reuse address 
    int optval = TRUE;
    socklen_t optlen = sizeof(optval);
    int ret = setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&optval, optlen);
    if(-1 == ret) LOG_ERROR("setsockopt() error!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_port = htons(atoi(argv[1]));
#endif  

    if(-1 == bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_ERROR("bind() error!");

    if(-1 == listen(serv_sock, 5))
        LOG_ERROR("bind() error!");

    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
    if(-1 == clnt_sock) LOG_ERROR("accept() error!");

    int str_len = 0;
    while((str_len = read(clnt_sock, message, sizeof(message))) > 0)
    {
        write(clnt_sock, message, str_len);
        write(STDOUT_FILENO, message, str_len);
    }

    close(clnt_sock);
    close(serv_sock);
    exit(EXIT_SUCCESS);
}