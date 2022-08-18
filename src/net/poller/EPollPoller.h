#ifndef __MRPC_NET_POLLER_EPOLLPOLLER_H
#define __MRPC_NET_POLLER_EPOLLPOLLER_H

#include <vector>
#include "Poller.h"

struct epoll_event;

namespace mrpc
{
namespace net
{
/**
 * @brief 基于 epoll(2) 实现的 I/O 复用类，该类所有的函数都
 *        是非线程安全的，但这些函数都只会在 LoopThread 调用
 */
class EPollPoller : public Poller
{
public:
    /**
     * @brief 构造函数，为特定的 EventLoop 创建一个 EPollPoller
     * @param[in] loop 调用 assertInLoopThread() 验证 EPollPoller 的函数是否安全使用
     */
    EPollPoller(EventLoop* loop);

    ~EPollPoller() override;
 
    /**
     * @brief I/O 复用 poll 虚函数，返回监听到的活跃 Channels
     * @param[in] timeoutMs 最大阻塞时间，单位毫秒
     * @param[out] activeChannels 保存所有返回的活跃 Channels
     */
    TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;

    /**
     * @brief 虚函数，更新 Channel 上的事件
     * @param[in] channel 需要更新的 channel
     */
    void updateChannel(Channel* channel) override;

     /**
     * @brief 虚函数, 移除 Channel
     * @param[in] channel 需要移除的 Channel
     */
    void removeChannel(Channel* channel) override;

private:
    /**
     * 初始的 EventList 大小，随着服务器负载压力增大，EventList 的容量大小
     * 也会随之扩大，这也是 epollfd 监听的 fd 使用水平触发的原因。当同时活
     * 跃的事件数超过 EventList 的容量时，会放弃处理多出的活跃事件并两倍扩
     * 大 EventList 的容量，在下一次 epoll 返回时处理之前未处理的活跃事件
     */
    static const int kInitEventListSize = 16;   

    /**
     * @brief 返回字符串表示的 epoll_ctl 的 operation，用于调试
     * @param[in] op EPOLL_CTL_ADD、EPOLL_CTL_MOD 或 EPOLL_CTL_DEL
     */
    static const char* OperationToString(int op);

    /**
     * @brief 用 poll 返回的 EventList 填充 activeChannels
     * @param[in] numEvents 活跃事件数目
     * @param[out] activeChannels 保存活跃的 activeChannels
     */
    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;

    /**
     * @brief 执行 epoll_ctl 函数更新 epollFd
     * @param[in] operation EPOLL_CTL_ADD、EPOLL_CTL_MOD 或 EPOLL_CTL_DEL
     * @param[in] channel 需要更新的 channel
     */
    void update(int operation, Channel* channel);

private:
    typedef std::vector<struct epoll_event> EventList;

    int         m_epollFd;  // epoll I/O 复用使用的 epollfd
    EventList   m_events;   // 用于保存 epoll 返回的所有活跃事件
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_POLLER_EPOLLPOLLER_H