#ifndef __MRPC_BASE_WEAKCALLBACK_H__
#define __MRPC_BASE_WEAKCALLBACK_H__

#include <memory>
#include <functional>

namespace mrpc
{

template<typename CLASS, typename... ARGS>
class WeakCallback
{
public:
    WeakCallback(const std::weak_ptr<CLASS>& object,
                 const std::function<void(CLASS*, ARGS ...)>& function)
        : m_object(object), m_function(function) { }
    
    void operator()(ARGS&&... args) const
    {
        std::shared_ptr<CLASS> ptr(m_object.lock());
        if(ptr)
        {
            m_function(ptr.get(), std::forward<ARGS>(args)...);
        }    
        // else
        // {
        //   LOG_TRACE << "expired";
        // }
    }
private:
    std::weak_ptr<CLASS> m_object;
    std::function<void (CLASS*, ARGS...)> m_function;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...))
{
    return WeakCallback<CLASS, ARGS...>(object, function);
}

template<typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(const std::shared_ptr<CLASS>& object,
                                              void (CLASS::*function)(ARGS...) const)
{
    return WeakCallback<CLASS, ARGS...>(object, function);
}

}  // namespace mrpc

#endif  // __MRPC_BASE_WEAKCALLBACK_H__