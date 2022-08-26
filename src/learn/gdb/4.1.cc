#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

int a = 0;

void *thread1_func(void *arg)
{
    while(1)
    {
        ++a;
        sleep(10);
    }
}

int main(int argc, char* argv[])
{
    pthread_t t1;

    1
}