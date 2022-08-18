#include "inttypes.h"
#include "../../header.h"

int main(void)
{
    
    uint32_t host_addr = 0x12345678;
    uint16_t host_port = 0x1234;
    uint32_t net_addr = htonl(host_addr);
    uint16_t net_port = htons(host_port);

    printf("Host ordered address: 0x%" PRIx32 "\n", host_addr);
    printf("Network ordered address: 0x%" PRIx32 "\n", net_addr);
    printf("Host ordered port: 0x%" PRIx16 "\n", host_port);
    printf("Network ordered port: 0x%" PRIx16 "\n", net_port);

    exit(EXIT_SUCCESS);
}