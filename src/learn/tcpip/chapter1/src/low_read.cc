#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 100

void ErrorHandling(const char* errMsg)
{
    fprintf(stderr, "%s\n", errMsg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    char buf[BUF_SIZE];
    memset(buf, 0, sizeof(buf));

    int fd = open("data.txt", O_RDONLY);
    if(-1 == fd) ErrorHandling("open() error!");
    printf("file descriptor = %d\n", fd);

    if(read(fd, buf, sizeof(buf) - 1) == -1)
    {
        ErrorHandling("read() error!");
    }
    printf("file data: %s\n", buf);
    
    exit(0);
}
