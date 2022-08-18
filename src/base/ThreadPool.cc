#include "ThreadPool.h"
#include "Exception.h"
#include "StringPiece.h"

namespace mrpc
{

ThreadPool::ThreadPool(StringArg name)
    : m_name(name.c_str()),
      m_mutex(),
      m_notEmpty(m_mutex),
      m_notFull(m_mutex),
      m_maxQueueSize(0),
      m_running(false) { }

ThreadPool::~ThreadPool()
{
    if(m_running)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(m_threads.empty());
    m_running = true;
    m_threads.reserve(numThreads);
    for(int i = 0; i < numThreads; ++i)
    {
        char threadNum[32];
        snprintf(threadNum, sizeof(threadNum), "%d", i + 1);
        m_threads.emplace_back(
            new Thread(
                std::bind(&ThreadPool::runInThread, this),
                m_name + ": thread" + threadNum
            )
        );
        m_threads[i]->start();
    }
    if(numThreads == 0 && m_initCallBack)
    {
        m_initCallBack();
    }
}

void ThreadPool::stop()
{
    {
        LockGuard<AssertMutex> lock(m_mutex);
        m_running = false;
        m_notEmpty.notifyAll();
        m_notFull.notifyAll();
    }
    
    for(auto& thread : m_threads)
    {
        thread->join();
    }
}

size_t ThreadPool::queueSize() const
{
    LockGuard<AssertMutex> lock(m_mutex);
    return m_queue.size();
}

void ThreadPool::run(ThreadTask task)
{   
    /// 同步执行任务
    if(m_threads.empty())
    {
        task();
    }
    /// 将任务放入任务队列
    else
    {
        LockGuard<AssertMutex> lock(m_mutex);
        while(isFull() && m_running)
        {
            m_notFull.wait();
        }

        if(!m_running) return;
        assert(!isFull());

        m_queue.push_back(std::move(task));
        m_notEmpty.notify();
    }
}

ThreadPool::ThreadTask ThreadPool::outQueue()
{
    LockGuard<AssertMutex> lock(m_mutex);
    while(m_queue.empty() && m_running)
    {
        m_notEmpty.wait();
    }
    ThreadTask task;
    if(!m_queue.empty())
    {
       task = m_queue.front();
       m_queue.pop_front();
       if(m_maxQueueSize > 0)
       {
        m_notFull.notify();
       } 
    }
    return task;
}

bool ThreadPool::isFull() const 
{
    m_mutex.assertLocked();
    return m_maxQueueSize > 0 && m_queue.size() >= m_maxQueueSize;
}

void ThreadPool::runInThread()
{
    try
    {
        if(m_initCallBack)
        {
            m_initCallBack();
        }
        while(m_running)
        {
            ThreadTask task(outQueue());
            if(task)
            {
                task();
            }
        }
    }
   catch (const Exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        fprintf(stderr, "stack trace: %s\n", e.stackTrace());
        abort();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", m_name.c_str());
        throw; // rethrow
    }
    
}

}  // namespace mrpc