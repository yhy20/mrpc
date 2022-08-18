#include "../../header.h"

int main(void)
{
    int tcp_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int udp_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    printf("SOCK_STREAM: %d\n", SOCK_STREAM);
    printf("SOCK_DGRAM: %d\n", SOCK_DGRAM);

    /**
     * int getsockopt (int sock, int level, int optname, void *optval, socklen_t *optlen)
     * 
     * @brief 查看套接字的相关信息
     * @param[in] sock 指定要查看选项的套接字文件描述符
     * @param[in] level 要查看可选项的协议层（SOL_SOCKET or IPPROTO_IP or IPPROTO_TCP)
     * @param[in] optname 要查看的可选项名
     * @param[out] optval 指定查看结果的缓冲地址值
     * @param[inout] optlen 向第四个参数 optval 传递缓冲大小，调用函数后保存通过第四个参数返回的选项信息字节数
     * @return 成功时返回 0，失败返回 -1
     * 
     * PS: SOL_SOCKET 表示套接字相关通用选项，IPPROTO_IP 表示 IP 协议相关选项，IPPROTO_TCP 表示 TCP 协议相关选项
     */

    int sock_type = 0;
    socklen_t optlen = sizeof(sock_type);

    /// 指定 SOL_SOCKET 层的套接字类型信息
    int ret = getsockopt(tcp_sock, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
    if(-1 == ret) LOG_ERROR("getsockopt() error!");
    printf("Socket type one: %d\n", sock_type);

    sock_type = 0;

    ret = getsockopt(udp_sock, SOL_SOCKET, SO_TYPE, (void*)&sock_type, &optlen);
    if(-1 == ret) LOG_ERROR("getsockopt() error!");
    printf("Socket type two: %d\n", sock_type);

    exit(EXIT_SUCCESS);
}