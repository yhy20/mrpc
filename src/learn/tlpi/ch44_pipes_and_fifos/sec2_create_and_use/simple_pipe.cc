#include <sys/wait.h>
#include "../../header.h"

#define BUF_SIZE 20

/// 程序清单 44-2: simple_pipe.cc
int main(int argc, char* argv[])
{
    int pipefd[2];
    /// create the pipe 
#if defined(_GNU_SOURCE)
    if(-1 == pipe2(pipefd, O_CLOEXEC))
        LOG_FATAL("pipe2() error!");
#else
    if(-1 == pipe(pipefd))
        LOG_FATA("pope() error!");
#endif

    switch(fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0: // child - reads from pipe
        if(-1 == close(pipefd[1])) // close unused write end
            LOG_FATAL("child process pipefd[1] close() error!");

        ssize_t numRead;
        char buf[BUF_SIZE];
        for(;;)
        {
            numRead = read(pipefd[0], buf, BUF_SIZE);
            if(-1 == numRead) LOG_FATAL("read() error!");
            if(0 == numRead) break;
            if(numRead != write(STDOUT_FILENO, buf, numRead))
            {
                LOG_FATAL("child process write() error!");
            }
        }
        write(STDOUT_FILENO, "\n", 1);
        if(-1 == close(pipefd[0])) 
            LOG_FATAL("child process pipefd[0] close() error!");
        exit(EXIT_SUCCESS);

    default: // parent - writes to pipe
        if(-1 == close(pipefd[0])) // close unused read end
            LOG_FATAL("parent process pipefd[0] close() error!");
        char msg[] = "Hello!";
        if(strlen(msg) != (size_t)write(pipefd[1], msg, strlen(msg)))
            LOG_FATAL("parent process write() error!");
        if(-1 == close(pipefd[1]))
            LOG_FATAL("parent process pipefd[1] close() error!");
        wait(nullptr);
        exit(EXIT_SUCCESS);
    }
}