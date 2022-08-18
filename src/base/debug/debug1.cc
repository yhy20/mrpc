#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

int main()
{
    // int n =  123456789;
    // char buf[32];
    // snprintf(buf, sizeof(buf), "%06d", n);
    // printf("%s\n", buf);
    // printf("%06d\n", n);
    const int m = 24 * 60 * 60;
    time_t t1 = time(nullptr);
    time_t t2 = t1 / m * m;
    // time_t t3 = (t2 + 1)
    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;

    return 0;
}