#include <ctime>
#include <iostream>

int main()
{
    /// time_t 只精确到秒
    time_t epoch_time = time(nullptr);
    std::cout << "Epoch time: " << epoch_time << std::endl;

    return 0;
}