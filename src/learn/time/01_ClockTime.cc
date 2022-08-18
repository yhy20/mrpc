#include <ctime>
#include <cmath>
#include <iostream>

int main()
{
    clock_t time1 = clock();
    double sum = 0;
    for(int i = 0; i < 100000000; i++)
    {
        sum += sqrt(i);
    }
    clock_t time2 = clock();

    double t = static_cast<double>(time2 - time1) / CLOCKS_PER_SEC ;
    std::cout << "CLOCKS_PER_SEC: " << CLOCKS_PER_SEC << std::endl;
    std::cout << "Process running time: " << t << "s" << std::endl;

  return 0;
}