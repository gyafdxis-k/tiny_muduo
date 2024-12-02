#pragma once
#include "channel.h"
#include "noncopyable.h"
#include "timestamp.h"
#include <vector>
#include <unordered_map>
// muduo库多路事件分发器
class Channel;
class EventLoop;
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 给所有的IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activateChannels) = 0;

    virtual void updateChannel(Channel *channel);
    virtual void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel) const;

    // eventLoop 可以通过该接口获取默认的IO复用的实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义事件循环所属的EventLoop;
};