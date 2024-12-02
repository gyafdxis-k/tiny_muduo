#pragma once
#include "noncopyable.h"
#include <functional>
#include "thread.h"
#include "eventloop.h"
#include <condition_variable>
#include <mutex>
#include <string>
class EventLoopThread : noncopyable
{

public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string &name = std::string(""));
    ~EventLoopThread();
    EventLoop *startLoop();

private:
    void threadFunc();
    EventLoop *loop_;
    bool existing_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};