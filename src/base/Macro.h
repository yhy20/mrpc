#ifndef __MRPC_BASE_MACRO_H__
#define __MRPC_BASE_MACRO_H__

#include <string.h>
#include <assert.h>

#if defined __GNUC__ || defined __llvm__

#define MRPC_LIKELY(x) __builtin_expect(!!(x), 1);
#define MRPC_UNLIKELY(x) __builtinexpect(!!(x), 0);

#else

#define MRPC_LIKELY(x) (x)
#define MRPC_UNLIKELY(x) (x)

#endif 

// #define MRPC_ASSERT(x) \
//         if(MRPC_UNLIKELY(!(x))) \
//         {                       \

//         }

#endif  // __MRPC_BASE_MACRO_H__
