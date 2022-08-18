#include "../../header.h"

#define SYNC_SIG SIGUSR1 // Synchronizatoin signal.

/// Signal handler - does nothing but return.
static void handler(int sig)
{

}

int main(void)
{
    pid_t pid;
    setbuf(stdout, nullptr);

    sigset_t blockMask, origMask, emptyMask;
    struct sigaction sa;
    sigemptyset(&blockMask);
    sigaddset(&blockMask, SYNC_SIG); // Block signal.
    if(-1 == sigprocmask(SIG_BLOCK, &blockMask, &origMask))
        LOG_FATAL("sigprocmask() error!");
    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if(-1 == sigaction(SYNC_SIG, &sa, nullptr))
        LOG_FATAL("sigaction() error!");
    
}