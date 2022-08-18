/*
 * @Author: yhy
 * @Date: 2022-08-03 13:56:48
 * @LastEditors: yhy
 * @LastEditTime: 2022-08-03 14:02:54
 * @Description: 
 */
#ifndef __MRPC_NET_POLLER_POLLPOLLER_H__
#define __MRPC_NET_POLLER_POLLPOLLER_H__

#include <vector>

#include "Poller.h"

struct pollfd;

namespace mrpc
{
namespace net
{

/**
 * @brief 基于 poll(2) 实现的 I/O 复用类 
 */
class PollPoller : public Poller
{
public:
    PollPoller(EventLoop* loop);
    ~PollPoller() override;

    TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
    
private:
    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;

private:
    typedef std::vector<struct pollfd> PollFdList;
    PollFdList m_pollFds;
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_POLLER_POLLPOLLER_H__