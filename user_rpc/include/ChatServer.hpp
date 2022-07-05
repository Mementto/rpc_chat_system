#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class UserModel;
class ChatServer {
public:
    ChatServer(EventLoop* loop, 
               InetAddress& listenAddr, 
               const std::string& nameArg);

    void start();

private:

    /**
     * @brief 连接回调
     * 
     */
    void onConnection(const TcpConnectionPtr&);

    /**
     * @brief 读写回调
     * 
     */
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp);

    /**
     * @brief muduo入口
     * 
     */
    TcpServer m_server;

    /**
     * @brief 主事件循环
     * 
     */
    EventLoop* m_loop;
};