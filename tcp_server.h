#pragma once
#include "noncopyable.h"
#include "eventloop.h"
#include "acceptor.h"
#include "inet_address.h"
#include "eventloop_threadpool.h"
#include "callbacks.h"
#include <functional>
#include <string>
#include <atomic>
#include <unordered_map>
class TcpServer : noncopyable
{

public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option
    {
        KNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name, Option option = KNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);

    void start();

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *loop_; // the accept loop

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_; // main loop

    std::shared_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_;       // 新连接时候回调
    MessageCallback messageCallback_;             // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息写完后的回调
    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};