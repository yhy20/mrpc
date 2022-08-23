#include "../../header.h"

/**
 * #include <signal.h>
 * int kill(pit_t pid, int sig);
 * Returns 0 on success, or -1 on error.
 * 
 * (1) pid > 0, 则发送信号给由 pid 指定进程。
 * (2) pid == 0, 则发送信号给与调用进程同组的每个进程，包括调用进程自身。
 * (3) pid == -1, 则信号发送范围是：调用进程有权将信号发往每一个目标进程，
 *     除去 init（进程 ID 为 1）和调用进程自身。如果特权进程发起这一调用，
 *     那么会发送信号给系统中所有的进程，上述两个进程除外。显而易见，有时
 *     也将这种信号发送的方式称之为广播信号
 * (4) pid < -1, 则会向组 ID 等于该 pid 绝对值的进程组内所有下属进程发
 *     送信号。向一个进程组的所有进程发送信号在 shell 作业控制中有特殊用途
 * 
 * 如果并无进程与指定的 pid 相匹配，那么 kill() 调用失败，同时将 errno 设
 * 置为 ESRCH（查无此进程）
 *     
 * 进程要发送信号给另一进程，还需要适当的权限，其权限规则如下：
 * (1) 特权级进程可以向任何进程发送信号
 * (2) 以 root 用户和组运行的 init 进程（进程号为 1），是一种特例，仅能接
 *     收已安装了处理器函数的信号。这可以防止系统管理员意外杀死 init 进程
 *     ——这一系统运行的基石
 * (3) 如果发送者的实际或有效用户 ID 匹配于接收者的实际用户 ID 或者保存设置
 *     用户 ID，那么非特权进程也可以向另一进程发送信号。利用这一规则，用户可
 *     以向他们启动的 set-user-ID 程序发送信号，而无需考虑目标进程的有效用户
 *     ID 的当前设置。
 * (4) 
 * 
 */

int main(int argc, char* argv[])
{
    
}