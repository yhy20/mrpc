#include <sys/wait.h>
#include "../header.h"


/// TODO: 目前没测出 exit(3) 函数的效果
int main(void)
{
    int pipefd[2];
    
    if(-1 == pipe(pipefd))
        LOG_FATAL("pipe() error!");

    FILE* fp = fopen("/dev/fd/1", "w");
    // char buf[4096];
    // setbuf(fp, buf);

    // char msg[] = "test\n";

    // fwrite(msg, strlen(msg), 1, fp);
    // fflush(fp);

   

    switch(fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0: 
        // fprintf(fp, "test\n");
        // fflush(fp);
        if(-1 == close(pipefd[1]))
            LOG_ERROR("close pipefd[1] error!");

        int dummy;
        if(0 != read(pipefd[0], &dummy, sizeof(dummy)))
            LOG_ERROR("read pipefd[0] error!");

        if(-1 == close(pipefd[0]))
            LOG_ERROR("colse pipefd[0] error!");

        // exit(EXIT_SUCCESS);
        fflush(fp);
        LOG_FATAL("child process exit!");
        // LOG_FATAL_EX("child process exit!");
        
    default:
        if(-1 == close(pipefd[0]))
            LOG_ERROR("close pipefd[0] error!");

        /// parent process output the first part of the data
        fprintf(fp, "Hello, ");
        // fflush(fp);
        sleep(1);
        
        if(-1 == close(pipefd[1]))
            LOG_ERROR("close pipefd[1] error!");

        /// child process exit, but we don't want the stdout buffer to be flushed
        wait(nullptr);

        sleep(1);

        /// parent process output the second part of the data
        fprintf(fp, "World\n");
        fflush(stdout);
        
        LOG_INFO("parent process exit!");
        exit(EXIT_SUCCESS);
    }
}