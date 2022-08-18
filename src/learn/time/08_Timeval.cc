#include <time.h>
#include <stdio.h>
#include <sys/time.h>

int main()
{
    struct timeval tv;
    struct timezone tz;


    printf("gettimeofday time:\n");
    gettimeofday(&tv, &tz);

    printf("tv_sec:%ld tv_usec:%ld\n", tv.tv_sec, tv.tv_usec);
    printf("tz:%d %d\n", tz.tz_dsttime, tz.tz_minuteswest);

    return 0;
}