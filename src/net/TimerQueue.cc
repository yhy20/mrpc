#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <unistd.h>
#include <sys/timerfd.h>

#include "Timer.h"
#include "TimerId.h"
#include "Logging.h"
#include "EventLoop.h"
#include "TimerQueue.h"

namespace mrpc
{
namespace net
{
namespace details
{

int CreateTimerFd()
{
    int timerFd = ::timerfd_create(CLOCK_MONOTONIC, 
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if(-1 == timerFd)
    {
        LOG_SYSERR << "Failed in timerfd_create";
    }
    return timerFd;
}

struct timespec HowMuchTimeFromNow(TimeStamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - \
                            TimeStamp::Now().microSecondsSinceEpoch();

    if(microseconds < 100)
    {
        microseconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / TimeStamp::s_microSecondsPerSecond);
    ts.tv_nsec = static_cast<time_t>(
        (microseconds % TimeStamp::s_microSecondsPerSecond) * 1000);

    return ts;
}

void ReadTimerFd(int timerFd, TimeStamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerFd, &howmany, sizeof(howmany));
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if(n != sizeof(howmany))
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

/**
 * @brief 重置 timerFd 的超时时刻
 * @param[in] timerFd 时间文件描述符
 * @param[in] expiration 超时时刻
 */
void ResetTimerFd(int timerFd, TimeStamp expiration)
{
    /// wake up loop by timerfd_settime.
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memZero(&newValue, sizeof(newValue));
    memZero(&oldValue, sizeof(oldValue));
    newValue.it_value = HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerFd, 0, &newValue, &oldValue);
    if(-1 == ret)
    {   
        LOG_SYSERR << "timerfd_settime()";
    }
}

}  // namespace details

TimerQueue::TimerQueue(EventLoop* loop)
    : m_loop(loop),
      m_timerFd(details::CreateTimerFd()),
      m_timerFdChannel(m_loop, m_timerFd),
      m_timers(),
      m_callingExpiredTimers(false)
{
    m_timerFdChannel.setReadCallback(
        std::bind(&TimerQueue::handleRead, this));

    // we are always reading the timerfd, we disarm it with timerfd_settime.
    m_timerFdChannel.enableReading();
}

TimerQueue::~TimerQueue()
{
    m_timerFdChannel.disableAll();
    m_timerFdChannel.remove();
    ::close(m_timerFd);

    for(const Entry& timer : m_timers)
    {
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,
                             TimeStamp when,
                             double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    m_loop->runInLoop(
        std::bind(&TimerQueue::addTimerInLoop, this, timer));  
    
    return TimerId(timer, timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    m_loop->runInLoop(
        std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    m_loop->assertInLoopThread();
    /// 插入一个定时器可能会使最早到期的时刻发生改变，需要重置 timerFd 的倒计时时间
    bool earliestChanged = insert(timer);
    /// 最早超时时刻确实发生了改变
    if(earliestChanged)
    {   
        /// 重置 timerFd 的倒计时时间
        details::ResetTimerFd(m_timerFd, timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    m_loop->assertInLoopThread();
    assert(m_timers.size() == m_activeTimers.size());
    ActiveTimer timer(timerId.m_timer, timerId.m_sequence);
    ActiveTimerSet::iterator itr = m_activeTimers.find(timer);
    if(itr != m_activeTimers.end())
    {
        size_t n = m_timers.erase(Entry(itr->first->expiration(), itr->first));
        assert(n == 1); (void)n;
        delete itr->first;
        m_activeTimers.erase(itr);
    }
    /// 如果在定时器列表中没有找到，可能已经到期，且正在处理的定时器
    else if(m_callingExpiredTimers)
    {
        m_cancelingTimers.insert(timer);
    }
    assert(m_timers.size() == m_activeTimers.size());
}

void TimerQueue::handleRead()
{
    m_loop->assertInLoopThread();
    TimeStamp now(TimeStamp::Now());
    /// 处理 timerFd 读事件
    details::ReadTimerFd(m_timerFd, now);
    
    /// 获取当前超时了的定时器列表，可能有多个定时器都超时了
    std::vector<Entry> expired = getExpired(now);
    /// 进入定时器处理状态
    m_callingExpiredTimers = true;
    m_cancelingTimers.clear();
    /// 执行所有超时定时器的回调
    for(const Entry& itr : expired)
    {
        itr.second->run();
    }
    /// 退出定时器处理状态
    m_callingExpiredTimers = false;
    reset(expired, now);    
}

/// 返回当前所有超时定时器的列表
std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now)
{
    assert(m_timers.size() == m_activeTimers.size());
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator end = m_timers.lower_bound(sentry);
    assert(end == m_timers.end() || now < end->first);
    std::copy(m_timers.begin(), end, back_inserter(expired));
    m_timers.erase(m_timers.begin(), end);

    for(std::vector<Entry>::iterator itr = expired.begin();
        itr != expired.end(); ++itr)
    {
        ActiveTimer timer(itr->second, itr->second->sequence());
        size_t n = m_activeTimers.erase(timer);
        assert(n == 1); (void)n;
    }
    assert(m_timers.size() == m_activeTimers.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, TimeStamp now)
{
    TimeStamp nextExpire;
    for(const Entry& itr : expired)
    {
        ActiveTimer timer(itr.second, itr.second->sequence());
        if(itr.second->repeat()
           && m_cancelingTimers.find(timer) == m_cancelingTimers.end())
        {
            itr.second->restart(now);
            insert(itr.second);
        }
        else
        {
            delete itr.second;
        }
    }

    if(!m_timers.empty())
    {
        nextExpire = m_timers.begin()->second->expiration();
    }
    if(nextExpire.valid())
    {
        details::ResetTimerFd(m_timerFd, nextExpire);
    }

}

bool TimerQueue::insert(Timer* timer)
{
    m_loop->assertInLoopThread();
    assert(m_timers.size() == m_activeTimers.size());
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    TimerList::iterator itr = m_timers.begin();
    if(itr == m_timers.end() || when < itr->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator, bool> result = 
                            m_timers.insert(Entry(when, timer));
        assert(result.second); (void)result;
    }
    {
        std::pair<ActiveTimerSet::iterator, bool> result =
                            m_activeTimers.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
            
    }

    assert(m_timers.size() == m_activeTimers.size());
    return earliestChanged;
}

}  // namespace net
}  // namespace mrpc



