#ifndef __MRPC_BASE_THREAD_H__
#define __MRPC_BASE_THREAD_H__

#include <atomic>
#include <functional>

#include "StringPiece.h"
#include "CountDownLatch.h"

namespace mrpc
{

/**
 * @brief 线程类
 */
class Thread : noncopyable
{
public:
    typedef std::function<void ()> ThreadTask;

    explicit Thread(ThreadTask task, StringArg name = "");

    ~Thread();

    void start();

    int join();

    bool started() const { return m_started; }

    pid_t tid() const { return m_tid; }

    const std::string& name() const { return m_name; }

    static int numCreated() { return s_numCreated.load(std::memory_order_relaxed); }

private:
    void setDefaultName();

private:
    bool            m_started;
    bool            m_joined;
    pthread_t       m_pthreadId;
    pid_t           m_tid;
    ThreadTask      m_task;
    std::string     m_name;
    CountDownLatch  m_latch;
    
    static std::atomic<int> s_numCreated;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_THREAD_H__