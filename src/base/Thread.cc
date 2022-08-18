#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <iostream>

#include "Thread.h"
#include "Exception.h"
#include "CurrentThread.h"

namespace mrpc
{
namespace details
{

pid_t GetTid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void AfterFork()
{
    CurrentThread::internal::t_tid = 0;
    CurrentThread::internal::t_threadName = "main";
    CurrentThread::Tid();
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer()
    {
        CurrentThread::internal::t_threadName = "main";
        CurrentThread::Tid();
        pthread_atfork(nullptr, nullptr, &AfterFork);
    }

    ~ThreadNameInitializer() = default;
};

ThreadNameInitializer init;

struct ThreadData
{
    typedef Thread::ThreadTask ThreadTask;
    ThreadTask m_task;
    std::string m_name;
    pid_t* m_tid;
    CountDownLatch* m_latch;

    ThreadData(ThreadTask task,
               const std::string& name,
               pid_t* tid,
               CountDownLatch* latch)
        : m_task(std::move(task)),
          m_name(name),
          m_tid(tid),
          m_latch(latch)
    { }

    void runInThread()
    {
        *m_tid = CurrentThread::Tid();
        m_tid = nullptr;
        m_latch->countDown();
        m_latch = nullptr;

        CurrentThread::internal::t_threadName = m_name.c_str();
        ::prctl(PR_SET_NAME, CurrentThread::internal::t_threadName);
        try
        {
            m_task();
            CurrentThread::internal::t_threadName = "finished";
        }
        catch (const Exception& ex)
        {
            CurrentThread::internal::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", m_name.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            abort();
        }
        catch (const std::exception& ex)
        {
            CurrentThread::internal::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", m_name.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            CurrentThread::internal::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", m_name.c_str());
            throw; // rethrow
        }
    }
};

void* StartThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return nullptr;
}

}  // namespace details

void CurrentThread::CachedTid()
{
    if(internal::t_tid == 0)
    {
        internal::t_tid = details::GetTid();
        internal::t_tidStringLength = \
            snprintf(internal::t_tidString,
                     sizeof(internal::t_tidString), 
                     "[%5d]", internal::t_tid);
    }
}

std::atomic<int> Thread::s_numCreated(0);

Thread::Thread(ThreadTask task, StringArg name)
    : m_started(false),
      m_joined(false),
      m_pthreadId(0),
      m_tid(0),
      m_task(std::move(task)),
      m_name(name.c_str()),
      m_latch(1)     
{
    setDefaultName();
}

Thread::~Thread()
{
    if(m_started && !m_joined)
    {
        pthread_detach(m_pthreadId);
    }
}

void Thread::setDefaultName()
{
    int num = s_numCreated.fetch_add(1, std::memory_order_relaxed);
    if (m_name.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        m_name = buf;
    }
}

void Thread::start()
{
    assert(!m_started);
    m_started = true;
    details::ThreadData* data = \
        new details::ThreadData(m_task, m_name, &m_tid, &m_latch);

    if(pthread_create(&m_pthreadId, nullptr, &details::StartThread, data))
    {
        m_started = false;
        delete data;
        // TODO: add LOG
        std::cerr << "Failed in pthread_create!";
    }
    else
    {
        m_latch.wait();
        assert(m_tid > 0);
    }
}

int Thread::join()
{
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    return pthread_join(m_pthreadId, nullptr);
}

}  // namespace mrpc