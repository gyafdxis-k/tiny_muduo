#pragma once
#include "noncopyable.h"
#include "timestamp.h"
#include <functional>
#include <memory>
// Channel 理解为通道，封装了sockfd和感兴趣的event 比如EPOLLIN EPOLLOUT等事件
class EventLoop;
class Timestamp;
class Channel : noncopyable {
public:
    using EventCallback = std::function<void()> ;
    using ReadEventCallback = std::function<void(Timestamp)>;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    // fd得到poller通知以后，处理事件的，调用事件的
    void handleEvent(Timestamp receiveTime);
    void setReadCallback(ReadEventCallback cb) {readEventCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback wb) { writeCallback_ = std::move(wb);}
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb);}

    // 防止channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);
    int fd() const { return fd_; }
    int events() const { return events_;}
    void set_revents(int revt) { revents_ = revt;}

    void enableReading() { events_ |= kReadEvent; update();}
    void disableReading() { events_ &= ~kReadEvent; update();}
    void enableWriting() { events_ |= KWriteEvent; update();}
    void disableWriting() { events_ &= ~KWriteEvent; update();}
    void disableAll() { events_ = kNoneEvent; update();}

    bool isNoneEvent() const { return events_ == kNoneEvent;}
    bool isWriting() const { return events_ & KWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
    EventLoop* ownerLoop() { return loop_;}
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int KWriteEvent;

    EventLoop *loop_; //事件循环
    const int fd_; // poller 监听的对象
    int events_; //注册感兴趣的事件
    int revents_; // poller具体发生的事件
    int index_;
    std::weak_ptr<void> tie_;
    bool tied_;
    // 因为channel通道中能获取fd 最终发生的具体的事件events，所以它负责调用具体事件的回调
    ReadEventCallback readEventCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

};