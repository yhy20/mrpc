#ifndef __MRPC_NET_EVENTLOOPTHREAD_H__
#define __MRPC_NET_EVENTLOOPTHREAD_H__

#include "Mutex.h"
#include "Thread.h"
#include "Condition.h"
#include "StringPiece.h"

namespace mrpc
{
namespace net
{

class EventLoop;

/**
 * 执行 EventLoop 事件循环的线程
 */
class EventLoopThread : noncopyable
{
public:
    typedef std::function<void(EventLoop*)> ThreadInitCallback;
    /**
     * @brief  构造函数，创建一个事件循环线程
     * @param[in] cb 线程创建后，进入 loop 循环前需要执行的用户传入的线程初始化函数
     * @param[in] name 线程名称
     */
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    StringArg name = "");

    ~EventLoopThread();

    /**
     * @brief 启动线程，仅能调用一次
     * @return 指向当前线程持有的 EventLoop 对象的指针，供其他线程在外部调用，在
     *         TcpServer 中， Acceptor 线程会使用该返回值来分发客户端的请求
     */
    EventLoop* startLoop();

private:
    /**
     * @brief 线程执行的任务
     */
    void threadTask();

private:
    EventLoop*          m_loop;     // 在当前线程执行的事件循环
    Mutex               m_mutex;    // 互斥锁
    Condition<Mutex>    m_cond;     // 条件变量
    ThreadInitCallback  m_callback; // 用户可选传入的线程创建后先执行的函数
    Thread              m_thread;   // 线程对象
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_EVENTLOOPTHREAD_H__