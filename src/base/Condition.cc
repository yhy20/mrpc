#include "Condition.h"

#include <iostream>

namespace mrpc
{

template <>
void Condition<AssertMutex>::wait()
{
    AssertMutex::UnassignGuard ug(m_mutex);
    pthread_cond_wait(&m_pcond, m_mutex.native_handle());
}

template <>
bool Condition<AssertMutex>::waitForSeconds(double seconds)
{
    struct timespec time;
    // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
    clock_gettime(CLOCK_REALTIME, &time);

    static const int64_t s_nanoSecondsPerSecond = 1000000000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * s_nanoSecondsPerSecond);

    time.tv_sec += static_cast<time_t>((time.tv_nsec + nanoseconds) / s_nanoSecondsPerSecond);
    time.tv_nsec = static_cast<long>((time.tv_nsec + nanoseconds) % s_nanoSecondsPerSecond);

    AssertMutex::UnassignGuard ug(m_mutex);
    return ETIMEDOUT == pthread_cond_timedwait(&m_pcond, m_mutex.native_handle(), &time);
}

}  // namespace mrpc