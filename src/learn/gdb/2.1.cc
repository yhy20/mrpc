#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* thread_func(void* arg)
{
    while(1)
    {
        sleep(10);
    }

    return NULL;
}

int main(void)
{
    pthread_t t1, t2;

    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);

    sleep(1000);
    return 0;
}