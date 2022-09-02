#include <iostream>
/// 全局的 operator new 应该无法重载

void* operator new(std::size_t size)
{   
    std::cout << "operator new" << std::endl;
    return std::malloc(size);
}

/// https://zhuanlan.zhihu.com/p/354046948

/**
 * (1) 浅析 operator new 的重载为什么必须第一个参数是 size_t 并且必须返回 void* 
 * 首先、第一个参数必须是 size_t，是因为 new 关键字的默认行为不可更改。当使用
 * new 关键字时，需要指定分配内存的类型，然后根据类型来计算需要分配内存的大小，
 * 并将其作为第一个参数传递给 operator new，需要注意，传递该参数是编译器自动
 * 的行为，用户无法更改，类似于 this 指针。所以 operator new 的重载无法像普通
 * 函数一样，使用任意类型和个数的参数。
 * 
 * 其次、返回值是
 * 
 * (2) 浅析 operator new 可以进行签名与默认函数完全一样的重载。
 * 对于普通函数而言，该做法是无法通过编译的，这相当于实现了两个声明完全一样的函数，
 * 正常情况下会导致函数调用二义性。不过 operator new 不是普通函数，它是一个关键
 * 字，它的调用行为和普通函数不同。当编译器发现有调用 new 关键字时，首先会在当前
 * 类和其基类中寻找 operator new, 找不到则在全局中寻找合适的实现，还找不到才会使
 * 用默认的实现。
 * 
 * (3) 浅析 placement new 
 * 其实根本没有所谓的 placement new，placement new 不过是一种 operator new 的重
 * 载形式罢了，本质上是为了摆脱 new 申请新空间再构造对象的行为，希望使用已有的空间
 * 来构造对象，由于 new 的默认行为无法更改，用户唯一能够更改的是控制 new 调用时传入 
 * 的参数（需要注意 size_t 参数是默认传递的），而 new 内部实现会把用户额外传入的参数
 * 原封不动的传递给 operator new，通过参数决定调用哪种形式的 operator new。试想一
 * 下，如果要改变 new 行为，使用已有的空间构造对象，显然需要传递指向该空间的指向，最
 * 终有了如下形式的所谓的 placement new
 * void* operator new(std::size_t size, void* ptr);
 */
void* operator new(std::size_t size, int num, char c)
{   
    std::cout << "size = " << size << std::endl;
    std::cout << "num = " << num << std::endl;
    std::cout << "c = " << c << std::endl;
    return std::malloc(size);
}

struct ST
{
public: 
    ST() { }
private:
    int a;
    int b;
    int c;
};

int main(void)
{
    void* ptr1 = operator new(10);
    void* ptr2 = operator new(10, 1, 'c');
    void* ptr3 = new ST;
    void* ptr4 = new(1, 'c') ST();  

    return 0;
}