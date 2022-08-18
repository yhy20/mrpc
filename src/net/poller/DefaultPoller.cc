/*
 * @Author: yhy
 * @Date: 2022-08-02 20:54:06
 * @LastEditors: yhy
 * @LastEditTime: 2022-08-02 20:55:15
 * @Description: 
 */
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
