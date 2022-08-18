#ifndef __MRPC_NET_EVENTLOOP_H__
#define __MRPC_NET_EVENTLOOP_H__

#include <atomic>
#include <vector>
#include <functional>

#include <boost/any.hpp>

#include "Mutex.h"
#include "TimerId.h"
#include "TimeStamp.h"
#include "Callbacks.h"
#include "CurrentThread.h"

namespace mrpc
{
namespace net
{

class Poller;
class Channel;
class TimerQueue;

/**
 * @brief 事件循环，非阻塞 I/O + One Loop Per Thread 模型的基础
 */
class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;
    
    EventLoop();
    /**
     * @brief 析构函数，EventLoop 对象的析构一般发生在 quit() 调
     * 用后，需要注意的是 EventLoop 对象的函数随时会被其他线程调用，
     * 设计程序时应当保证 EventLoop 将消亡时不会被其他线程调用。解
     * 决这个问题较好的方法是使用 shared_ptr 管理 EventLoop 对象，
     * 不过 EventLoop() 析构往往意味着整个服务器进程结束。
     */
    ~EventLoop();

    /**
     * @brief 非线程安全，用于启动 loop 循环，该函数必须在
     *        EventLoop 对象的创建线程调用且仅能调用一次
     */
    void loop();

public:
    /**
     * 下述非 100% 线程安全函数在绝大多数情况下都会得到正确的结果，
     * 它们要么意味着服务器进程将结束了，要么实际上使用频率非常低，
     * 甚至不会被使用，不值得使用互斥锁划分临界区导致性能受损。
     */

    /**
     * @brief 非 100% 线程安全，该函数用于退出 loop 循环，如果创
     *        建一个局部的 EventLoop 对象且多个线程使用裸指针访问
     *        quit() 函数，则有可能访问到已经析构了的 EventLoop
     *        对象并导致错误，使用 shared_ptr 管理 EventLoop 对
     *        象可以避免该问题并保证 100% 线程安全，不过调用 quit
     *        通常意味着整个服务器进程都将结束了（实际上网络库仍然
     *        使用的裸指针访问的 EventLoop, 不过代码设计保证了程序
     *        能够正常运行）  
     */
    void quit();

    /**
     * @brief 非 100% 线程安全，返回 poll 调用返回时刻，poll 调用返
     *        回可能表示 wakeup 唤醒执行 pendingFunctors 待办任务、
     *        定时器超时执行定时任务、网络数据到达进行业务处理
     */
    TimeStamp pollReturnTime() { return m_pollReturnTime; }

    /**
     * @brief 非 100% 线程安全，返回 loop 循环迭代的次数
     */
    int64_t iteration() const { return m_iteration; }

public:
    /// 下列线程安全函数会被其他线程或用户高频调用

    /**
     * @brief 线程安全函数，用于在 loop 循环内执行某项特定的任务
     *        若在 loop 循环所在线程调用，则立即执行 callback 任务
     *        若在其他线程调用，则压入 pendingFunctors 任务队列并
     *        wakeup loop 循环等待执行
     * @param[in] cb callback 回调任务
     */
    void runInLoop(Functor cb);

    /**
     * @brief 线程安全函数，用于将特定任务压入 pendingFunctors 任务队列
     * @param[in] cb callback 回调任务
     */
    void queueInLoop(Functor cb);

    /**
     * @brief 线程安全函数，返回 pendingFunctors 队列中待执行的任务数
     */
    size_t queueSize() const;

    /**
     * @brief 线程安全函数，在指定时刻执行定时器回调任务
     * @param[in] time 定时器任务的执行时刻 
     * @param[in] cb 定时器回调任务
     */
    TimerId runAt(TimeStamp time, TimerCallback cb);

    /**
     * @brief 线程函数，在指定延迟时间片后执行定时器回调任务
     * @param[in] delay 延迟时间（单位秒）
     * @param[in] cb 定时器回调任务
     */
    TimerId runAfter(double delay, TimerCallback cb);

    /**
     * @brief 线程安全函数，每间隔一定时间执行一次定时器回调任务
     * @param[in] interval 定时器任务执行的时间间隔
     * @param[in] cb 定时器回调任务
     */
    TimerId runEvery(double interval, TimerCallback cb);

