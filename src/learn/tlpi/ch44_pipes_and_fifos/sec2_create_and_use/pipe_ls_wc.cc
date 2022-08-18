#include <sys/wait.h>
#include "../../header.h"

/// 程序清单 44-4 pipe_ls_wc.cc 
/// 使用管道连接 ls 和 wc 
int main(void)
{
    int pipefd[2];
    if(-1 == pipe(pipefd))
        LOG_FATAL("pipe() error!");
    
    // LOG_INFO("pipefd[0] = %d, pipefd[1] = %d",
    //          pipefd[0], pipefd[1]);

    switch(fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0: // first child: exec 'ls' to write to pipe.
        if(-1 == close(pipefd[0]))
            LOG_FATAL("close(pipefd[0]) error!");
        
        if(STDOUT_FILENO != pipefd[1])
        {
            if(-1 == dup2(pipefd[1], STDOUT_FILENO))
                LOG_FATAL("dup2(pipefd[1], STDOUT_FILENO) error!");

            if(-1 == close(pipefd[1]))
                LOG_FATAL("close pipefd[1] error!");

            /// TODO: learn execlp
            execlp("ls", "ls", (char*)nullptr);
            LOG_FATAL("execlp ls");
        }  

    default:
        break;
    }

    switch(fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0: // second child: exec 'wc' to read from pipe.
        if(-1 == close(pipefd[1]))
            LOG_FATAL("close(pipefd[1]) error!");
        
        if(STDIN_FILENO != pipefd[0])
        {
            if(-1 == dup2(pipefd[0], STDIN_FILENO))
                LOG_FATAL("dup2(pipefd[0], STDIN_FILENO) error!");
            if(-1 == close(pipefd[0]))
                LOG_FATAL("close(pipefd[0]) error!");
        }

        execlp("wc", "wc", "-l", (char*)nullptr);
        LOG_FATAL("execlp wc");

    default:
        break;
    }

    if(-1 == close(pipefd[0]))
        LOG_FATAL("parent porcess close(pipefd[0]) error!");
    if(-1 == close(pipefd[1]))
        LOG_FATAL("parent porcess close(pipefd[1]) error!");
    if(-1 == wait(nullptr))
        LOG_FATAL("wait 1");
    if(-1 == wait(nullptr))
        LOG_FATAL("wait 2");
    
    exit(EXIT_SUCCESS);
}