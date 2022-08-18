#include "inttypes.h"
#include "../../header.h"

void StringToAddress(const char* addr, sockaddr_in* addr_inet)
{
    
    if(!inet_aton(addr, &addr_inet->sin_addr))
    {
        LOG_ERROR("inet_addr() error!");
    }
    else
    {
        printf("Network ordered integer addr: 0x%" PRIx32 "\n",
                static_cast<uint32_t>(addr_inet->sin_addr.s_addr));
    }
}

void AddressToString(sockaddr_in* addr_inet)
{
    const char* str = inet_ntoa(addr_inet->sin_addr);
    if(nullptr == str)
    {
        LOG_ERROR("inet_ntoa() error!");
    }
    else
    {
        printf("Dotted-address: %s\n", str);
    }
    
}

int main(void)
{
    const char* addr1 = "127.0.0.1";
    const char* addr2 = "0.0.0.0";
    const char* addr3 = "1.2.3.4";
    const char* addr4 = "1.2.3.256";

    struct sockaddr_in addr_inet;
    StringToAddress(addr1, &addr_inet);
    AddressToString(&addr_inet);

    StringToAddress(addr2, &addr_inet);
    AddressToString(&addr_inet);

    StringToAddress(addr3, &addr_inet);
    AddressToString(&addr_inet);

    StringToAddress(addr4, &addr_inet);
    AddressToString(&addr_inet);

    exit(EXIT_SUCCESS);
}