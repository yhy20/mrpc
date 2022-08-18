#include "../header.h"

#define BUF_SIZE 3

int main(int argc, char* argv[])
{
    char buf[BUF_SIZE];
    if(-1 == access("./data.txt", F_OK))
    {
        CreateFile("./data.txt", 300 * 1024 * 1024, 100);
    }
    
    int fd1 = open("./data.txt", O_RDONLY);
    if(-1 == fd1) LOG_FATAL("open() error!");
    int fd2 = open("./data.copy.txt", O_WRONLY | O_CREAT | O_TRUNC);
    if(-1 == fd2) LOG_FATAL("open() error!");
    
    ssize_t len = 0;
    while((len = read(fd1, buf, sizeof(buf))) > 0)
        write(fd2, buf, len);

    close(fd1);
    close(fd2);
    exit(EXIT_SUCCESS);
}