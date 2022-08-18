#include "EventLoop.h"
#include "EventLoopThread.h"

namespace mrpc
{
namespace net
{
EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 StringArg name)
    : m_loop(nullptr),
      m_mutex(),
      m_cond(m_mutex),
      m_callback(cb),
      m_thread(std::bind(&EventLoopThread::threadTask, this), name) { }

/**
 *  线程安全问题详细分析
 *  以下问题一假设 startLoop 与 ~EventLoopThread 在不同线程同时执行，正常情况下这两个函数
 *  应该在同一线程依次执行，不会发生错误。
 * 
 * (1) 问题一、由于在执行 threadTask 函数的 m_loop = &loop 语句前 m_loop 并未被初始化
 *     所以，如果在 threadTask 函数执行过程中，EventLoopThread 对象执行了析构函数，线
 *     程析构时会默认执行 detach 将子线程和主线程分离，主线程中 EventLoopThread 的所有
 *     成员变量都会被析构，这可能导致子线程中的 threadTask 函数中访问已析构的 m_callback
 *     、m_mutex、m_cond 等成员变量导致非法内存访问错误。这个问题本质上是主线程和子线程
 *     detach 后子线程尝试访问主线程中已经析构的成员
 * 
 *  想要要避免上述情况发生，就必须保证 startLoop 函数正常执行完毕前，EventLoopThread 对
 *  象不能被析构，最简单的方法就是让 startLoop 函数只在创建 EventLoopThread 对象的线程执行
 * 
 * (2) 问题二、由于 threadTask 线程任务中局部对象 EventLoop 会提供给其他线程使用，如果在其
 *     他线程的使用过程中，EventLoopThread 对象析构且导致线程结束 EventLoop 对象也析构，则
 *     其他线程会通过指针访问已析构的 EventLoop 对象，造成非法内存访问错误。
 * 
 *  解决上述问题的方法只能是控制 EventLoopThread 对象的生命周期来满足其他线程的调用访问
 */
EventLoopThread::~EventLoopThread()
{
    if(m_loop != nullptr)
    {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    assert(!m_thread.started());
    m_thread.start();

    EventLoop* loop = nullptr;

    /// 等待线程的相关初始化执行完毕
    {
        LockGuard<Mutex> lock(m_mutex);
        while(m_loop == nullptr)
        {
            m_cond.wait();
        }
        loop = m_loop;
    }

    return loop;
}

void EventLoopThread::threadTask()
{
    EventLoop loop;
    if(m_callback)
    {
        m_callback(&loop);
    }

    {
        LockGuard<Mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify();
    }

    loop.loop();
    
    /// EventLoopThread 类使用正确的情况下，下面加锁意义不大
    /// LockGuard<Mutex> lock(m_mutex);
    m_loop = nullptr;
}

}  // namespace net
}  // namespace mrpc