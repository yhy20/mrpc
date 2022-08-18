#include "apue.h"

#define BUFFSIZE 32             

int main()
{
    int n;
    char buf[BUFFSIZE];
    int infd = open("./dir/data.txt", O_RDONLY);
    if(-1 == infd) LOG_ERROR("infd open() error");

    mode_t old =  umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int outfd = open("./dir/data_copy.txt", O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
    if(-1 == outfd) LOG_ERROR("outfd open() error");
#undef RWRWRW
    umask(old);

    while((n = read(infd, buf, BUFFSIZE)) > 0)
        if(write(outfd, buf, n) != n) 
            LOG_ERROR("write() error");

    if(n < 0) LOG_ERROR("read() error");

    return 0;
}