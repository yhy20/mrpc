#include "Poller.h"
#include "Channel.h"

namespace mrpc
{
namespace net
{
Poller::Poller(EventLoop* loop)
    : m_ownerLoop(loop) { }

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const
{
    assertInLoopThread();
    ChannelMap::const_iterator itr = m_channels.find(channel->fd());
    return itr != m_channels.end() && itr->second == channel;
}

}  // namespace net
}  // namespace mrpc