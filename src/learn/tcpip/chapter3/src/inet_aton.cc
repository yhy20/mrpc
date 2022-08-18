#include "inttypes.h"
#include "../../header.h"

void StringToAddress(const char* addr)
{
    struct sockaddr_in addr_inet;
    if(!inet_aton(addr, &addr_inet.sin_addr))
    {
        LOG_ERROR("inet_addr() error!");
    }
    else
    {
        printf("Network ordered integer addr: 0x%" PRIx32 "\n",
                static_cast<uint32_t>(addr_inet.sin_addr.s_addr));
    }
}

int main(void)
{
    const char* addr1 = "127.0.0.1";
    const char* addr2 = "0.0.0.0";
    const char* addr3 = "1.2.3.4";
    const char* addr4 = "1.2.3.256";

    
    StringToAddress(addr1);
    StringToAddress(addr2);
    StringToAddress(addr3);
    StringToAddress(addr4);

    exit(EXIT_SUCCESS);
}