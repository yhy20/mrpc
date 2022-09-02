#include <iostream>
#include <typeinfo>

typedef char* String_t;
#define String_d char*
#define String_e cchar*

int main()
{
    /// typedef char * String_t 定义了一个新的类型别名，有类型检查。
    /// #define String_d char * 只做了简单的替换，无类型检查。
    /// 前者在编译的时候处理，后者在预编译的时候处理。

   
    String_t a, b;
    String_d c, d;
    std::cout << typeid(a).name() << std::endl;
    std::cout << typeid(b).name() << std::endl;
    std::cout << typeid(c).name() << std::endl;
    std::cout << typeid(d).name() << std::endl;

    /// typedef 还要做类型检查，我的理解是 typedef 在 IDE 编写代码时检查语法并显示错误
    /// String_e e;
    exit(EXIT_SUCCESS);
}

