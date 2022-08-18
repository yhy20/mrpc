#include <limits.h>

#include "../../header.h"

int main()
{
    int pipe_fd[2];
    pipe(pipe_fd);

    long pipe_buf_size = ::fpathconf(pipe_fd[1], _PC_PIPE_BUF);
    printf("pipe_buf_size = %ld\n", pipe_buf_size);

    return 0;
}