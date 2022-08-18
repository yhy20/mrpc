#ifndef __MRPC_BASE_BLOCKINGQUEUE_H__
#define __MRPC_BASE_BLOCKINGQUEUE_H__

#include <assert.h>

#include <deque>
#include <memory>

#include "Mutex.h"
#include "Condition.h"

namespace mrpc
{

template <typename T>
class BlockingQueue : noncopyable
{
public:
    BlockingQueue()
        : m_mutex(),
          m_notEmpty(m_mutex),
          m_queue() { }

    BlockingQueue(BlockingQueue&& rhs) = delete;
    BlockingQueue& operator = (BlockingQueue&& rhs) = delete;

public:
    void push(T data)
    {
        LockGuard<Mutex> lock(m_mutex);
        m_queue.push_back(std::move(data));
        m_notEmpty.notify();
    }

    void movePush(T&& data)
    {
        LockGuard<Mutex> lock(m_mutex);
        m_queue.push_back(std::move(data));
        m_notEmpty.notify();
    }

    void pop(T& data)
    {
        LockGuard<Mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            m_notEmpty.wait();
        }
        assert(!m_queue.empty());
        data = std::move(m_queue.front());
        m_queue.pop_front(); 
    }

    std::shared_ptr<T> pop()
    {
        LockGuard<Mutex> lock(m_mutex);
        while(m_queue.empty())
        {
            m_notEmpty.wait();
        }
        assert(!m_queue.empty());
        std::shared_ptr<T> data = 
            std::make_shared<T>(std::move(m_queue.front()));

        m_queue.pop_front();
        return data;
    }

    bool tryPop(T& data)
    {
        LockGuard<Mutex> lock(m_mutex);
        if(m_queue.empty())
            return false;
        
        data = std::move(m_queue.front());
        m_queue.pop_front();
        return true; 
    }
    
    std::shared_ptr<T> tryPop()
    {
        LockGuard<Mutex> lock(m_mutex);
        if(m_queue.empty())
            return nullptr;
        
        std::shared_ptr<T> data = 
            std::make_shared<T>(std::move(m_queue.front()));

        m_queue.pop_front();
        return data;
    }

private:
    mutable Mutex m_mutex;
    Condition<Mutex> m_notEmpty;
    std::deque<T> m_queue;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_BLOCKINGQUEUE_H__