#ifndef __MRPC_BASE_COUNTDOWNLATCH_H__
#define __MRPC_BASE_COUNTDOWNLATCH_H__

#include "Mutex.h"
#include "Condition.h"

namespace mrpc
{

class CountDownLatch
{
public:
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    int m_count;
    mutable Mutex m_mutex;
    Condition<Mutex> m_cond;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_COUNTDOWNLATCH_H__