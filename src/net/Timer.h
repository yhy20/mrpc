#ifndef __MRPC_NET_TIMER_H__
#define __MRPC_NET_TIMER_H__

#include "Atomic.h"
#include "TimeStamp.h"
#include "Callbacks.h"

namespace mrpc
{
namespace net
{

/**
 * @brief 内部使用的 timer 事件类
 */
class Timer : noncopyable
{
public:
    /**
     * @brief 构造函数，创建一个定时器
     */
    Timer(TimerCallback cb, TimeStamp when, double interval)
        : m_callback(std::move(cb)),
          m_interval(interval),
          m_repeat(interval > 0.0),
          m_sequence(s_numCreated.incrementAndGet()),
          m_expiration(when) { }

    /**
     * @brief 运行定时器任务回调
     */
    void run() const { m_callback(); }

    /**
     * @brief 返回定时器的超时时刻
     */
    TimeStamp expiration() const { return m_expiration; }

    /**
     * @brief 返回定时器任务是否重复调用
     */
    bool repeat() const { return m_repeat; }

    /**
     * @brief 定时器的序号
     */
    int64_t sequence() const { return m_sequence; }

    /**
     * @brief 依据 now 时间重置需要重复执行的定时器
     */
    void restart(TimeStamp now);

    /**
     * @brief 获取目前已创建的定时器个数
     */
    static int64_t numCreated() { return s_numCreated.load(); }

private:
    static AtomicInt64  s_numCreated;   // 已创建定时器的个数

    const TimerCallback m_callback;     // 定时器任务回调函数
    const double        m_interval;     // 定时器任务重复调用的时间间隔
    const bool          m_repeat;       // 定时器任务是否重复调用
    const int64_t       m_sequence;     // 定时器的序号
    TimeStamp           m_expiration;   // 定时器的超时时刻
};


}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TIMER_H__
