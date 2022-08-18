#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>

#include "TimeStamp.h"

namespace mrpc
{

static_assert(sizeof(TimeStamp) == sizeof(int64_t),
                "Timestamp should be same size as int64_t");

std::string TimeStamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = m_microSecondsSinceEpoch / s_microSecondsPerSecond;
    int64_t microseconds = m_microSecondsSinceEpoch % s_microSecondsPerSecond;
    snprintf(buf, sizeof(buf), "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string TimeStamp::toFormattedString(bool showMicroseconds) const
{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch / s_microSecondsPerSecond);
    struct tm time;
    localtime_r(&seconds, &time);

    if (showMicroseconds)
    {
        int microseconds = static_cast<int>(m_microSecondsSinceEpoch % s_microSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d:%06d",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                 time.tm_hour, time.tm_min, time.tm_sec,
                 microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                 time.tm_hour, time.tm_min,time.tm_sec);
    }
    return buf;
}

TimeStamp TimeStamp::Now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds * s_microSecondsPerSecond + tv.tv_usec);
}

}  // namespace mrpc
