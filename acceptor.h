#pragma once
#include "noncopyable.h"
#include "channel.h"
#include "socket.h"
#include <functional>
class EventLoop;
class InetAddress;
class Acceptor : noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }
    bool listenning() const { return listening_; }
    void listen();

private:
    void handleRead();
    EventLoop *loop_; // acceptor用的就是用户定义的那个baseloop，也称为mainLoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};