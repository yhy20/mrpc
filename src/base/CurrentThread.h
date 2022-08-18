#ifndef __MRPC_BASE_CURRENTTHREAD_H__
#define __MRPC_BASE_CURRENTTHREAD_H__

#include "Types.h"

namespace mrpc
{
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

#define LIKELY(expression) __builtin_expect(expression, 1)
#define UNLIKELY(expression) __builtin_expect(expression, 0)

    if(UNLIKELY(internal::t_tid == 0))
    {   
        CachedTid();
    }

#undef LIKELY
#undef UNLIKELY 

    return internal::t_tid;
}

inline const char*  TidString()
{
    return internal::t_tidString;
}

inline int TidStringLength()
{
    return internal::t_tidStringLength;
}

inline const char* ThreadName()
{
    return internal::t_threadName;
}

bool IsMainThread();

}  // namespace CurrentThread
}  // namespace mrpc

#endif  // __MRPC_BASE_CURRENTTHREAD_H__