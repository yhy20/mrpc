#include "CountDownLatch.h"

namespace mrpc
{

CountDownLatch::CountDownLatch(int count)
  : m_count(count),
    m_mutex(),
    m_cond(m_mutex) 
{ }

void CountDownLatch::wait()
{
    LockGuard<Mutex> lock(m_mutex);
    while(m_count > 0)
    {
        m_cond.wait();
    }
}

void CountDownLatch::countDown()
{
    LockGuard<Mutex> lock(m_mutex);
    --m_count;
    if(m_count == 0)
    {
        m_cond.notifyAll();
    }
}

int CountDownLatch::getCount() const 
{
    LockGuard<Mutex> lock(m_mutex);
    return m_count;
}

}  // namespace mrpc