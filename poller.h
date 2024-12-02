#pragma once
#include "noncopyable.h"
#include "timestamp.h"
#include <vector>
#include <unordered_map>
// muduo库多路事件分发器
class Channel;
class EventLoop;
#define TINY_MUDUO_EXPORT __attribute__((visibility("default")))
class TINY_MUDUO_EXPORT Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    // 给所有的IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activateChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    bool hasChannel(Channel *channel) const;

    // eventLoop 可以通过该接口获取默认的IO复用的实现
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_; // 定义事件循环所属的EventLoop;
};