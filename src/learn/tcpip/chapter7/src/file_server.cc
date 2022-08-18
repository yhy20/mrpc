#include "../../header.h"

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    char buf[BUF_SIZE] = { 0 };

    struct sockaddr_in serv_addr, clnt_addr;

#ifdef USE_DEFAULT_ADDRESS
    if(argc != 1)
    {
        LOG_ERROR("Doesn't accept any args");
    }
#else
    if(argc != 2)
    {
        LOG_ERROR("Usage: ./%s <port>", basename(argv[0]));
    }
#endif  

    int serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
    if(-1 == clnt_sock) LOG_ERROR("accept() error!");
    
    FILE* fp = fopen("./src/poetry.txt", "rb");

    // while(true)
    // {
    //     int read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);
    //     if(read_cnt < BUF_SIZE)
    //     {
    //         write(clnt_sock, buf, read_cnt);
    //         break; 
    //     }
    //     write(clnt_sock, buf, BUF_SIZE);
    // }

    struct stat statbuf;
    stat("./src/poetry.txt", &statbuf);
    off_t file_size = statbuf.st_size;
    off_t write_size = 0;

    while(write_size < file_size)
    {
        fread((void*)buf, BUF_SIZE, 1, fp);
        ssize_t len = write(clnt_sock, buf, std::min((off_t)BUF_SIZE, file_size - write_size));
        if(-1 == len) LOG_ERROR("write() error!");
        write_size += len;
    }

    shutdown(clnt_sock, SHUT_WR);
    /// netstat -natp | grep -E '127.0.0.1:2001|0.0.0.0:2001|Proto'
    /// sleep 用于观察服务器进入 FIN_WAIT2 状态，如下所示
    /// Proto Recv-Q Send-Q Local Address           Foreign Address         State       PID/Program name
    /// tcp       11      0 127.0.0.1:2001          127.0.0.1:34608         FIN_WAIT2   1250/./file_server
    sleep(10);
    ssize_t len = read(clnt_sock, buf, BUF_SIZE);
    if(-1 == len) LOG_ERROR("read() error!");
    printf("Message from client: %s\n", buf);

    fclose(fp);
    close(clnt_sock);
    close(serv_sock);

    exit(EXIT_SUCCESS);   
}