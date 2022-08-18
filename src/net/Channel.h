#ifndef __MRPC_NET_CHANNEL_H__
#define __MRPC_NET_CHANNEL_H__

#include <memory>
#include <functional>

#include "TimeStamp.h"
#include "noncopyable.h"

namespace mrpc
{
namespace net
{

class EventLoop;
/**
 * @brief 文件描述符 Channel 类，该类并不拥有文件描述符，只是提供一系列
 *        接口，用于设置 fd 事件类型与对应事件的回调函数，内部保存的文件
 *        描述符可能是 socketfd, eventfd, timerfd, signalfd。需要注意
 *        Channel 的所有函数都是非线程安全的，它们只会在 LoopThread 调用
 */
class Channel : noncopyable
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(TimeStamp)> ReadEventCallback;

    /**
     * @brief 构造函数，创建的 Channel 对象必须且仅注册一个 EventLoop 对象，
     *        Channel 类中的函数仅能在所属 Loop 循环所在线程调用，不能在其他
     *        线程使用，并且传入的 fd 是单独创建的，其生命周期不由 Channel 类
     *        管理，而由 fd 的创建者管理
     * @param[in] loop Channel 对象注册的 EventLoop，此后 Channel 类表示的事件都会注册到该 EventLoop
     * @param[in] fd 某种类型的文件描述符（socketfd, eventfd, timerfd, signalfd）
     */
    Channel(EventLoop* loop, int fd);

    /**
     * @brief 析构函数，Channel 的生命周期不由 EventLoop 管理，而由其创建者管理
     *        在多线程情况下，必须保证在 handleEvent 时期内，Channel 对象不被析构  
     */
    ~Channel();

    /**
     * 以下四种回调都应当在 Channel Added To Loop 前完成设置
     * 当已经 Add To Loop 后，只能根据情况修改 Channel 监听的事件
     */

    /**
     * @brief 处理事件，执行相应的事件回调（not thread safe but only in loop)
     */
    void handleEvent(TimeStamp receiveTime);

    /**
     * 设置读事件发生时触发的回调任务
     */
    void setReadCallback(ReadEventCallback cb) { m_readCallbcak = std::move(cb); }

    /**
     * 设置写事件发生时触发的回调任务
     */
    void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }

     /**
     * 设置 fd 关闭时触发的回调任务
     */
    void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }

     /**
     * 设置错误发生时触发的回调任务
     */
    void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

    /**
     * @brief 引用特定的对象，确保其生命周期大于 handleEvent 的执行周期
     */
    void tie(const std::shared_ptr<void>&);

    /**
     * @brief 返回 Channel 类保存的 fd
     */
    int fd() const { return m_fd; }

    /**
     * @brief 返回在 fd 上监听的事件类型
     */
    int events() const { return m_events; }

    /**
     * @brief 设置返回事件，用于调试
     */
    void setRevents(int revt) { m_revents = revt; }

    /**
     * @brief 开启监听读事件，并在 Channel 对象所属的 EventLoop 对象中更新相关信息
     */
    void enableReading() { m_events |= kReadEvent; update(); }

    /**
     * @brief 关闭监听读事件，并在 Channel 对象所属的 EventLoop 对象中更新相关信息
     */
    void disableReading() { m_events &= ~kReadEvent; update(); }

    /**
     * @brief 关闭监听写事件，并在 Channel 对象所属的 EventLoop 对象中更新相关信息
     */
    void enableWriting() { m_events |= kWriteEvent; update(); }

    /**
     * @brief 关闭监听写事件，并在 Channel 对象所属的 EventLoop 对象中更新相关信息
     */
    void disableWriting() { m_events &= ~kWriteEvent; update(); }

    /**
     * @brief 注销全部监听事件，并在 Channel 对象所属的 EventLoop 对象中更新相关信息
     */
    void disableAll() { m_events = kNoneEvent; update(); }

    /**
     * @brief Channel 保存的 fd 是否正在监听读事件
     */
    bool isReading() const { return m_events & kReadEvent; }
    /**
     * @brief Channel 保存的 fd 是否正在监听写事件
     */
    bool isWriting() const { return m_events & kWriteEvent; }
    /**
     * @brief Channel 保存的 fd 是否正没有监听任何事件
     */
    bool isNoneEvent() const { return m_events == kNoneEvent; }

    /**
     * @brief 返回 index，index 用于表示 Poller 中 Channel 的添加状态
     */
    int index() { return m_index; }

    /**
     * @brief 设置 index，index 用于表示 Poller 中 Channel 的添加状态
     */
    void setIndex(int idx) { m_index = idx; }

    /**
     * @brief 返回字符串表示的在 fd 上监听的 events, 用于调试
     */
    std::string eventsToString() const;

    /**
     * @brief 返回字符串表示的从 poller 返回的 revents，用于调试
     */
    std::string reventsToString() const;
    
    /**
     * @brief 取消日志记录 POLLHUP 事件 
     */
    void doNotLogHup() { m_logHup = false; }

    /**
     * @brief 返回 Channel 注册的 EventLoop 对象
     */
    EventLoop* ownerLoop() { return m_loop; }

    /**
     * @brief 从所属 EventLoop 对象的 Poller 中移除 Channel 对象
     */
    void remove();

private:
    /**
     * @brief 返回字符串表示的事件
     */
    static std::string EventsToString(int fd, int ev);

    /**
     * @brief 在 Channel 注册的 EventLoop 中更新监听
     */
    void update();

    /**
     * @brief 在 tie 的保护下调用各种事件处理回调
     */
    void handleEventWithGuard(TimeStamp receiveTime);

private:
    static const int kNoneEvent;   // 无事件
    static const int kReadEvent;   // 读事件
    static const int kWriteEvent;  // 写事

    EventLoop*  m_loop;            // channel 注册的 EventLoop
    const int   m_fd;              // channel 保存的 fd (channel 不负责关闭 fd)
    int         m_events;          // channel 保存的 fd 上监听的事件 
    int         m_revents;         // loop 的 poller 返回的事件
    int         m_index;           // 用于 poller 中表示 channel 的状态
    bool        m_logHup;          // 控制 POLLHUP 事件发生时，日志是否记录
    bool        m_tied;            // 是否 tie 了某个对象
    bool        m_eventHandling;   // 是否正常处理发生的事件
    bool        m_addedToLoop;     // channel 上保存的事件是否注册到 Loop 的 Poller

    std::weak_ptr<void> m_tie;             // 用于延长特定对象的生命周期
    ReadEventCallback   m_readCallbcak;    // 读事件发生时触发的回调
    EventCallback       m_writeCallback;   // 写事件发生时触发的回调
    EventCallback       m_closeCallback;   // TCP 连接断开时触发的回调
    EventCallback       m_errorCallback;   // 错误事件发生时触发的回调
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_CHANNEL_H__