#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <iostream>
#include <functional>
#include <condition_variable>

#include "Util.h"
#include "Mutex.h"
#include "Condition.h"
#include "StringPiece.h"

using namespace mrpc;

template <typename _Mutex>
void NotifyAll(StringArg title)
{
    Util::PrintTitle(title);
    const int threadNum = 10;

    /// 防止虚假唤醒
    bool ok = false;

    /// 条件变量
    _Mutex mtx;
    Condition<_Mutex> cond(mtx);

    auto threadTask = [&](int num)->void {
        LockGuard<_Mutex> lock(mtx);

        /// 防止虚假唤醒
        while(!ok)
        {
            cond.wait();
        }
        std::cout << "Thread" << num << " run!" << std::endl;
    };
    
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    for(int i = 0; i < threadNum; ++i)
    {
        threads.push_back(std::thread(threadTask, i + 1));
    } 

    /// 防止信号丢失
    Util::SleepSec(1);

    {
        LockGuard<_Mutex> lock(mtx);
        /// 唤醒所有线程
        ok = true;
        cond.notifyAll();
    }
    
    for(int i = 0; i < threadNum; ++i)
    {
        threads[i].join();
    }
}

/**
 * @brief 测试 notifyAll 函数
 */
void TestNotifyAll()
{
    NotifyAll<Mutex>("Test mrpc::Condition<mrpc::Mutex>");
    NotifyAll<std::mutex>("Test mrpc::Condition<std::mutex>");
    NotifyAll<AssertMutex>("Test mrpc::Condition<mrpc::AssertMutex>");
}

/**
 * @brief 交替打印 Foo、Bar 类
 * @details 不能随意传入模板参数，可行的组合如下
 *          mrpc::Condition<T> 可以和 std::mutex, mrpc::Mutex, mrpc::AssertMutex 一同使用
 *          std::condition_variable 只能和 std::unique_lock<std::mutex> 一同使用
 *          
 */
template <typename _Mutex, typename _Condition>
class FooBar 
{
public:
    /// conditionWait 参数只用于偏特化的类
    FooBar(int n, bool conditionWait)
        : m_num(n),
          m_isFoo(true),
          m_mutex(),
          m_cond(m_mutex) { }

    ~FooBar() { }

    void foo(std::function<void(int)>& printFoo)
    {
        for(int i = 0; i < m_num; i++)
        {
            LockGuard<_Mutex> lock(m_mutex);
            while(!m_isFoo)
            {
                m_cond.wait();
            }
            printFoo(i + 1);
            m_isFoo = false;
            m_cond.notify();
        }
    }

    void bar(std::function<void(int)>& printBar)
    {
        for(int i = 0; i < m_num; i++)
        {   
            LockGuard<_Mutex> lock(m_mutex);
            while(m_isFoo)
            {
                m_cond.wait();
            }       
            printBar(i + 1);
            m_isFoo = true;
            m_cond.notify();
        }
    }

private:
    int m_num;

    /// 条件判断外层有锁的保护，不需要使用原子类型
    bool m_isFoo;
    _Mutex m_mutex;
    _Condition m_cond;
};

/**
 * @brief 交替打印 Foo、Bar 类的 <std::mutex, std::condition_variable> 偏特化
 * @details std::condition_variable 只能和 std::unique_lock<std::mutex> 一起使用
 *          可以选择使用 condition_variable 的两种 wait 重载函数
 */
template <>
class FooBar<std::mutex, std::condition_variable>
{
public:
    FooBar(int n, bool conditionWait = false)
        : m_num(n),
          m_isFoo(true),
          m_conditionWait(conditionWait),
          m_mutex(),
          m_cond() { }

    ~FooBar() { }

    void foo(std::function<void(int)>& printFoo)
    {
        for(int i = 0; i < m_num; i++)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_conditionWait)
            {
                m_cond.wait(lock, [this]()->bool { return m_isFoo; });
            }
            else
            {
                while(!m_isFoo)
                {
                    m_cond.wait(lock);
                }
            }
           
