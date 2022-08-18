#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

#include <map>
#include <functional>

#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"

using namespace mrpc;
using namespace mrpc::net;

void Print(const char* msg)
{
    static std::map<const char*, TimeStamp> lasts;
    TimeStamp& last = lasts[msg];
    TimeStamp now = TimeStamp::Now();
    printf("%s tid = %d %s delay %f\n",
           now.toFormattedString().c_str(),
           CurrentThread::Tid(), msg,
           TimeDifference(now, last));
    last = now;
}

namespace mrpc
{
namespace net
{
namespace details
{
int CreateTimerFd();
void ReadTimerFd(int timerFd, TimeStamp now);
}
}
}

class PeriodicTiemr
{
public:
    PeriodicTiemr(EventLoop* loop, double interval, const TimerCallback& cb)
        : m_loop(loop),
          m_timerFd(mrpc::net::details::CreateTimerFd()),
          m_timerFdChannel(loop, m_timerFd),
          m_interval(interval),
          m_cb(cb)
    {
        m_timerFdChannel.setReadCallback(
            std::bind(&PeriodicTiemr::handleRead, this));
        m_timerFdChannel.enableReading();
    }

    ~PeriodicTiemr()
    {
        m_timerFdChannel.disableAll();
        m_timerFdChannel.remove();
        ::close(m_timerFd);
    }

    void start()
    {
        struct itimerspec spec;
        memZero(&spec, sizeof(spec));
        spec.it_interval = ToTimeSpec(m_interval);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(m_timerFd, 0 /* relative timer */, &spec, NULL);
        if(ret) LOG_SYSERR << "timerfd_settime() error!";
    }

private:
    void handleRead()
    {
        m_loop->assertInLoopThread();
        mrpc::net::details::ReadTimerFd(m_timerFd, TimeStamp::Now());
        if(m_cb) m_cb();
    }

    static struct timespec ToTimeSpec(double seconds)
    {
        struct timespec ts;
        memZero(&ts, sizeof(ts));
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
        nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

private:
    EventLoop* m_loop;
    const int m_timerFd;
    Channel m_timerFdChannel;
    const double m_interval; // in seconds
    TimerCallback m_cb;
};

int main(void)
{
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::Tid()
             << " Try adjusting the wall clock, see what happens.";
    EventLoop loop;
    PeriodicTiemr timer(&loop, 1, std::bind(Print, "PeriodicTimer"));
    timer.start();
    loop.runEvery(1, std::bind(Print, "EventLoop::runEvery"));
    loop.loop();

    return 0;
}
