#include "../../header.h"

#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    char message[BUF_SIZE] = { 0 };

    struct sockaddr_in serv_addr, clnt_addr;
    
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
    
#ifdef REUSE_SOCKET_ADDR
    int option = true;
    socklen_t optlen = sizeof(option);
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen);
#endif

    if(-1 == bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
        LOG_ERROR("bind() error!");

    if(-1 == listen(serv_sock, 5))
        LOG_ERROR("llisten() error!");

    socklen_t clnt_addr_len = sizeof(clnt_addr);
    for(int i = 0; i < 5; ++i)
    {
        // sleep(30);
        // printf("ehllo\n");
        /// 在 accept 之前即可建立 ESTABLISHED 状态
        /// 三次握手之后，tcp 连接会加入到 accept 队列，accept() 会从队列中取一个连接返回，若队列为空则阻塞
        /// accept() 阻塞的条件是没有一个完成了三次握手的连接, accept() 是从队列里取节点，每个节点都是已经完成了三次握手的
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
        if(-1 == clnt_sock) LOG_ERROR("accept() error!");

        printf("Client %d connected!\n", i + 1);

        int str_len = 0;
        while((str_len = read(clnt_sock, message, sizeof(message))) > 0)
        {
            write(clnt_sock, message, str_len);
        }
        close(clnt_sock);
    }
    close(serv_sock);
    
    exit(EXIT_SUCCESS);
}