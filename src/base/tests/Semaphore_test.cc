#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <functional>

#include "Util.h"
#include "Mutex.h"
#include "StringPiece.h"

using namespace mrpc;

class FooBar 
{
public:
    FooBar(int n)
        : m_num(n),
          m_semFoo(1),
          m_semBar(0) { }

    ~FooBar() { }

    void foo(std::function<void(int)>& printFoo)
    {
        for(int i = 0; i < m_num; i++)
        {
            m_semFoo.wait();
            printFoo(i + 1);
            m_semBar.signal();
        }
    }

    void bar(std::function<void(int)>& printBar)
    {
        for(int i = 0; i < m_num; i++)
        {
            m_semBar.wait();
            printBar(i + 1);
            m_semFoo.signal();
        }
    }

private:
    int m_num;
    Semaphore m_semFoo, m_semBar;
};

void TestFooBar(int num, StringArg title)
{   
    Util::PrintTitle(title);
    std::function<void(int)> printFoo = [](int i) -> void { printf("num%d: Foo\n", i); };
    std::function<void(int)> printBar = [](int i) -> void { printf("num%d: Bar\n", i); };
    FooBar obj(num);
    std::thread t1(&FooBar::foo, &obj, ref(printFoo));
    std::thread t2(&FooBar::bar, &obj, ref(printBar));
    t1.join();
    t2.join();
}

int main()
{
    TestFooBar(10, "Test Semaphore");
    
    return 0;
}