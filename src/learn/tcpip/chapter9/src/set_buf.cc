#include "../../header.h"

int main(void)
{
    int send_buf = 1024 * 3, recv_buf = 1024 * 3;
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    /**
     * int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
     * 
     * @brief 设置套接字的相关信息
     * @param[in] sock 指定要设置选项的套接字文件描述符
     * @param[in] level 要设置的可选项协议层（SOL_SOCKET or IPPROTO_IP or IPPROTO_TCP)
     * @param[in] optname 要设置的可选项名
     * @param[in] optval 指定要更改选项的缓冲地址值
     * @param[in] optlen 向第四个参数 optval 传递可选项信息的字节数
     * @return 成功时返回 0，失败返回 -1
     * 
     * PS: SOL_SOCKET 表示套接字相关通用选项，IPPROTO_IP 表示 IP 协议相关选项，IPPROTO_TCP 表示 TCP 协议相关选项
     */

    int ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&recv_buf, sizeof(recv_buf));
    if(-1 == ret) LOG_ERROR("setsockopt() error!");

    ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&send_buf, sizeof(send_buf));
    if(-1 == ret) LOG_ERROR("setsockopt() error!");

    socklen_t optlen = sizeof(send_buf);
    ret = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&recv_buf, &optlen);
    if(-1 == ret) LOG_ERROR("setsockopt() error!");

    ret = getsockopt(sock ,SOL_SOCKET, SO_RCVBUF, (void*)&recv_buf, &optlen);
    if(-1 == ret) LOG_ERROR("setsockopt() error!");

    printf("Input buffer size: %d\n", recv_buf);
    printf("Output buffer size: %d\n", send_buf);

    exit(EXIT_SUCCESS);
}