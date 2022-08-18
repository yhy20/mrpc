#ifndef __MRPC_BASE_CONDITION_H__
#define __MRPC_BASE_CONDITION_H__

#include <pthread.h>
#include <iostream>

#include "Mutex.h"


namespace mrpc
{

template <typename _Mutex>
class Condition : noncopyable
{
public:
    explicit Condition(_Mutex& mutex)
        : m_mutex(mutex)
    {
        pthread_cond_init(&m_pcond, nullptr);
    }

    ~Condition()
    {
        pthread_cond_destroy(&m_pcond);
    }

    void wait()
    {
        pthread_cond_wait(&m_pcond, m_mutex.native_handle());
    }

    bool waitForSeconds(double seconds);

    void notify()
    {
        pthread_cond_signal(&m_pcond);
    }

    void notifyAll()
    {
        pthread_cond_broadcast(&m_pcond);
    }

private:
    _Mutex& m_mutex;
    pthread_cond_t m_pcond;
};

template <typename _Mutex>
bool Condition<_Mutex>::waitForSeconds(double seconds)
{
    struct timespec time;
    /// FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
    clock_gettime(CLOCK_REALTIME, &time);

    static const int64_t s_nanoSecondsPerSecond = 1000 * 1000 * 1000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * s_nanoSecondsPerSecond);

    time.tv_sec += static_cast<time_t>((time.tv_nsec + nanoseconds) / s_nanoSecondsPerSecond);
    time.tv_nsec = static_cast<long>((time.tv_nsec + nanoseconds) % s_nanoSecondsPerSecond);

    return ETIMEDOUT == pthread_cond_timedwait(&m_pcond, m_mutex.native_handle(), &time);
}

template <>
void Condition<AssertMutex>::wait();

template <>
bool Condition<AssertMutex>::waitForSeconds(double seconds);

}  // namespace mrpc

#endif  // __MRPC_BASE_CONDITION_H__