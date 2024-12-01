#pragma once
#include "poller.h"
#include "timestamp.h"
#include <vector>
#include <sys/epoll.h>
/*
epoll的使用
epoll_create
epoll_ctl add/mod/del
epoll_wait
*/
class Channel;
class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
private:
    static const int KInitEventListSize = 16;
    //填写活跃的链接
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;
    // 更新channel的通道
    void update(int operation, Channel* channel);
    using EventList = std::vector<epoll_event>;
    int epollfd_;
    EventList events_;
};