    /**
     * @brief 线程安全函数，从指定时刻开始，每间隔一定时间执行一次定时器回调任务
     * @param[in] time 定时器开始执行的时刻
     * @param[in] interval 定时器任务执行的时间间隔
     * @param[in] cb 定时器回调任务
     */
    TimerId runEveryAt(TimeStamp startTime, double interval, TimerCallback cb);

    /**
     * @brief 线程安全函数，用于取消某个定时器任务
     * @param[in] timerId 用于标识一个定时器的 Id 号
     */
    void cancel(TimerId timerId);

public:
    /// 下列函数主要用于内部使用

    /**
     * @brief 线程安全函数，用于唤醒 Loop 循环
     */
    void wakeup();

    /**
     * @brief 非线程安全，但仅在 Loop 循环所在线程执行 
     */
    void updateChannel(Channel* channel);

    /**
     * @brief 非线程安全，但仅在 Loop 循环所在线程执行 
     */
    void removeChannel(Channel* channel);
    
    /**
     * @brief 非线程安全，但仅在 Loop 循环所在线程执行
     */
    bool hasChannel(Channel* channel);

public:
    /**
     * @brief 线程安全函数，断言调用时是否在 EventLoop 的创建线程
     */
    void assertInLoopThread()
    {
        if(!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    /**
     * @brief 线程安全函数，判断当前调用线程是否是 EventLoop 创建线程
     */
    bool isInLoopThread() const { return m_threadId == CurrentThread::Tid(); }

    /**
     * @brief 该函数是否 100% 线程安全取决于 CPU 平台的 volatile bool 类型是否线程安全 
     */
    bool eventHanding() const { return m_eventHandling;}

    /**
     * @brief 非线程安全，为用户预留的冗余，用于存放用户数据
     */
    void setContext(const boost::any& context) { m_context = context; }

    /**
     * @brief 非线程安全，取出用户数据
     */
    const boost::any& getContext() const { return m_context; }

    /**
     * @brief 非线程安全，获取可修改的用户数据
     */
    boost::any* getMutableContext() { return &m_context; }

    /**
     * @brief 获取当前线程的 EventLoop，若当前线程未创建 Eventloop 则返回 nullptr
     */
    static EventLoop* GetEventLoopOfCurrentThread();

private:
    /**
     * @brief 由于在非 loop 线程调用而中断进程，使用日志 LOG_FATAL 打印出错误调用信息
     */
    void abortNotInLoopThread();
    /**
     * @brief 处理 wakeup 唤醒时向 wakeupFd 写入的数据
     */
    void handleWakeup();
    /**
     * @brief 执行待办任务队列的任务
     */
    void doPendingFunctors();
    /**
     * @brief 用于调试时打印出活跃 Channel 的相关信息
     */
    void printActiveChannels() const;

private:
    typedef std::vector<Channel*> ChannelList;

    int                 m_wakeupFd;               // 用于唤醒 Poller 的 wakeupFd
    int64_t             m_iteration;              // Loop 的迭代次数
    const pid_t         m_threadId;               // EventLoop 所属线程 Id
    TimeStamp           m_pollReturnTime;         // poll 调用的返回时刻
    mutable Mutex       m_mutex;                  // 互斥锁
    boost::any          m_context;                // 为用户预留的冗余空间
    Channel*            m_currentActiveChannel;   // 当前正在处理的活跃 Channel
    ChannelList         m_activeChannels;         // 活跃 Channel 列表

    std::atomic<bool>   m_quit;                   // 是否退出 Loop 循环
    volatile bool       m_looping;                // 是否处于 looping 状态
    volatile bool       m_eventHandling;          // 是否正在处理回调事件
    volatile bool       m_callingPendingFunctors; // 是否正在处理待执行任务

    std::unique_ptr<Poller>      m_poller;              // I/O 复用 Poller
    std::unique_ptr<TimerQueue>  m_timerQueue;          // 定时器队列
    std::unique_ptr<Channel>     m_wakeupChannel;       // 唤醒 wakeupChannel
    std::vector<Functor>         m_pendingFunctors;     // 待执行任务队列
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_EVENTLOOP_H__