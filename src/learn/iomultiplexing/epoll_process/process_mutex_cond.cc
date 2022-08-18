#include "../header.h"

/// 简单测试一下进程间同步的代码
/// 用于 EPOLLIN 信号测试
int main()
{
    pid_t pid = fork();
    if(0 == pid)
    {
        Interaction* share_mem1 = GetInteraction("test_share_mem1");
        Interaction* share_mem2 = GetInteraction("test_share_mem2");
        sleep(1);
        printf("frist!\n");
        pthread_cond_signal(&share_mem1->cond);

        pthread_mutex_lock(&share_mem2->mutex);
        pthread_cond_wait(&share_mem2->cond, &share_mem2->mutex);
        pthread_mutex_unlock(&share_mem2->mutex);
        sleep(1);
        printf("third!\n");
    }
    else
    {
        Interaction* share_mem1 = GetInteraction("test_share_mem1");
        Interaction* share_mem2 = GetInteraction("test_share_mem2");

        pthread_mutex_lock(&share_mem1->mutex);
        pthread_cond_wait(&share_mem1->cond, &share_mem1->mutex);
        pthread_mutex_unlock(&share_mem1->mutex);
        sleep(1);
        printf("second!\n");

        pthread_cond_signal(&share_mem2->cond);
        sleep(2);
    }
    return 0;
}