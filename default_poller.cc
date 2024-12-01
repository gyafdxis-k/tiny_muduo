#include "poller.h"
#include <stdlib.h>
#include "epoll_poller.h"
Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return nullptr;
    } else {
        return new EpollPoller(loop);
    }
}