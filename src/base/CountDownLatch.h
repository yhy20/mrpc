#ifndef __MRPC_BASE_COUNTDOWNLATCH_H__
#define __MRPC_BASE_COUNTDOWNLATCH_H__

#include "Mutex.h"
#include "Condition.h"

namespace mrpc
{
/**
 * @brief 用于一次性同步的倒计时锁
 */
class CountDownLatch
{
public:
    /**
     * @brief 构造函数
     * @param[in] count CountDown 次数
     */
    explicit CountDownLatch(int count);

    void wait();

    void countDown();

    int getCount() const;

private:
    int                 m_count;    // CountDown 次数
    mutable Mutex       m_mutex;    // 互斥锁
    Condition<Mutex>    m_cond;     // 信号量
};

}  // namespace mrpc

#endif  // __MRPC_BASE_COUNTDOWNLATCH_H__