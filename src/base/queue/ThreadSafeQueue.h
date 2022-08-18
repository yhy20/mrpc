#ifndef __MRPC_BASE_THREADSAFEQUEUE_H__
#define __MRPC_BASE_THREADSAFEQUEUE_H__

#include <mutex>
#include <memory>
#include <condition_variable>

namespace mrpc
{

// 参考《C++并发编程实践》实现的细粒度锁的线程安全队列，用来作为异步日志的消息队列
template <typename T>
class ThreadSafeQueue
{ 
public:
    ThreadSafeQueue() : m_head(new node), m_tail(m_head.get()) {}
    ThreadSafeQueue(const ThreadSafeQueue& rhs) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& rhs) = delete;
    ThreadSafeQueue(ThreadSafeQueue&& rhs) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&& rhs) = delete;
    ~ThreadSafeQueue() = default;

public:
    // 使用拷贝构造函数 copy 传参进行 push
    void push(T data);
    // 使用右值引用 move 传参进行 push     
    void movePush(T&& data);
    // 阻塞的 pop 函数，提供两种获取队列数据的接口
    void waitAndPop(T& data);                       
    std::shared_ptr<T> waitAndPop();
    // 非阻塞的 pop 函数，提供两种获取队列数据的接口
    bool tryPop(T& data);   
    std::shared_ptr<T> tryPop();    
    bool empty();

private:
    struct node;

    // 一些仅在 ThreadSafeQueue 内部使用的工具函数
    node* getTail()
    {
        std::lock_guard<std::mutex> tailLock(m_tailMutex);
        return m_tail;
    }

    std::unique_ptr<node> popHead()
    {
        std::unique_ptr<node> oldHead = std::move(m_head);
        m_head = std::move(oldHead->next);
        return oldHead;
    }

    std::unique_lock<std::mutex> waitForData()
    {
        std::unique_lock<std::mutex> headLock(m_headMutex);
        m_cond.wait(headLock, [&]()->bool{ return m_head.get() != getTail();});
        return headLock;
    }

    std::unique_ptr<node> waitAndPopHead(T& data)
    {
        std::unique_lock<std::mutex> headLock(waitForData());
        data = std::move(*m_head->data);
        return popHead();
    }

    std::unique_ptr<node> waitAndPopHead()
    {
        std::unique_lock<std::mutex> headLock(waitForData());
        return popHead();
    }

     std::unique_ptr<node> tryPopHead()
    {
        std::lock_guard<std::mutex> headLock(m_headMutex);
        if(m_head.get() == getTail())
        {
            return std::unique_ptr<node>();
        }
        return popHead();
    }

    std::unique_ptr<node> tryPopHead(T& data)
    {
        std::lock_guard<std::mutex> headLock(m_headMutex);
        if(m_head.get() == getTail())
        {
            return std::unique_ptr<node>();
        }
        data = std::move(*m_head->data);
        return popHead();
    }

private:
    // 队列的节点
    struct node
    {
        std::shared_ptr<T>      data;
        std::unique_ptr<node>   next;
    };

private:
    /*
    析构时，从 m_head 智能指针开始依次释放管理的内存，
    m_tail 指向的内存应该由上级节点的 next 智能指针释放。
    如果 m_tail 也使用智能指针，会导致 2 次释放同一块内存，
    所以此处 m_tail 应该使用普通指针。
    */
    std::unique_ptr<node>       m_head;         // 链表头指针
    node*                       m_tail;         // 链表尾指针
    std::mutex                  m_headMutex;    // 保护出队的互斥锁
    std::mutex                  m_tailMutex;    // 保护入队的互斥锁
    std::condition_variable     m_cond;         // 唤醒阻塞出队的条件变量
};

template <typename T>
void ThreadSafeQueue<T>::push(T data)
{
    std::shared_ptr<T> newData = std::make_shared<T>(std::move(data));
    std::unique_ptr<node> newNode(new node);

    {
        std::lock_guard<std::mutex> tailLock(m_tailMutex);
        m_tail->data = newData;
        node* newTail = newNode.get();
        m_tail->next = std::move(newNode);
        m_tail = newTail;
    }

    m_cond.notify_one();
}

template <typename T>
void ThreadSafeQueue<T>::movePush(T&& data)
{
    std::shared_ptr<T> newData = std::make_shared<T>(std::move(data));
    std::unique_ptr<node> newNode(new node);

    {
        std::lock_guard<std::mutex> tailLock(m_tailMutex);
        m_tail->data = newData;
        node* newTail = newNode.get();
        m_tail->next = std::move(newNode);
        m_tail = newTail;
    }

    m_cond.notify_one();
}

template <typename T>
void ThreadSafeQueue<T>::waitAndPop(T& data)
{
    const std::unique_ptr<node> oldHead = waitAndPopHead(data); 
}

template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::waitAndPop()
{
    const std::unique_ptr<node> oldHead = waitAndPopHead();
    return oldHead->data;
}

template <typename T>
bool ThreadSafeQueue<T>::empty()
{
    std::lock_guard<std::mutex> headLock(m_headMutex);
    return (m_head.get() == getTail());
}

template <typename T>
bool ThreadSafeQueue<T>::tryPop(T& data)
{
    const std::unique_ptr<node> oldHead = tryPopHead(data);
    return oldHead != nullptr;
}


template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::tryPop()
{
    const std::unique_ptr<node> oldHead = tryPopHead();
    return oldHead ? oldHead->data : std::shared_ptr<T>();
}

}  // namespace mrpc

#endif  // __MRPC_BASE_THREADSAFEQUEUE_H__