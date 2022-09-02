#include <string>
#include <iostream>

/// 定义基本数据类型变量的同时可以指定初值，如果未指定初始值，则 C++ 会区执行
/// 默认初始化行为，如下所示：
/// 全局变量和静态变量（包括静态局部变量）会默认初始化为 0。
/// 栈中变量（函数体中的自动变量）和堆中的变量（动态内存）会保有不确定的值。
/// https://blog.csdn.net/qq_41786318/article/details/82183268
static int a;
int b;
int* ptr;
std::string str;

/// main 之前执行
/**
 * 1. 设置栈指针。
 * 2. 初始化静态的 static 变量和 global 变量， 即 .data 段的内容。
 * 3. 将未初始化部分的全局变量赋初值，short, int, long 等为 0。
 * 4. 全局对象初始化，在 main 之前调用构造函数。
 * 5. 将 main 函数的参数 argc, argv 等传递给 main 函数，之后才真正运行。 
 */
void func()
{
    /// 初始化为 0
    static int a1;
    
    /// 随机值
    char b1;
    long long b2;
    int* c1;

    std::cout << "a = " << a << std::endl;
    std::cout << "b = " << b << std::endl;
    std::cout << "ptr = " << ptr << std::endl;
    std::cout << "str = " << str << std::endl;
    std::cout << "a1 = " << a1 << std::endl;
    std::cout << "b1 = " << b1 << std::endl;
    std::cout << "b2 = " << b2 << std::endl;
    std::cout << "c1 = " << c1 << std::endl; 
}

int main(void)
{
    func();

    return 0;
}