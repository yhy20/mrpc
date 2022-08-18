#ifndef __MRPC_BASE_ATOMIC_H__
#define __MRPC_BASE_ATOMIC_H__

#include <stdint.h>

#include "noncopyable.h"

namespace mrpc
{
namespace details
{
template <typename T>
class AtomicIntergerT : noncopyable
{
public:
    AtomicIntergerT() : m_value(0) { }
    
    T load()
    {
        return __sync_val_compare_and_swap(&m_value, 0, 0);
    }

    T store(T newValue)
    {
        return __sync_lock_test_and_set(&m_value, newValue);
    }

    T getAndAdd(T x)
    {
        return __sync_fetch_and_add(&m_value, x);
    }

    T addAndGet(T x)
    {
        return getAndAdd(x) + x;
    }

    T incrementAndGet()
    {
        return addAndGet(1);
    }

    T decrementAndGet()
    {
        return addAndGet(-1);
    }

    void add(T x)
    {
        getAndAdd(x);
    }

    void increment()
    {
        incrementAndGet();
    }

    void decrement()
    {
        decrementAndGet();
    }

private:
    volatile T m_value;
};

}  // namespace details

typedef details::AtomicIntergerT<int64_t> AtomicInt64;
typedef details::AtomicIntergerT<int32_t> AtomicInt32;

}  // namespace mrpc

#endif  // __MRPC_BASE_ATOMIC_H__