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
        LOG_ERROR("Usage: ./%s <address>", basename(argv[0]));
    }
#endif    

    struct hostent *host;

    /**
     * www.qq.com
     * www.google.com
     * www.yahoo.com
     * www.163.com
     * www.baidu.com
     * 
     * iptv.tsinghua.edu.cn
     * ipv6test.google.com
     */

/// 由域名获取包括 IP 地址在内的域相关信息
#ifdef USE_DEFAULT_ADDRESS
    host = gethostbyname("www.baidu.com");
#else
    host = gethostbyname(argv[1]);
#endif 

    
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