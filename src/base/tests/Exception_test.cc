// #include <vector>
// #include <string>
// #include <iostream>
// #include <functional>

// #include "Util.h"
// #include "Exception.h"
// #include "CurrentThread.h"

// using namespace mrpc;

// class Bar
// {
// public:
//     void test(std::vector<std::string> names = {})
//     {
//         printf("Stack:\n%s\n", CurrentThread::StackTrace(true).c_str());
//         [] {
//             printf("Stack inside lambda:\n%s\n", CurrentThread::StackTrace(true).c_str());
//         } ();

//         /// TODO: 解决打印库函数的问题
//         std::function<void()> func([] {
//         printf("Stack inside std::function:\n%s\n", CurrentThread::StackTrace(true).c_str());
//         });

//         func();

//         func = std::bind(&Bar::callback, this);

//         func();
//         throw Exception("Test!");
//     }

// private:
//     void callback()
//     {
//         printf("Stack inside std::bind:\n%s\n", CurrentThread::StackTrace(true).c_str());
//     }
// };


// void Foo()
// {
//     Bar b;
//     b.test();
// }

// void Test1()
// {
//     try
//     {
//         Foo();
//     }
//     catch(const Exception& e)
//     {
//         printf("reason: %s\n", e.what());
//         printf("stack trace:\n%s\n", e.stackTrace());
//     }
// }

// void Test()
// {
//     printf("stack trace:\n%s\n", CurrentThread::StackTrace().c_str());
//     printf("stack trace:\n%s\n", CurrentThread::StackTrace(true).c_str());
// }

// int main()
// {
//     // Test();
//     Test1();
//     // Test2();

//     return 0;
// }