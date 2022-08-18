#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void ErrorHandling(const char* errMsg)
{
    fprintf(stderr, "%s\n", errMsg);
    exit(EXIT_FAILURE);
}

int main(void)
{   
    char buf[] = "Let's go!\n";
    int fd = open("data.txt", O_CREAT | O_WRONLY | O_TRUNC);
    if(-1 == fd) ErrorHandling("open() error!");
    printf("file descriptor = %d\n", fd);

    /// sizeof(buf) - 1 避免写入 '\0' 字符
    if(-1 == write(fd, buf, sizeof(buf) - 1))
    {
        ErrorHandling("write() error!");
    }

    close(fd);
    exit(0);
}