#ifndef __MRPC_NET_POLLER_H__
#define __MRPC_NET_POLLER_H__

#include <map>
#include <vector>

#include "TimeStamp.h"
#include "EventLoop.h"

namespace mrpc
{
namespace net
{

class Channel;

/**
 * @brief I/O 复用纯虚函类，该类的所有函数都是非线程安全的，仅在 LoopThread 执行
 */
class Poller : noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;
    
    /**
     * @brief 构造函数，用于子类调用
     * @brief loop Poller 所属的 EventLoop
     */
    Poller(EventLoop* loop);
    
    virtual ~Poller();

    /**
     * @brief 虚函数，poll 阻塞检查是否有活跃事件发生
     * @param[in] timeoutMs 超时时间，单位毫秒
     * @param[out] activeChannels 保存所有返回的活跃 Channels
     */
    virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    
    /**
     * @brief 虚函数，更新 Channel 上的事件
     * @param[in] channel 需要更新的 channel
     */
    virtual void updateChannel(Channel* channel) = 0;
    
    /**
     * @brief 虚函数, 移除 Channel
     * @param[in] channel 需要移除的 Channel
     */
    virtual void removeChannel(Channel* channel) = 0;

    /**
     * @brief 检查某个 Channel 是否添加到 Poller
     * @param[in] channel 需要检查的 channel
     */
    bool hasChannel(Channel* channel) const ;

    /**
     * @brief 断言函数调用是否安全
     */
    void assertInLoopThread() const
    {
        m_ownerLoop->assertInLoopThread();
    }

    /**
     * @brief 生成并返回一个Poller，默认使用 Epoll
     */
    static Poller* NewDefaultPoller(EventLoop* loop);

protected:
    /// Channel->fd() for key, Channel* for value.
    typedef std::map<int, Channel*> ChannelMap; 
    ChannelMap m_channels;  // Poller 中注册的 Channel

private:
    EventLoop* m_ownerLoop; // Poller 所属的 EventLoop
};

}  // namespace net
}  // namespace muduo

#endif // __MRPC_NET_POLLER_H__