            printFoo(i + 1);
            m_isFoo = false;
            m_cond.notify_one();
        }
    }

    void bar(std::function<void(int)>& printBar)
    {
        for(int i = 0; i < m_num; i++)
        {   
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_conditionWait)
            {
                m_cond.wait(lock, [this]()->bool { return !m_isFoo; });
            }
            else
            {
                while(m_isFoo)
                {
                    m_cond.wait(lock);
                }
            }   
            printBar(i + 1);
            m_isFoo = true;
            m_cond.notify_one();
        }
    }

private:
    int m_num;
    bool m_isFoo;
    bool m_conditionWait;
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

template <typename _Mutex, typename _Condition = Condition<_Mutex>>
void FooBarPrint(int num, StringArg title, bool conditionWait = false)
{   
    Util::PrintTitle(title);
    std::function<void(int)> printFoo = [](int i) -> void { printf("num%d: Foo\n", i); };
    std::function<void(int)> printBar = [](int i) -> void { printf("num%d: Bar\n", i); };
    FooBar<_Mutex, _Condition> obj(num, conditionWait);
    std::thread t1(&FooBar<_Mutex, _Condition>::foo, &obj, ref(printFoo));
    std::thread t2(&FooBar<_Mutex, _Condition>::bar, &obj, ref(printBar));
    t1.join();
    t2.join();
}

void TestFooBarPrint()
{
    FooBarPrint<Mutex>(10, "Test mrpc::Condition<mrpc::Mutex>");
    FooBarPrint<std::mutex>(10, "Test mrpc::Condition<std::mutex>");
    FooBarPrint<AssertMutex>(10, "Test mrpc::Condition<mrpc::AssertMutex>");
    FooBarPrint<std::mutex, std::condition_variable>(10, "Test std::condition_variable with wait");
    FooBarPrint<std::mutex, std::condition_variable>(10, "Test std::condition_variable with conditionWait", true);
}   

template <typename _Mutex>
void WaitForSeconds(StringArg title, double waitTime, int notifyTime)
{
    Util::PrintTitle(title);
    const int threadNum = 10;

    /// 条件变量
    _Mutex mtx;
    Condition<_Mutex> cond(mtx);

    auto threadTask = [&]()->void {
        LockGuard<_Mutex> lock(mtx);

        bool timeout = false;
        /// 防止虚假唤醒
        timeout = cond.waitForSeconds(waitTime);
        
        if(timeout)
            std::cout << "timeout!" << std::endl;
        else 
            std::cout << "motifyed by main thread!" << std::endl;

        std::cout << "Thread run!" << std::endl;
    };

    std::thread t(threadTask);
    /// 使等待超时    
    Util::SleepSec(notifyTime);

    std::cout << "notify work thread!" << std::endl;
    /// 唤醒线程
    cond.notify();

    t.join();
}

void TestWaitForSeconds()
{
    WaitForSeconds<Mutex>("Test mrpc::Mutex", 0.5, 1);
    WaitForSeconds<std::mutex>("Test std::mutex", 1.5, 1);
    WaitForSeconds<AssertMutex>("Test mrpc::AssertMutex", 0.5, 1);
}

void TestAssert()
{
    Util::PrintTitle("Test unlock mutex Assert");
    AssertMutex mtx;
    Condition<AssertMutex> cond(mtx);

    std::cout << "Use unlock!" << std::endl;
    mtx.lock();
    cond.waitForSeconds(1);
    mtx.unlock();

    std::cout << "Use LockGuard to unlock mutex!" << std::endl;
    {
        LockGuard<AssertMutex> lock(mtx);
        cond.waitForSeconds(1);
    }

    std::cout << "Use Condition but doesn't unlock!" << std::endl;
    mtx.lock();
    cond.waitForSeconds(1);
}

int main()
{
    TestNotifyAll();
    TestFooBarPrint();
    TestWaitForSeconds();
    TestAssert();
    
    return 0;
}