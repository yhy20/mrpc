#include "../../header.h"
/**
 * 程序清单 24-5: fork() 之后，父、子进程竞争输出信息
 * 运行 ./fork_whos_on_first | ./fork_whos_on_first.count.awk 检查竞争情况
 * 100w 次 fork() 子父进程竞争输出共 10 组测试情况如下：
 * Num children =  100000
 * parent  99927  99.93%
 * child      73   0.07%
 * Num children =  200000
 * parent 199867  99.93%
 * child     133   0.07%
 * Num children =  300000
 * parent 299798  99.93%
 * child     202   0.07%
 * Num children =  400000
 * parent 399722  99.93%
 * child     278   0.07%
 * Num children =  500000
 * parent 499676  99.94%
 * child     324   0.06%
 * Num children =  600000
 * parent 599603  99.93%
 * child     397   0.07%
 * Num children =  700000
 * parent 699536  99.93%
 * child     464   0.07%
 * Num children =  800000
 * parent 799480  99.94%
 * child     520   0.07%
 * Num children =  900000
 * parent 899435  99.94%
 * child     565   0.06%
 * Num children =  1000000
 * parent 999384  99.94%
 * child     616   0.06%
 * All done
 * parent 999384  99.94%
 * child     616   0.06%
 * 由上可知道，不应对 fork() 之后执行父、子进程的特定顺序做任何假设
 * 若确需在进程间保证某一特定执行顺序，则必须采用某种同步技术，具体包括
 * 信号量(semaphore)、文件锁(file lock)、管道(pipe)、信号(signal)等
 */
int main(void)
{
    pid_t pid;
    const int kChildProcessNum = 1000 * 1000;
    setbuf(stdout, nullptr);
    for(int i = 0; i < kChildProcessNum; ++i)
    {
        switch(pid = fork())
        {
        case -1:
            LOG_FATAL("fork() error!");

        case 0:
            printf("%d child\n", i + 1);
            _exit(EXIT_SUCCESS);

        default:
            printf("%d parent\n", i + 1);
            wait(nullptr);
            break;
        }
    }
    exit(EXIT_SUCCESS);
}