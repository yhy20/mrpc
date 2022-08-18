#include <chrono>
#include <iostream>

void Test1()
{
    std::chrono::time_point<std::chrono::system_clock> now 
        = std::chrono::system_clock::now();

    time_t time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Now is " << ctime(&time);
    // std::cout << now << std::endl;
}

// void Test2()
// {
//     std::chrono::time_point<std::chrono::steady_clock> now
//         = std::chrono::steady_clock::now();

//     // std::cout << now << std::endl;
// }

// void Test3()
// {

// }

int main()
{
    Test1();
}