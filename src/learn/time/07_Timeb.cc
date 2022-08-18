#include <math.h>
#include <stdio.h>
#include <iostream>
#include <sys/timeb.h>

int main()
{   
    struct timeb start;
    struct timeb stop;
    double sum = 0;
    ftime(&start);
    for(int i = 0; i < 100000000; i++)
    {
        sum += sqrt(i);
    }
    ftime(&stop);
    std::cout << "ms time is " << (stop.time - start.time) * 1000 \
        + (stop.millitm - start.millitm) << std::endl;;

    return 0;
}