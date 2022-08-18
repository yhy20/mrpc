#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>

int main(void)
{
#ifdef USE_TEST
    printf("hello USE_TEST!\n");
#endif 
    int fd1 = socket(PF_INET, SOCK_STREAM, 0);
    int fd2 = open("data.txt", O_CREAT | O_WRONLY | O_TRUNC);
    int fd3 = socket(PF_INET, SOCK_DGRAM, 0);

    printf("file descriptor 1 = %d\n", fd1);
    printf("file descriptor 2 = %d\n", fd2);
    printf("file descriptor 3 = %d\n", fd3);

    close(fd1);
    close(fd2);
    close(fd3);

    return 0;
}