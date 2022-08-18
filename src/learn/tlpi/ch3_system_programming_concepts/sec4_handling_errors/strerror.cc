#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "../../header.h"

int main(void)
{
    int fd = 100;
    int one = 1;
    ssize_t len = write(fd, &one, sizeof(one));
    if(-1 == len) 
    {   
        printf("write: %s\n", strerror(errno));
        
    }

#if(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
    // strerror_r();
#else
    // strerror_r();
#endif

}