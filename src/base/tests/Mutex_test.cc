#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <functional>

#include "Util.h"
#include "Mutex.h"
#include "StringPiece.h"

using namespace mrpc;

template <typename MUTEX, typename LOCK_GUARD = LockGuard<MUTEX>>
void BasicTest(StringArg title)
{   
    Util::PrintTitle(title);
    const int max = 10000;
    const int threadNum = 10;
    int sum = 0;
    MUTEX mtx;
    auto threadTask = [&]()->void {
        for(int i = 0; i < max; ++i)
        {
            LOCK_GUARD lock(mtx);
            ++sum;
        }
    };
    
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    for(int i = 0; i < threadNum; ++i)
    {
        threads.push_back(std::thread(threadTask));
    } 
    for(int i = 0; i < threadNum; ++i)
    {
        threads[i].join();
    }
    std::cout << "sum = " << sum << std::endl;
}

template <typename MUTEX>
class FooBar 
{
public:
    FooBar(int n)
    {
        m_num = n;
        m_mtxBar.lock();
    }

    ~FooBar()
    {
        m_mtxBar.unlock();
    }

    void foo(std::function<void(int)>& printFoo)
    {
        for(int i = 0; i < m_num; i++)
        {
            m_mtxFoo.lock();
            printFoo(i + 1);
            m_mtxBar.unlock();
        }
    }

    void bar(std::function<void(int)>& printBar)
    {
        for(int i = 0; i < m_num; i++)
        {
            m_mtxBar.lock();
            printBar(i + 1);
            m_mtxFoo.unlock();
        }
    }

private:
    int m_num;
    MUTEX m_mtxFoo, m_mtxBar;
};

template <typename T>
void TestFooBar(int num, StringArg title)
{   
    Util::PrintTitle(title);
    std::function<void(int)> printFoo = [](int i) -> void { printf("num%d: Foo\n", i); };
    std::function<void(int)> printBar = [](int i) -> void { printf("num%d: Bar\n", i); };
    FooBar<T> obj(num);
    std::thread t1(&FooBar<T>::foo, &obj, ref(printFoo));
    std::thread t2(&FooBar<T>::bar, &obj, ref(printBar));
    t1.join();
    t2.join();
}

void TestLockGuard()
{
    BasicTest<Mutex>("Test Mutex");
    BasicTest<std::mutex>("Test std::mutex");
    BasicTest<AssertMutex>("Test AssertMutex");
    BasicTest<SpinLock>("Test SpinLock");
}

void TestUniqueLock()
{
    BasicTest<Mutex, UniqueLock<Mutex>>("Test Mutex");
    BasicTest<std::mutex, UniqueLock<std::mutex>>("Test std::mutex");
    BasicTest<AssertMutex, UniqueLock<AssertMutex>>("Test AssertMutex");
    BasicTest<SpinLock, UniqueLock<SpinLock>>("Test SpinLock");
}

void TestFooBarPrint()
{
    TestFooBar<Mutex>(10, "Test Mutex");
    TestFooBar<std::mutex>(10, "Test std::mutex");
    TestFooBar<AssertMutex>(10, "Test AssertMutex");
    TestFooBar<SpinLock>(10, "Test SpinLock");
}

int main()
{
    TestLockGuard();
    TestUniqueLock();
    TestFooBarPrint();
    
    return 0;
}