#include <exception>
#include <iostream>
#include <limits>
#include <stdio.h>

struct TS
{
public:
    TS(int _a) : a(_a) { }

private:
    int a;
};

/**
 * operator new 的三种形式
 * 1) 全局重载
 * void* operator new(size_t size)
 * {
 *     
 * }
 * 
 */

int main(void)
{
    /// throw std::bad_alloc
    // void* ptr1 = operator new(std::numeric_limits<size_t>::max());

    /// return nullptr; 
    void* ptr2 = operator new(std::numeric_limits<size_t>::max(), std::nothrow);
    if(nullptr == ptr2) printf("allocate failed!\n");


    void* p1 = operator new(sizeof(TS));

    
    // void* p2 = operator new(sizeof(TS) , p1);
    /// new(p1) TS(100) 内部会调用 placement new;
    // TS* p3 = new(p1) TS(100);
    std::cout << typeid(new(p1) TS(100)).name() << std::endl;


    // TS* t = new TS(1);
    // TS* t1 = new(std::nothrow) TS(2);
    return 0;
}
