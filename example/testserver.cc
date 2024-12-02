#include <tiny_muduo/tcp_server.h>
#include <tiny_muduo/logger.h>
#include <functional>
class EchoServer {
public:
    EchoServer(EventLoop* loop,
            const InetAddress &addr,
            const std::string &name)
        : server_(loop, addr, name)
        , loop_(loop)
        {
            server_.setConnectionCallback(
                std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
            );
            server_.setMessageCallback(
                std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
            );
            server_.setThreadNum(3);
        }
        void start() {
            server_.start();
        }
private:
    EventLoop *loop_;
    TcpServer server_;
    // 连接建立或者断开的回调
    void onConnection(const TcpConnectionPtr &conn) {
        if (conn->connected()) {
            LOG_INFO("Connection up:%s", conn->peerAddress().toIpPort().c_str());
        } else {
            LOG_INFO("Connection down:%s", conn->peerAddress().toIpPort().c_str());
        }
    }
    // 
    void onMessage(const TcpConnectionPtr &conn,
    Buffer *buff, Timestamp Timestamp) {
        std::string msg = buff->retrievelAllAsString();
        conn->send(msg);
        conn->shutdown();

    }

};
int main() {
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "EchoServer-01");
    server.start();
    loop.loop();
    return 0;
}