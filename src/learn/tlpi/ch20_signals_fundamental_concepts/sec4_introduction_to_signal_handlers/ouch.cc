#include "../../header.h"

/// 不可重入 UNSAFE (see Section 21.1.2)
static void sigHandler(int sig)
{
    printf("Ouch!\n");
}

/// 程序清单 20-1: 为 SIGINT 信号注册一个处理程序
int main(void)
{
    /**
     * Invaocation of a signal handler may interrupt the main program
     * flow at any time; the kernel calls the handler on the process's
     * behalf, and when the handler returns, execution of the program 
     * resumes at the point where the handler interruptd it.
     */
    if(SIG_ERR == signal(SIGINT, sigHandler))
        LOG_FATAL("signal() error!");
    
    for(int i = 0; i < 5; ++i)
    {
        printf("i = %d\n", i);
        
        /// SIGINT 会中断 sleep() 函数
        sleep(3);   // Loop slowly.
    }
}