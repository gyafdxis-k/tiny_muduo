#pragma once
#include <functional>
#include <vector>
#include "noncopyable.h"
#include <atomic>
#include <unistd.h>
#include <memory>
#include "timestamp.h"
#include <mutex>
#include "current_thread.h"
class Channel;
class Poller;
// 包括两大模块 channel pool
class EventLoop : noncopyable {
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 开启事件循环
    void loop();
    // 退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    // 在当前Loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放到队列中，唤醒loop所在线程，执行cb
    void queueInLoop(Functor cb);

    // 唤醒loop所在线程
    void wakeup();

    // eventloop 方法 poller方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    bool hasChannel(Channel* channel);
    bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }

private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;
    std::atomic_bool quit_;
    
    const pid_t threadId_;

    Timestamp pollReturnTime_; // poller返回发生事件的channel的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_; // 当main loop 获取一个新用户的channel，通过轮训算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;


    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_; //标志当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; // 存储loop需要执行的所有回调操作
    std::mutex mutex_; //保护上面vector的容器的线程安全操作
};