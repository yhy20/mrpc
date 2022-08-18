#include <ratio>
#include <iostream>

void Test1()
{
    // std::ratio<1, 1000>       milliseconds;
    // std::ratio<1, 1000000>    microseconds;
    // std::ratio<1, 1000000000> nanoseconds;

    std::ratio_add<std::ratio<5, 7>, std::ratio<59, 1023>> result;
    double value =  static_cast<double>(result.num) / result.den;
    std::cout << result.num << "/" << result.den << " = " << value << std::endl;
}

int main()
{
    Test1();    

    return 0;
}