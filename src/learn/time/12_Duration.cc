#include <cmath>

#include <chrono>
#include <iostream>

void Test1()
{
    std::chrono::hours two_hours(2);
    std::chrono::minutes five_minutes(5);

    auto duration = two_hours + five_minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    std::cout << "02:05 is " << seconds.count() << " seconds" << std::endl;
}

// void Test2()
// {
//     using namespace std::chrono_literals;
//     auto two_hours = 2h;
//     auto five_minutes = 5min;

//     auto duration = two_hours + five_minutes;
//     auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
//     std::cout << "02:05 is " << seconds.count() << " seconds" << std::endl;
// }

void Test3()
{
    auto start = std::chrono::steady_clock::now();
    double sum = 0;
    for(int i = 0; i < 100000000; i++) {
        sum += sqrt(i);
    }
    auto stop = std::chrono::steady_clock::now();

    auto time_diff = stop - start;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff);
    std::cout << "Operation cost : " << duration.count() << "ms" << std::endl;  
}

int main()
{
    Test1();
    // Test2();
    Test3();
}