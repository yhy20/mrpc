#ifndef __MRPC_BASE_NONCOPYABLE_H__
#define __MRPC_BASE_NONCOPYABLE_H__

namespace mrpc
{

class noncopyable
{
public:
    noncopyable(const noncopyable& rhs) = delete;
    noncopyable& operator=(const noncopyable& rhs) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_NONCOPYABLE_H__
