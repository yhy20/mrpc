#include <assert.h>

#include <atomic>
#include <thread>
#include <iostream>

#include "Atomic.h"
#include "ThreadPool.h"

using namespace mrpc;
using namespace details;

template <typename T>
void BasicTest()
{
    T a;
    assert(a.load() == 0);
    assert(a.getAndAdd(1) == 0);
    assert(a.load() == 1);
    assert(a.addAndGet(2) == 3);
    assert(a.load() == 3);
    assert(a.incrementAndGet() == 4);
    assert(a.load() == 4);
    a.increment();
    assert(a.load() == 5);
    assert(a.addAndGet(-3) == 2);
    assert(a.store(100) == 2);
    assert(a.load() == 100);
}

template <typename _Atomic>
class Foo
{
public:
    _Atomic count;
    std::atomic<int> acount;

    Foo() : acount(0) { }

    void threadTask()
    {
        for(int i = 0; i < 1000; ++i)
        {
            count.getAndAdd(2);
            count.addAndGet(2);
            count.incrementAndGet();
            count.decrementAndGet();
            count.increment();
            count.decrement();
            count.add(6);
            acount.fetch_add(10, std::memory_order_relaxed);
        }  
    }
};

template <typename _Atomic>
void ThreadSafeTest()
{   
    Foo<_Atomic> f;
    ThreadPool pool;
    pool.start(5);
    for(int i = 0; i < 10; ++i)
    {
        pool.run(std::bind(&Foo<_Atomic>::threadTask, &f));
    }
    while(pool.queueSize() > 0)
    {
        std::this_thread::yield();
    }

    std::cout << "count = " << f.count.load() << std::endl;
    std::cout << "acount = " << f.acount.load() << std::endl;
}

int main()
{
    BasicTest<AtomicInt64>();
    BasicTest<AtomicInt32>();
    ThreadSafeTest<AtomicInt64>();
    ThreadSafeTest<AtomicInt32>();

    return 0;
}