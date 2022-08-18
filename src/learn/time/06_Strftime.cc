#include <ctime>
#include <iostream>

void Test1()
{
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    printf("Now is %4d-%02d-%02d %02d:%02d:%02d\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

void Test2()
{   
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    std::cout << "Now is " << buf << std::endl;
}

int main()
{
    Test1();
    Test2();

    return 0;
}