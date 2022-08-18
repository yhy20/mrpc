#include "../header.h"

int main() 
{ 
    int pipe_fd[2];     
    if (-1 == pipe(pipe_fd)) 
        LOG_FATAL("pipe error!");

    pid_t pid = fork();
    if(-1 == pid) LOG_FATAL("fork error!");

    if(0 == pid)
    {
        close(pipe_fd[1]); 

        char buf[100] = { 0 };
        ssize_t len = read(pipe_fd[0], buf, sizeof(buf));
        if(-1 == len) LOG_ERROR("read() error");
        sleep(1);
        printf("second\n");
        close(pipe_fd[0]);
    }
    else
    {
        close(pipe_fd[0]);
        sleep(1);
        printf("first\n");

        uint64_t one = 1;
        write(pipe_fd[1], &one, sizeof(one));

        /// wait for child process
        sleep(2);
        close(pipe_fd[1]);
    }

    return 0; 
}
 