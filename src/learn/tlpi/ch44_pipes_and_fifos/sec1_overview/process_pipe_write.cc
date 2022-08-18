#include <sys/wait.h>

#include <string>
#include <thread>

#include "../../header.h"

/**
 * 定义 TEST_WRITE_SHORT_MSG 宏，则测试多个进程向管道中写入短消息，保证总共写入的字节数不会超过
 * 管道的实际内核 buffer 大小，这表明所有的数据写入是非阻塞的，且一次数据写入不会被截断，是原子的
 * 不会发生数据交叉的现象
 * 
 * 不定义 TEST_WRITE_SHORT_MSG 宏，则默认测试多个进程向管道中写入长消息，保证每次写入的数据都超
 * 过了管道的实际内核 buffer 大小，打开写入数据的 temp 文件可用观察到一条长消息被截断分次写入，发
 * 生了数据交叉的现象
 * 
 * 多个进程写入的数据小于 PC_PIPE_BUF 能保证写入时的原子性，即写入的数据不会混乱
 * pipe_buf_size 实际大小为 65536，在写入数据超过 65536 时，可以观察到写入数据发生交叉的情况
 * 若一次写入的数据小于 PIPE_BUF，则 write 会在必要时候阻塞直到管道中可用空间足以原子的完成操作
 * 若一次写入的数据大于 PIPE_BUF，则 write 会尽可能多的传输数据以充满整个管道，然后阻塞直到一些
 * 
 * TODO: 测试写数据阻塞时被信号中断返回部分写入字节数的情况
 * TODO: 用线程实现同样的测试，观察区别
 * 
 * 详细讨论 关闭未使用管道文件描述的重要性
 * TODO: ch44.3 关闭未使用管道文件描述符
 * (1) 防止进程无意义的消耗文件描述符资源，超过文件描述符的限制
 * (2) 
 * (3) 
 * (4) 
 */

#define PRINT_FORK_INFO

/// #undef means test write long msg.
#define TEST_WRITE_SHORT_MSG
// #undef TEST_WRITE_SHORT_MSG

#define BUF_SIZE 2 * 65536

const int kChildProcessNum = 5;

int main()
{
    int pipefd[2];
    if(-1 == pipe(pipefd)) LOG_FATAL("pipe() error!");
    
#ifdef PRINT_FORK_INFO
    Interaction* share_mem = GetInteraction("mem");
#endif

    /// get the real pipe buf size, whcih means 
    /// whether data can be written atomically
    int pipe_buf_size = fcntl(pipefd[1], F_GETPIPE_SZ);
    printf("pipe_buf_size = %d\n", pipe_buf_size);
    
    /// to see the changes of pipefd before and after fork
    // printf("Before fork(), pipefd[0] = %d, pipefd[1] = %d\n", pipefd[0], pipefd[1]);

    pid_t child_pids[kChildProcessNum];

    int i = 0;
    for(; i < kChildProcessNum; ++i)
    {
        pid_t pid = fork();
        if(-1 == pid) LOG_FATAL("fork() error!");
        else if(0 == pid) break;
        else
        {
            child_pids[i] = pid;
        }
    }

    if(i < kChildProcessNum) // child processes
    {
        /**
         * 写这段打印测试是因为在使用 fork 后的 child_pids 数
         * 组时遇到了 bug，在共享内存上创建 mutex 来设置进程临
         * 界区防止打印混乱，下列程序片段详细的显示了 5 次调用 
         * fork 时 child_pids 数组在 fork 后状态
         */
        pthread_mutex_lock(&share_mem->mutex);
        printf("<--------------in-------------->\n");
        printf("in child process %d\n", i + 1);
        for(int j = 0; j < kChildProcessNum; ++j)
        {
            printf("child process %d pid = %ld\n", j + 1, (long)child_pids[j]);
        }
        printf("<--------------out-------------->\n");
        pthread_mutex_unlock(&share_mem->mutex);

        /// to see the changes of pipefd before and after fork
        // printf("After fork(), in child process %ld, pipefd[0] = %d, "
        //        "pipefd[1] = %d\n",(long)::getpid() , pipefd[0], pipefd[1]);

        close(pipefd[0]); // close unused read end
#ifdef TEST_WRITE_SHORT_MSG
        std::string shrotMsg = 
            "I am child process " + std::to_string((long)::getpid()) + '\n';
        write(pipefd[1], shrotMsg.c_str(), shrotMsg.size());
#else
        std::string longMsg = std::string(2 * 65536, char(i + 1 + '0'));
        write(pipefd[1], longMsg.c_str(), longMsg.size());
#endif  
        close(pipefd[1]);

        /// TODO: learn _exit, _Exit, exit
        exit(EXIT_SUCCESS);
    }
    /// parent process
    else
    {
        /// to see the changes of pipefd before and after fork
        // printf("After fork(), in parent process, "
        //        "pipefd[0] = %d, pipefd[1] = %d\n", pipefd[0], pipefd[1]);
        
        close(pipefd[1]); // close unused write end
        char buf[BUF_SIZE] = { 0 };
#ifndef TEST_WRITE_SHORT_MSG
        FILE* fp = fopen("./temp.txt", "w");
#endif 
        /// wait to prevent data printing confusion
        sleep(1);
        ssize_t len = 0;
        while((len = read(pipefd[0], buf, BUF_SIZE)) > 0)
        {
            buf[len] = '\0';
#ifdef TEST_WRITE_SHORT_MSG
            printf("len = %ld\n%s", (long)len, buf);
#else
            fwrite(buf, len, 1, fp);
            printf("write bytes %ld\n", (long)len);
#endif
        }
        if(-1 == len) LOG_FATAL("read() error!");
        close(pipefd[0]);

#ifndef TEST_WRITE_SHORT_MSG
        fflush(fp);
        fclose(fp);
#endif 

        for(int j = 0; j < kChildProcessNum; ++j)
        {
            int status;
            pid_t ret_pid = waitpid(child_pids[j], &status, 0);
            if(-1 == ret_pid)
            {
                LOG_ERROR("wait for child process %ld error.\n", (long)child_pids[j]);
            }
            if(WIFEXITED(status))
            {
                printf("child process %ld terminated normally.\n", (long)child_pids[j]);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        exit(EXIT_SUCCESS);
    }
}