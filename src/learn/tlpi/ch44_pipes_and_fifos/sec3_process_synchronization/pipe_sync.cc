#include "../../header.h"

/// 程序清单 44-3: pipe_sync.cc
/// 使用管道同步多个进程
int main()
{
    setbuf(stdout, nullptr);
    printf("%s Parent started!\n", CurrTime("%T"));
    const int kChildProcessNum = 3;
    unsigned int sleepTimes[kChildProcessNum] = {4, 2, 6}; 
    int pipefd[2];
    if(-1 == pipe(pipefd)) LOG_FATAL("pipe error!");
    for(int i = 0; i < kChildProcessNum; ++i)
    {
        switch(fork())
        {
        case -1:
            LOG_FATAL("fork() error!");
        case 0:
            if(-1 == close(pipefd[0]))
                LOG_FATAL("close() error!");
            
            /// Child does some work and lets parent it's done.

            /// Simulate processing.
            sleep(sleepTimes[i]);
            
            printf("%s child %d (PID=%ld) closing pipe\n",
                   CurrTime("%T"), i + 1, (long)getpid());
            if(-1 == close(pipefd[1]))
                LOG_FATAL("close() error!");
            /// Child now carries on to do other things.

            _exit(EXIT_SUCCESS);
            
        default:
            break;
        }
    }

    /// Parent comes here, close write end of pipe so we can see EOF.
    if(-1 == close(pipefd[1]))
        LOG_FATAL("close() error!");
    
    /// Parent may do other work, then synchronizes with children.
    int dummy;
    if(0 != read(pipefd[0], &dummy, sizeof(dummy)))
        LOG_FATAL("parent didn't get EOF!");
    
    printf("%s Parent ready to go\n", CurrTime("%T"));

    /// Parent can now carry on to do other things.

    exit(EXIT_SUCCESS);
}