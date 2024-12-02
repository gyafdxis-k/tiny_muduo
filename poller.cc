#include "poller.h"
#include "channel.h"

Poller::Poller(EventLoop *loop) : ownerLoop_(loop)
{
}

bool Poller::hasChannel(Channel *channel) const
{
    ChannelMap::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
