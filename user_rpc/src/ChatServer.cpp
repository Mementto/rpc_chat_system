#include "ChatServer.hpp"
#include "ChatService.hpp"
#include "ChatException.hpp"
#include "json.hpp"

#include <iostream>
#include <functional>
#include <muduo/base/Logging.h>

using namespace nlohmann;

ChatServer::ChatServer(EventLoop* loop, 
           InetAddress& listenAddr, 
           const std::string& nameArg)
    : m_server(loop, listenAddr, nameArg)
    , m_loop(loop) {

    m_server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, 
                  std::placeholders::_1));

    m_server.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, 
                  std::placeholders::_1, 
                  std::placeholders::_2, 
                  std::placeholders::_3));

    m_server.setThreadNum(6);
}

void ChatServer::start() {
    m_server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    if (!conn->connected()) {

        ChatService::instance()->clientExceptionQuit(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, 
    Buffer* buffer, Timestamp times) {
    
    std::string buf = buffer->retrieveAllAsString();

    auto service = ChatService::instance();
    json js;
    try {
        //string 2 json
        js = json::parse(buf);

        //根据msgId从service中获取指定事件处理函数并执行
        auto handler = service->getHandler(js["msg_id"].get<int>());
        handler(conn, js, times);
    } catch (json::parse_error parse) {
        LOG_ERROR << parse.what();
    } catch (json::type_error type) {
        LOG_ERROR << type.what();
    } catch (json::other_error other) {
        LOG_ERROR << other.what();
    } catch (ChatException chatException) {
        LOG_ERROR << chatException.getAck() << "->" << chatException.getException().what();
    }
}