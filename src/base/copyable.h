#ifndef __MRPC_BASE_COPYABLE_H__
#define __MRPC_BASE_COPYABLE_H__

namespace mrpc 
{

/**
 * @brief A tag class emphasises the objects are copyable.
 */
class copyable
{
protected:
    copyable() = default;
    ~copyable() = default;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_COPYABLE_H__
