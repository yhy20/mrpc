#ifndef __MRPC_BASE_THREADPOOL_H__
#define __MRPC_BASE_THREADPOOL_H__

#include <deque>
#include <vector>
#include <string>
#include <memory>

#include "Mutex.h"
#include "Thread.h"
#include "Condition.h"
#include "StringPiece.h"

namespace mrpc
{

class ThreadPool : noncopyable
{
    typedef std::vector<std::unique_ptr<Thread>> Threads;
public:
    typedef std::function<void()> ThreadInitCallBack;
    typedef std::function<void()> ThreadTask;

    explicit ThreadPool(StringArg name = "ThreadPool");
    ~ThreadPool();

    /// Must be called before start().
    void setMaxQueueSize(int maxSize) { }
    void setThreadInitCallback(const ThreadTask& cb) { m_initCallBack = cb; }

    void start(int numThreads);
    void stop();

    const std::string& name() const { return m_name; }
    size_t queueSize() const;
    void run(ThreadTask task);

private:
    bool isFull() const;
    void runInThread();
    ThreadTask outQueue();

private:
    std::string m_name;
    mutable AssertMutex m_mutex;
    Condition<AssertMutex> m_notEmpty;
    Condition<AssertMutex> m_notFull;
    ThreadInitCallBack m_initCallBack;
    Threads m_threads;
    std::deque<ThreadTask> m_queue;
    size_t m_maxQueueSize;
    bool m_running;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_THREADPOOL_H__