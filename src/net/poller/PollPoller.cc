#include <poll.h>
#include <errno.h>
#include <assert.h>

#include "Types.h"
#include "Logging.h"
#include "Channel.h"
#include "PollPoller.h"

namespace mrpc
{
namespace net
{

PollPoller::PollPoller(EventLoop* loop)
    : Poller(loop) { }

PollPoller::~PollPoller() = default;

TimeStamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*m_pollFds.begin(), m_pollFds.size(), timeoutMs);
    int savedErrno = errno;
    TimeStamp now(TimeStamp::Now());
    if(numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happened";

    }
    else if(numEvents == 0)
    {
        LOG_TRACE << " Nothing happened";
    }
    else
    {
        if(savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << " PollPoller::poll()";
        }
    }
    return now;
}

void PollPoller::updateChannel(Channel* channel)
{

}

void removeChannel(Channel* channel)
{
    
}

void PollPoller::fillActiveChannels(int numEvents,
                                    ChannelList* activeChannels) const
{

}

}  // namespace net
}  // namespace mrpc

