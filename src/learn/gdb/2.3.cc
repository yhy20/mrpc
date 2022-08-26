#include <stdio.h>
#include <pthread.h>

typedef struct 
{
    int a;
    int b;
    int c;
    int d;
    pthread_mutex_t mutex;
}ex_st;

int main(void)
{
    ex_st st = { 1, 2, 3, 4, PTHREAD_MUTEX_INITIALIZER };
    /// set step-mode on
    /// 进入 printf 函数后，
    printf("%d,%d,%d,%d\n", st.a, st.b, st.c, st.d);
    return 0;
}
