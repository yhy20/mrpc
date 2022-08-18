#ifndef __MRPC_BASE_MUTEX_H__
#define __MRPC_BASE_MUTEX_H__

#include <assert.h>
#include <semaphore.h>

#include "noncopyable.h"
#include "CurrentThread.h"

namespace mrpc
{

/**
 * @brief 信号量
 */
class Semaphore : noncopyable 
{
public:
    /**
     * @brief 构造函数
     * @param[in] value 初始信号值
     */
    Semaphore(uint32_t value = 0)
    {
        sem_init(&m_semaphore, 0, value);
    }

    /**
     * @brief 析构函数
     */
    ~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }

    /**
     * @brief 获取信号量
     */
    void wait()
    {
        sem_wait(&m_semaphore);
    }

    /**
     * @brief 释放信号量
     */
    void signal()
    {
        sem_post(&m_semaphore);
    }

private:
    sem_t m_semaphore;
};


class Mutex : noncopyable
{
public:
    Mutex()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t* native_handle()
    {
        return &m_mutex;
    } 

private:
    pthread_mutex_t m_mutex;
};

template <typename _Mutex>
class Condition;

class AssertMutex : noncopyable
{
public:
    AssertMutex() : m_holder(0)
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~AssertMutex()
    {
        assert(m_holder == 0);
        pthread_mutex_destroy(&m_mutex);
    }

    bool isLockedByThisThread() const
    {
        return m_holder == CurrentThread::Tid();
    }

    void assertLocked() const
    {
        assert(isLockedByThisThread());
    }

    void lock()
    {
        pthread_mutex_lock(&m_mutex);
        assignHolder();
    }

    void unlock()
    {
        unassignHolder();
        pthread_mutex_unlock(&m_mutex);
    }

    pthread_mutex_t* native_handle()
    {
        return &m_mutex;
    }

private:
    friend class Condition<AssertMutex>;
    friend class AssertCondition;
    
    class UnassignGuard : noncopyable
    {
    public:
        explicit UnassignGuard(AssertMutex& owner)
            : m_owner(owner)
        {
            m_owner.unassignHolder();
        }

        ~UnassignGuard()
        {
            m_owner.assignHolder();
        }

    private:
        AssertMutex& m_owner;
    };

private:
    void unassignHolder()
    {
        m_holder = 0;
    }

    void assignHolder()
    {
        m_holder = CurrentThread::Tid();
    }

private:
    pthread_mutex_t m_mutex;
    pid_t m_holder;
};

class SpinLock : noncopyable
{
public:
    SpinLock()
    {
        pthread_spin_init(&m_mutex, 0);
    }

    ~SpinLock()
    {
        pthread_spin_destroy(&m_mutex);
    }

    void lock()
    {
        pthread_spin_lock(&m_mutex);
    }

    void unlock()
    {
        pthread_spin_unlock(&m_mutex);
    }

    
private:
    pthread_spinlock_t m_mutex;
};

template <typename T>
class LockGuard
{
public:
    explicit LockGuard(T& mutex) 
        : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    ~LockGuard()
    {
        m_mutex.unlock();
    }

private:
    T& m_mutex;
};

template <typename T>
class UniqueLock
{
public:
    UniqueLock(T& mutex)
        : m_mutex(mutex),
          m_locked(false)
    {
        m_mutex.lock();
        m_locked = true;
    }

    ~UniqueLock()
    {
        unlock();
    }

    void lock()
    {
        if(!m_locked)
        {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock()
    {
        if(m_locked)
        {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_MUTEX_H__