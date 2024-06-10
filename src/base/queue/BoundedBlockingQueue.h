#ifndef __MRPC_BASE_BOUNDEDBLOCKINGQUEUE_H__
#define __MRPC_BASE_BOUNDEDBLOCKINGQUEUE_H__

#include <assert.h>

#include <memory>

#include "Mutex.h"
#include "Condition.h"

// #include <boost/circular_buffer.hpp>

namespace mrpc
{

// template <typename T>
// class BoundedBlockingQueue : noncopyable
// {
//     typedef boost::circular_buffer<T> BoundedQueue;

// public:
//     explicit BoundedBlockingQueue(int maxSize)
//         : m_mutex(),
//           m_notEmpty(m_mutex),
//           m_notFull(m_mutex),
//           m_queue(maxSize) { }

//     BoundedBlockingQueue(BoundedBlockingQueue&& rhs) = delete;
//     BoundedBlockingQueue&& operator= (BoundedBlockingQueue&& rhs) = delete;

// public: 

//     void push(T data)
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         while(m_queue.full())
//         {
//             m_notFull.wait();
//         }
//         assert(!m_queue.full());
//         m_queue.push_back(std::move(data));
//         m_notEmpty.notify();
//     }

//     void movePush(T&& data)
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         while(m_queue.full())
//         {
//             m_notFull.wait();
//         }
//         assert(!m_queue.full());
//         m_queue.push_back(std::move(data));
//         m_notEmpty.notify();
//     }

//     void pop(T& data)
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         while(m_queue.empty())
//         {
//             m_notEmpty.wait();
//         }
//         assert(!m_queue.empty());
//         data = std::move(m_queue.front());
//         m_queue.pop_front();
//         m_notFull.notify();
//         return data;
//     }

//     std::shared_ptr<T> pop()
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         while(m_queue.empty())
//         {
//             m_notEmpty.wait();
//         }
//         assert(!m_queue.empty());
//         std::shared_ptr<T> data = 
//             std::make_shared<T>(std::move(m_queue.front()));
        
//         m_queue.pop_front();
//         m_notFull.notify();
//         return data;
//     }

//     bool full() const
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         return m_queue.full();
//     }

//     /**
//      * @brief 
//      * @details 若与 pop 等函数一同使用， 有线程安全风险
//      */
//     bool empty() const
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         return m_queue.empty();
//     }
    
//     size_t size() const
//     {
//         LockGuard<Mutex> lock(m_mutex);
//         return m_queue.size();
//     }

//     size_t capacity() const
//     {
//         // LockGuard<Mutex> lock(m_mutex);
//         return m_queue.capacity();
//     }

// private:
//     mutable Mutex m_mutex;
//     Condition<Mutex> m_notEmpty;
//     Condition<Mutex> m_notFull;
//     BoundedQueue m_queue;
// };

}  // namespace mrpc

#endif  // __MRPC_BASE_BOUNDEDBLOCKINGQUEUE_H__