#include "inttypes.h"
#include "../../header.h"

void ConvertTest(const char* addr)
{
    in_addr_t conv_addr = inet_addr(addr);
    if(INADDR_NONE == conv_addr)
    {
        LOG_ERROR("inet_addr() error!");
    }
    else
    {
        printf("Network ordered integer addr: 0x%" PRIx32 "\n",
                static_cast<uint32_t>(conv_addr));
    }
}

int main(void)
{
    const char* addr1 = "127.0.0.1";
    const char* addr2 = "0.0.0.0";
    const char* addr3 = "1.2.3.4";
    const char* addr4 = "1.2.3.256";

    ConvertTest(addr1);
    ConvertTest(addr2);
    ConvertTest(addr3);
    ConvertTest(addr4);

    exit(EXIT_SUCCESS);
}