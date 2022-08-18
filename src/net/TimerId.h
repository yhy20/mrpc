#ifndef __MRPC_NET_TIMERID_H__
#define __MRPC_NET_TIMERID_H__

#include <stdint.h>

#include "copyable.h"

namespace mrpc
{
namespace net
{

class Timer;

/**
 * @brief 定时器标识类，唯一的标识一个定时器，用于取消一个已添加的定时任务
 */
class TimerId : public copyable
{
public:
    /**
     * @brief 构造函数，构造一个非法的 TimerId
     */
    TimerId() 
        : m_timer(nullptr),
          m_sequence(0) { }

    /**
     * @brief 构造函数
     * @param[in] timer 指向定时器的指针
     * @param[in] seq timer 指向定时器的序号
     */
    TimerId(Timer* timer, int64_t seq)
        : m_timer(timer),
          m_sequence(seq) { } 

    friend class TimerQueue;

private:
    Timer*  m_timer;        // 定时器
    int64_t m_sequence;     // 定时器序号
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_TIMERID_H__