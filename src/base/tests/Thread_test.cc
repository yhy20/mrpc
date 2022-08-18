#include <string>
#include <functional>

#include "Util.h"
#include "Thread.h"
#include "CurrentThread.h"

using namespace mrpc;

void ThreadFunc()
{
    printf("tid = %d\n", CurrentThread::Tid());
}

void ThreadFunc1(int num)
{
    printf("tid = %d, num = %d \n", CurrentThread::Tid(), num);
}

void ThreadFunc2()
{   
    printf("tid = %d\n", CurrentThread::Tid());
    Util::SleepSec(1);
}


void TestFreeFunc()
{
    Util::PrintTitle("Test thread for free func without argument");
    Thread t(ThreadFunc);
    t.start();
    printf("tid = %d\n", t.tid());
    t.join();
}

void TestFreeFuncWithArg()
{
    Util::PrintTitle("Test thread for free func with argument");
    Thread t(std::bind(ThreadFunc1, 100));
    t.start();
    printf("tid = %d\n", t.tid());
    t.join();
}

class Foo
{
public:
    explicit Foo(double value) : m_value(value) { }

    void memberFunc()
    {
        printf("tid = %d, Foo:m_num = %f\n", CurrentThread::Tid(), m_value);
    }

    void memberFuncWithArg(const std::string& msg)
    {
        printf("tid = %d, Foo:m_num = %f, message = %s\n", CurrentThread::Tid(), m_value, msg.c_str());
    }

private:
    double m_value;
};

void TestMemberFunc()
{
    Util::PrintTitle("Test thread for member func without argument");
    Foo foo(12.75); 
    Thread t(std::bind(&Foo::memberFunc, &foo));
    t.start();
    printf("tid = %d\n", t.tid());
    t.join();
}

void TestMemberFuncWithArg()
{
    Util::PrintTitle("Test thread for member func with argument");
    Foo foo(12.75); 
    Thread t(std::bind(&Foo::memberFuncWithArg, &foo, "yhy"));
    t.start();
    printf("tid = %d\n", t.tid());
    t.join();
}


void TestSpecialCase()
{   
    Util::PrintTitle("Test Special Case");
    /// t1 may destruct eariler than thread creation.
    {
        Thread t1(ThreadFunc2);
        t1.start();
    }

    Util::SleepSec(2);

    /// t2 destruct later than thread creation.
    {
        Thread t2(ThreadFunc2);
        t2.start();
    }

    Util::SleepSec(2);
    printf("Number of created threads: %d\n", Thread::numCreated());
}

int main()
{
    TestFreeFunc();
    TestFreeFuncWithArg();
    TestMemberFunc();
    TestMemberFuncWithArg();
    TestSpecialCase();
    return 0;
}