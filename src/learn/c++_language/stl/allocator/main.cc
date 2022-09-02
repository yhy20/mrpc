#include <vector>
#include <unistd.h>
#include "myallocator.h"


int main(void)
{
    std::vector<int, MyAllocator<int>> v(0);
    for(size_t i = 0; i < 30; ++i)
    {
        sleep(1);
        v.push_back(i);
        printf("当前容器内存占用量：%ld\n", static_cast<long>(v.get_allocator().get_allocations()));
    }
}