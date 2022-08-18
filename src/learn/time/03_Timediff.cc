#include <cmath>
#include <ctime>
#include <iostream>

int main()
{
  time_t start = time(nullptr);
  double sum = 0;
  for(int i = 0; i < 1000000000; i++)
  {
    sum += sqrt(i);
  }
  time_t stop = time(nullptr);

  double time_diff = difftime(stop, start);
  std::cout << "time1: " << start << std::endl;
  std::cout << "time2: " << stop << std::endl;
  std::cout << "time_diff: " << time_diff << "s" << std::endl;

  return 0;
}