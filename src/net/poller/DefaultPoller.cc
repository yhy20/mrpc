#include <stdio.h>

#include "Poller.h"
#include "EPollPoller.h"

namespace mrpc
{
namespace net
{
    Poller* Poller::NewDefaultPoller(EventLoop* loop)
    {
        if(::getenv("MRPC_USE_POLL"))
        {
            /// TODO: PollPoller
            return new EPollPoller(loop);
        }
        else
        {
            return new EPollPoller(loop);
        }
    }
}  // namespace net
}  // namespace mrpc
