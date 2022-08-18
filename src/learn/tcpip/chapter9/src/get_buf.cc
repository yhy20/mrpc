#include "../../header.h"

int main(void)
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    int send_buf, recv_buf; 
    socklen_t optlen = sizeof(send_buf);
    int ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&send_buf, &optlen);
    if(-1 == ret) LOG_ERROR("getsockopt() error!");

    optlen = sizeof(recv_buf);
    ret = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&recv_buf, &optlen);
    if(-1 == ret) LOG_ERROR("getsockopt() error!");

    printf("Input buffer size: %d\n" \
           "Output buffer size: %d\n", recv_buf, send_buf);
    exit(0);
}