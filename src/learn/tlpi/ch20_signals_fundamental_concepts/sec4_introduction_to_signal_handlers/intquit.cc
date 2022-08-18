#include "../../header.h"

static void sigHander(int sig)
{
    static int count = 0;
    /**
     * UNSAFE: This handler uses non-async-signal-safe 
     * functions (printf(), exit(); see Section 21.1..2)
     * 该线程处理函数内部使用了非异步信号安全函数，所以不安全
     */
    if(SIGINT == sig)
    {
        ++count;
        printf("Caught SIGINT(%d)\n", count);
        /// Resume execution at point of interruptoin.
        return; 
    }

    /// Must be SIGQUIT - print a message and terminate the process.
    printf("Caught SIGQUIT - that's all folks!\n");  
    // abort();
    exit(EXIT_SUCCESS);
}

/**
 * 在当前 session 打开 core dump
 * (1) check: ulimit -c 
 * (2) open: ulimit -c unlimited 
 * (3) close: ulimit -c 0
 */

/// 程序清单 20-2: 为两个不同信号建立同一处理函数
int main(void)
{
    /// Established same handler for SIGNIT and SIGQUIT.

    if(SIG_ERR == signal(SIGINT, sigHander))
        LOG_FATAL("signal() error!");
    if(SIG_ERR == signal(SIGQUIT, sigHander))
        LOG_FATAL("signal() error!");
    
    for(;;)             // Loop forever, waiting for signals.
        pause();        // Block until a signal is caught.

}