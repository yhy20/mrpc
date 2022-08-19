#ifndef __MRPC_BASE_CURRENTTHREAD_H__
#define __MRPC_BASE_CURRENTTHREAD_H__

#include "Types.h"

namespace mrpc
{

/// 线程相关函数集合
namespace CurrentThread
{
namespace internal
{

extern thread_local int t_tid;
extern thread_local char t_tidString[32];
extern thread_local int t_tidStringLength;
extern thread_local const char* t_threadName;

}  // namespace internal

void CachedTid();

inline int Tid()
{

/**
 * Use __builtin_expect to provide the compiler with branch prediction information.
 * t_tid 一但被初始化后，就满足 t_tid != 0，所以下列 if 语句条件 t_tid == 0 语句是 UNLIKELY 的，
 * 或者说 t_tid != 0 是 LIKELY 的， 更多关于 __builtin_expect 的讨论见 stackoverflow:
 * https://stackoverflow.com/questions/7346929/what-is-the-advantage-of-gccs-builtin-expect-in-if-else-statements
 */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

    if(UNLIKELY(internal::t_tid == 0))
    {   
        CachedTid();
    }

#undef LIKELY
#undef UNLIKELY 

    return internal::t_tid;
}

/**
 * @brief 返回字符串表示的线程 id，用于日志
 */
inline const char*  TidString()
{
    return internal::t_tidString;
}

/**
 * @brief 返回字符串线程 id 长度，用于优化日志
 */
inline int TidStringLength()
{
    return internal::t_tidStringLength;
}

/**
 * @brief 返回当前线程名称，主线程默认名称为 main
 */
inline const char* ThreadName()
{
    return internal::t_threadName;
}

/**
 * @brief 判断是否为主线程
 */
bool IsMainThread();

}  // namespace CurrentThread
}  // namespace mrpc

#endif  // __MRPC_BASE_CURRENTTHREAD_H__