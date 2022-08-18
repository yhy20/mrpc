#include <ctime>
#include <iostream>

int main()
{
    /// 固定格式 Www Mmm dd hh:mm:ss yyyy\n
    time_t now = time(nullptr);
    std::cout << "Now is: " << ctime(&now);

    return 0;
}