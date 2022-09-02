#include <iostream>
#include <limits>
#include <stdlib.h>
// #include <tools/dbg.h>

template <typename T>
class MyAllocator
{
public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;
    
    using void_pointer = void *;
    using const_void_pointer = const void*;

    using size_type = size_t;
    using difference = std::ptrdiff_t;

    template <typename U>
    struct rebind
    {
        using other = MyAllocator<U>;
    };

    MyAllocator() = default;
    ~MyAllocator() = default;

    pointer allocate(size_type numObjects)
    {
        allocCount += numObjects;
        printf("MyAllocator::allocate, 内存分配：%ld\n", static_cast<long>(numObjects));
        return static_cast<pointer>(operator new(sizeof(T) * numObjects));
    }

    pointer allocate(size_type numObjects, const_void_pointer hint)
    {
        return allocate(numObjects);
    }

    void deallocate(pointer p, size_type numObjects)
    {
        printf("MyAllocator::deallocate, 内存释放：%ld\n", static_cast<long>(numObjects));
        allocCount -= numObjects;
        operator delete(p);
    }

    size_type max_size() const
    {
        return std::numeric_limits<size_type>::max();
    }

    size_type get_allocations() const
    {
        return allocCount;
    }

private:
    /// 统计当前内存使用量
    size_type allocCount; 

};