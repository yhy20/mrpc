#include "../../header.h"

static int idata = 111; // allocated in data segment.

int main(void)
{   
    int istack = 222; // allocated in stack segment.

    pid_t pid;
    switch(pid = fork())
    {
    case -1:
        LOG_FATAL("fork() error!");
    case 0:
        idata *= 3;
        istack *= 3;
        break;
    default:
        sleep(1);  // Give child a chance to execute.
    }

    /// both parent and child come here.

    LOG_INFO("PID=%ld %s idata=%d istack=%d", (long)getpid(),
            (pid == 0) ? "(child)" : "(parent)", idata, istack);
    exit(EXIT_SUCCESS);
}