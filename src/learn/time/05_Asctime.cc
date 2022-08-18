#include <ctime>
#include <iostream>

/**
 * gmtime() 函数将 time_t 时间转换为 UTC 表示的日历时间，它可能返回 NULL
 * 返回值指向一个静态分配的结构，该结构可能会被接下来的任何日期和时间函数调用覆盖
 * gmtime_r() 函数功能与此相同，但是它可以将数据存储到用户提供的结构体中
 * 
 * localtime() 函数将 time_t 时间转换为当地的日历时间，它可能返回 NULL
 * localtime() 函数将 timezone 设为 UTC 和本地标准时间的差值
 * 返回值指向一个静态分配的结构，该结构可能会被接下来的任何日期和时间函数调用覆盖
 * localtime_r() 函数功能与此相同，但是它可以将数据存储到用户提供的结构体中
 */
void Test1()
{
    time_t now = time(nullptr);
    std::cout << "now is: " << ctime(&now);

    tm* gmTime = gmtime(&now);
    std::cout << "gmTime = " << asctime(gmTime);

    tm* localTime = localtime(&now);
    std::cout << "localTime = " << asctime(localTime);
}

void Test2()
{
    time_t now = time(nullptr);
    tm gmTime, localTime;
    gmtime_r(&now, &gmTime);
    localtime_r(&now, &localTime);
    std::cout << "now is: " << ctime(&now);
    std::cout << "gmTime_r = " << asctime(&gmTime);
    std::cout << "localTime_r = " << asctime(&localTime);
}

int main()
{
    Test1();
    Test2();

    return 0;    
}