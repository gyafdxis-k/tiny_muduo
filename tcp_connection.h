#pragma once
#include "noncopyable.h"
#include "inet_address.h"
#include "callbacks.h"
#include "buffer.h"
#include <memory>
#include <string>
#include <atomic>
class Channel;
class EventLoop;
class Socket;

class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection> {

public:
    TcpConnection(EventLoop* loop, 
                const std::string &name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_;}
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    //发送数据
    void send(std::string buf);
    // 关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
        highWaterMarkCallback_ = cb;
    }
    void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }

    void connectEstablished();
    void connectDestoryed();

    enum StateE {
        KDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };
    void setState(StateE);

private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop(); 

    EventLoop *loop_; // 这里绝对不是baseLoop，因为TcpConnection 都是在subLoop中
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t hightWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

};