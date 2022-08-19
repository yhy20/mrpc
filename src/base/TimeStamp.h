#ifndef __MRPC_BASE_TIMESTAMP_H__
#define __MRPC_BASE_TIMESTAMP_H__

#include <string>

#include "copyable.h"

namespace mrpc
{

/**
 * @brief 精度为微妙级别的时间戳类
 */
class TimeStamp : public copyable
{
public:
    TimeStamp() : m_microSecondsSinceEpoch(0) { }

    explicit TimeStamp(int64_t microSecondsSinceEpoch)
        : m_microSecondsSinceEpoch(microSecondsSinceEpoch) { }

    /// default copy constructor/copy assignment operator/ destructor are okey

public:
    void swap(TimeStamp& rhs)
    {
        std::swap(m_microSecondsSinceEpoch, rhs.m_microSecondsSinceEpoch);
    }

    std::string toString() const;

    std::string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return m_microSecondsSinceEpoch > 0; }

    int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch; }

    time_t secondsSinceEpoch() const
    { 
        return static_cast<time_t>(m_microSecondsSinceEpoch / s_microSecondsPerSecond); 
    }

    static TimeStamp Now();
    static TimeStamp Invalid()
    {
        return TimeStamp();
    }

    static TimeStamp FromUnixTime(time_t t)
    {
        return FromUnixTime(t, 0);
    }

    static TimeStamp FromUnixTime(time_t t, int microseconds)
    {
        return TimeStamp(static_cast<int64_t>(t) * s_microSecondsPerSecond + microseconds);
    }

    static const int s_microSecondsPerSecond = 1000 * 1000;

public:
    bool operator< (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch < rhs.m_microSecondsSinceEpoch;
    }

    bool operator> (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch > rhs.m_microSecondsSinceEpoch;
    }

    bool operator== (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch == rhs.m_microSecondsSinceEpoch;
    }

    bool operator!= (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch != rhs.m_microSecondsSinceEpoch;
    }

     bool operator<= (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch <= rhs.m_microSecondsSinceEpoch;
    }

    bool operator>= (TimeStamp rhs) const
    {
        return m_microSecondsSinceEpoch >= rhs.m_microSecondsSinceEpoch;
    }

private:
    int64_t m_microSecondsSinceEpoch;
};

/**
 * @brief Gets time difference of two timestamps, result in seconds.
 * @param[in] high 被减时间戳
 * @param[in] low 减去时间戳
 */
inline double TimeDifference(TimeStamp high, TimeStamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / TimeStamp::s_microSecondsPerSecond;
}

/**
 * @brief Add seconds to given timestamp.
 * @param[in] timestamp 时间戳
 * @param[in] seconds 增加的秒数
 * @return timestamp + seconds as TimeStamp
 * @details addtime 后有可能溢出，导致错误
 */
inline TimeStamp AddTime(TimeStamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimeStamp::s_microSecondsPerSecond);
    return TimeStamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace mrpc

#endif  // __MRPC_BASE_TIMESTAMP_H__