#include "tcp_connection.h"
#include "channel.h"
#include "socket.h"
#include "eventloop.h"
#include "logger.h"
#include <errno.h>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <string>

EventLoop *CheckSubLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d subloop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}
TcpConnection::TcpConnection(EventLoop *loop, const std::string &name, int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
    : loop_(CheckSubLoopNotNull(loop))
    , name_(name)
    , state_(kConnected)
    , reading_(true)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(peerAddr)
    , hightWaterMark_(64*1024*1024) // 64M
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection:dtor[%s] at fd=%d state=%d \n", name_.c_str(), channel_->fd(), (int)state_);
}

void TcpConnection::send(std::string buf)
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
            
        } else {
            loop_->runInLoop(
                std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size())
            );
        }
    }
}
void TcpConnection::sendInLoop(const void *data, size_t len)
{
     ssize_t nwrote = 0;
     size_t remaining = len;
     bool faultError = false;
     if (state_ == KDisconnected) {
        LOG_ERROR("disconnected, give up writing");
        return;
     }
     if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(
                    bind(writeCompleteCallback_, shared_from_this()));
            }
        } else {
            int nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) { // SIGPIPE RESET
                    faultError = true;
                }
            }
        }
     }

     // 说明当前这一次write，并没有把数据全部发送出去，剩余的数据需要保存到缓冲区当中，然后给channel
     // 注册epollout事件，poller发现tp的发送缓冲区有空间，会通知相应的sock->channel，调用writeCallback_
     // 调用T产品Connection::handleWrite方法，把发送缓冲区的数据全部发送完成
     if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= hightWaterMark_ && oldLen < hightWaterMark_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append((char*)data + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting(); //这里一定要注册channel的写事件，否则事件poller不会处理写事件
        }
     }

}

void TcpConnection::shutdown()
{
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting()) { // output buf已经写完
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading(); //向poller注册channel的epollin事件
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestoryed()
{
    if (state_ == kConnected) {
        setState(KDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove(); // channel从poller中删除掉
    
}

void TcpConnection::setState(StateE state)
{
    state_ = state;
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n > 0) {
        // 已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) {
        int savedErrno = 0;
        size_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            channel_->disableWriting();
            if (writeCompleteCallback_) {
                // 唤醒loop_对应的thread线程，执行回调
                loop_->queueInLoop(bind(
                    writeCompleteCallback_, shared_from_this()
                ));
            }
            if (state_ == kDisconnecting) {
                shutdownInLoop();
            }
        } else {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    } else {
        LOG_ERROR("TcpConnection fd=%d is down, no more write \n", channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(KDisconnected);
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);
}




