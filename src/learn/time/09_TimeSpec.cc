#include <cstdio>
#include <ctime>

void Test1()
{
    timespec ts;
    timespec_get(&ts, TIME_UTC);
    char buf[100];
    strftime(buf, sizeof(buf), "%D %T", std::gmtime(&ts.tv_sec));
    printf("Current time: %s.%09ld UTC\n", buf, ts.tv_nsec);
}

void Test2()
{
    /// int clock_gettime (clockid_t __clock_id, struct timespec *__tp)
    /// 根据 clock_id(系统时钟) 的类型，获取当前时间。
    /// clock_id 常用取值枚举说明
    /// CLOCK_REALTIME              系统当前时间，从1970年1月1日算起
    /// CLOCK_MONOTONIC             从系统启动时间算起，经过的时间
    /// CLOCK_PROCESS_CPUTIME_ID    本进程运行时间
    /// CLOCK_THREAD_CPUTIME_ID	    本线程运行的时间

    struct timespec tsp;

    printf("CLOCK_REALTIME time:\n");
    clock_gettime(CLOCK_REALTIME, &tsp);
    printf("sec:%ld, nsec:%ld\n", tsp.tv_sec, tsp.tv_nsec);
    printf("-----------------------\n");

    printf("CLOCK_MONOTONIC time:\n");
    clock_gettime(CLOCK_MONOTONIC, &tsp);
    printf("sec:%ld, nsec:%ld\n", tsp.tv_sec, tsp.tv_nsec);
    printf("-----------------------\n");

    printf("CLOCK_PROCESS_CPUTIME_ID time:\n");
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tsp);
    printf("sec:%ld, nsec:%ld\n", tsp.tv_sec, tsp.tv_nsec);
    printf("-----------------------\n");

    printf("CLOCK_THREAD_CPUTIME_ID time:\n");
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tsp);
    printf("sec:%ld, nsec:%ld\n", tsp.tv_sec, tsp.tv_nsec);
    printf("-----------------------\n");
}

int main()
{
    Test1();
    Test2();
    return 0;
}