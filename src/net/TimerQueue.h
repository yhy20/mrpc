#ifndef __MRPC_NET_TIMEQUEUE_H__
#define __MRPC_NET_TIMEQUEUE_H__

#include <set>
#include <vector>

#include <Mutex.h>
#include <Channel.h>
#include <TimeStamp.h>
#include <Callbacks.h>

namespace mrpc
{
namespace net
{
class Timer;
class TimerId;
class EventLoop;

class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    /**
     * @brief 规划 callback 在指定时间运行，如果 interval > 0.0，则重复运行
     */
    TimerId addTimer(TimerCallback cb,
                     TimeStamp when,
                     double interval);
    
    void cancel(TimerId timerId);

private:
    typedef std::pair<TimeStamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    // called when timerfd alarms
    void handleRead();

    std::vector<Entry> getExpired(TimeStamp now);
    void reset(const std::vector<Entry>& expired, TimeStamp now);

    bool insert(Timer* timer);

private:
    EventLoop*  m_loop;             // 定时器队列所属的 EventLoop
    const int   m_timerFd;          // 定时器使用的唤醒 fd
    Channel     m_timerFdChannel;   // 定时器 Channel

    TimerList   m_timers;           // 定时器队列

    ActiveTimerSet  m_activeTimers;             // 活跃的定时器
    bool            m_callingExpiredTimers;     // 是否在调用活跃任务
    ActiveTimerSet  m_cancelingTimers;          // 取消定时器
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TIMEQUEUE_H__