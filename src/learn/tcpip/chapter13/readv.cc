#include "../header.h"

#define BUF_SIZE 100

int main(void)
{
    char buf1[BUF_SIZE] = { 0 };
    char buf2[BUF_SIZE] = { 0 };

    struct iovec vec[2];
    vec[0].iov_base = buf1;
    vec[0].iov_len = 4;
    vec[1].iov_base = buf2;
    vec[1].iov_len = BUF_SIZE;

    char str[] = "I like TCP/IP socket programming~\n";

    int fds[2];
    pipe(fds);
    pid_t pid = fork();
    if(0 == pid)
    {
        write(fds[1], str, strlen(str));
    }
    else
    {
        ssize_t str_len = readv(fds[0], vec, 2);
        printf("Read bytes: %zd\n", str_len);
        printf("First message: %s\n", buf1);
        printf("First message: %s", buf2);
    }
    exit(EXIT_SUCCESS);
}