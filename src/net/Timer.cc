#include "Timer.h"

namespace mrpc
{
namespace net
{

AtomicInt64 Timer::s_numCreated;

void Timer::restart(TimeStamp now)
{
    if(m_repeat)
    {
        m_expiration = AddTime(now, m_interval);
    }
    else
    {
        m_expiration = TimeStamp::Invalid();
    }
}

}  // namespace net
}  // namespace mrpc