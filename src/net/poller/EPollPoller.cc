/**
 * @Author: yhy
 * @Date: 2022-08-16 16:52:05
 * @LastEditors: yhy
 * @LastEditTime: 2022-08-16 17:07:44
 */

#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "Logging.h"
#include "Channel.h"
#include "EPollPoller.h"

namespace mrpc
{
namespace net
{

/// On Linux, the constants of poll(2) and epoll(4) are expected to be same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace
{
/// Channel 没有被添加到 epoll 底层红黑树和 ChannelMap 或者已经被两者同时删除了
const int kNew = -1;  
/// Channel 已经被添加到 epoll 底层红黑数和 ChannelMap 中了
const int kAdded = 1;
/// Channel 从 epoll 底层红黑树中删除了，但还存在于 ChannelMap 中
const int kDeleted = 2;

const char* IndexToString(int index)
{
    switch (index)
    {
        case kNew:
        return "kNew";
        case kAdded:
        return "kAdded";
        case kDeleted:
        return "kDeleted";
        default:
        assert(false && "ERROR index");
        return "Unknown Index";
    }
}

}  // namespace


EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      m_epollFd(::epoll_create1(EPOLL_CLOEXEC)),
      m_events(kInitEventListSize)
{
    if(-1 == m_epollFd)
    {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller()
{
    ::close(m_epollFd);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_TRACE << "Channel total count = " << m_channels.size();

    int numEvents = ::epoll_wait(m_epollFd,
                                 &*m_events.begin(),
                                 static_cast<int>(m_events.size()),
                                 timeoutMs);
    /**
     * 关于 savedErrno 是否需要的讨论
     * errno 本身是线程安全的，但即使在单线程中 errno 也可能在信
     * 号的处理函数中被修改，所以此处保存一个 errno 的副本是有用的
     */
    int savedErrno = errno;

    /// epoll_wait 调用的返回时刻，用作 poll 的返回值
    TimeStamp now(TimeStamp::Now());

    if(numEvents > 0)
    {
        LOG_TRACE << numEvents << " Events happened";
        fillActiveChannels(numEvents, activeChannels);

        /// 由于 m_events 需要动态扩容，所以应当使用水平触发模式
        if(implicit_cast<size_t>(numEvents) == m_events.size())
        {
            m_events.resize(m_events.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_TRACE << "Nothing happened";
    }
    else
    {        
        /**
         * EINTR: The call was interrupted by a signal handler before either
         *        any of the requested events occurred or the timeout expired
         */
        if(savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }

    return now;
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const 
{
    assert(implicit_cast<size_t>(numEvents) <= m_events.size());
    for(int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(m_events[i].data.ptr);

#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator itr = m_channels.find(fd);
        assert(itr != m_channels.end());
        assert(itr->second == channel);
#endif 

        channel->setRevents(m_events[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel* channel)
{
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() << " events = { " 
              << channel->eventsToString() << " } index = " << IndexToString(index);

    /**
     * 在 epoll 中添加一个新的 fd，保存这个 fd 的 channel 有两种状态
     * (1) channel 没有被添加到 ChannelMap，此时需要在 map 中添加该 channel
     * (2) channel 已经添加到 ChannelMap, 此时需要检查 map 是否有该 channel
     */
    if(index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        /// 在 map 中添加该 channel
        if(index == kNew) 
        {
            assert(m_channels.find(fd) == m_channels.end());
            m_channels[fd] = channel;
        }
        /// 检查 map 是否有该 channel
        else 
        {
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel);
        }

        /// 修改 channel 的状态为 kAdded
        channel->setIndex(kAdded);

        /// 在 epollFd 中注册 channel 中保存的 fd 
        update(EPOLL_CTL_ADD, channel);
    }
    
    /// 需要更新或删除已经注册的 fd，则其 channel 一定处于 kAdded 状态
    else
    {   
        /// 安全性检查
        int fd = channel->fd();
        assert(m_channels.find(fd) != m_channels.end());
        assert(m_channels[fd] == channel);
        assert(index == kAdded);
        (void)fd;

        /**
         * 如果 channel 为 NoneEvent，则从 epollfd 中
         * 删除 fd，并将 channel 的状态设置为 kDeleted
         */
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);

            channel->setIndex(kDeleted);
        }
        /// channel 中保存有事件，则修改 epollFd 在 fd 上监听的事件
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel)
{
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "Remove channel of fd " << fd;

    /// 检查 channel 是否合法
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);

    /// 检查 channel 的事件是否注销
    assert(channel->isNoneEvent());

    /// 只有 kAdded 和 kDeleted 状态的 channel 才被添加到 ChannelMap
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);

    /// 从 ChannelMap 中删除 channel
    size_t n = m_channels.erase(fd);
    assert(n == 1);
    (void)n;

    /// 从 epollFd 中删除
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    
    /// 更新 channel 状态未为 kNew
    channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    memZero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << OperationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
    
    if(-1 == ::epoll_ctl(m_epollFd, operation, fd, &event))
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op = " << OperationToString(operation) << " fd = " << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op = " << OperationToString(operation) << " fd = " << fd;
        }
    }
}

/// For TRACE and DEBUG
const char* EPollPoller::OperationToString(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
        return "ADD";
        case EPOLL_CTL_DEL:
        return "DEL";
        case EPOLL_CTL_MOD:
        return "MOD";
        default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}

}  // namespace net
}  // namespace mrpc

