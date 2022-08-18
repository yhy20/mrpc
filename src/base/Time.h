#ifndef __MRPC_BASE_TIME_H__
#define __MRPC_BASE_TIME_H__

#include <time.h>
#include <thread>

namespace mrpc
{

class ClockTime
{
public:
    void start()
    {
        m_startTime = clock();
    }

    void stop()
    {
        m_stopTime = clock();
    }

    double duration()
    {
        return static_cast<double>(m_stopTime - m_startTime) / (CLOCKS_PER_SEC);
    }   

private:
    clock_t m_startTime;
    clock_t m_stopTime;
};

/// TODO: 确定 chrono 使用方法 
class WallTime
{
public:
    void start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void stop()
    {
        m_stopTime = std::chrono::high_resolution_clock::now();
    }

    double duration()
    {
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(m_stopTime - m_startTime);
        return static_cast<double>(nanoseconds.count()) / s_nanosecondsPerSecond;
    }

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_stopTime;
    static const int s_nanosecondsPerSecond = 1000 * 1000 * 1000;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_TIME_H__