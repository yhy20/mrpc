#include "../../header.h"

/// 修改 IP 可在公网上简单测试代码
/// TCP 协议传输无边界字节流，（TODO: 包截断， 粘包问题详解）
int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size;
    char message[] = "Hello World!";

#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args");
    }
#else
    if(argc != 2)
    {
        LOG_ERROR("Usage: %s <port>", basename(argv[0]));
    }
#endif

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) LOG_ERROR("socket() error!");

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef USE_DEFAULT_ADDRESS
    serv_addr.sin_port = htons(2001);
#else
    serv_addr.sin_port = htons(atoi(argv[1]));
#endif
    
#ifdef USE_DEBUG_MODE
    int option = true;
    socklen_t optlen = sizeof(option);
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
#endif

    if(-1 == bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_ERROR("bind() error!");

    if(-1 == listen(serv_sock, 5))
        LOG_ERROR("llisten() error!");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_addr, &clnt_addr_size);
    if(-1 == clnt_sock)
        LOG_ERROR("accept() error!");

    write(clnt_sock, message, sizeof(message));
    close(clnt_sock);
    close(serv_sock);
    return 0;
}