#include "../../header.h"

//  struct hostent
//  {
//      char    *h_name;		/* Official name of host.  */
//      char    **h_aliases;	/* Alias list.  */
//      int     h_addrtype;		/* Host address type.  */
//      int     h_length;	    /* Length of address.  */
//      char    **h_addr_list;  /* List of addresses from name server.  */
//  };

int main(int argc, char* argv[])
{

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

    struct sockaddr_in addr;
    struct hostent *host;

    bzero(&addr, sizeof(addr));

/**
 * 8.8.8.8
 * 
 */

/// TODO: 解决 getnamebyname 获取的 IP 不能被 getnamebyaddr 解析的问题

/// 由 IP 地址获取域相关信息
#ifdef USE_DEFAULT_ADDRESS
    addr.sin_addr.s_addr = inet_addr("110.242.68.4");
#else
    addr.sin_addr.s_addr = inet_addr(argv[1]);
#endif 

    host = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);    
    if(host == nullptr) LOG_ERROR("gethostname() error!");

    printf("Official name: %s\n", host->h_name);
    for(int i = 0; host->h_aliases[i] != nullptr ; ++i)
    {
        printf("Aliases %d: %s\n", i + 1, host->h_aliases[i]);
    }
    printf("Address type: %s\n",
           (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");
    for(int i = 0; host->h_addr_list[i] != nullptr ; ++i)
    {
        printf("IP addr %d: %s\n", i + 1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }

    exit(EXIT_SUCCESS);
}