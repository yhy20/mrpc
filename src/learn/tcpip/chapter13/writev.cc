#include "../header.h"

/// '\0' 不会被显示到屏幕上
void debug()
{
    putc('\0', stdout);
    putc('a', stdout);
    putc('b', stdout);
    puts("");
}

int main(void)
{
    char buf1[] = "ABCDEFG";
    char buf2[] = "1234567";

    struct iovec vec[2];
    vec[0].iov_base = buf1;
    vec[0].iov_len = 3;
    vec[1].iov_base = buf2;
    vec[1].iov_len = 4;

    ssize_t str_len = writev(STDOUT_FILENO, vec, 2);
    if(-1 == str_len) LOG_ERROR("writev() error!");
    puts("");
    printf("Write bytes: %zd\n", str_len);

    exit(EXIT_SUCCESS);
}