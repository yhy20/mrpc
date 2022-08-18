#ifndef __MRPC_BASE_EXCEPTION_H__
#define __MRPC_BASE_EXCEPTION_H__

#include <string>
#include <exception>

namespace mrpc
{

class Exception : public std::exception
{
public:
    Exception(std::string what);
    ~Exception() noexcept override = default;

    /// default copy constructor/copy assignment operator/destructor are okay.

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

    const char* stackTrace() const noexcept
    {
        return m_stack.c_str();
    }

private:
    std::string m_message;
    std::string m_stack;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_EXCEPTION_H__