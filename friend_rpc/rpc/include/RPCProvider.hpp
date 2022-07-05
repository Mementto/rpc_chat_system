#pragma once

#include "google/protobuf/service.h"

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>

#include <google/protobuf/descriptor.h>
#include <unordered_map>

struct ServiceInfo {
private:
    using _MethodMap = std::unordered_map<std::string, const google::protobuf::MethodDescriptor*>;
public:
    google::protobuf::Service* m_service;   // 服务对象指针
    _MethodMap                 m_methodMap; // 服务对象方法map
};

class RPCProvider {
public:
    void NotifyService(google::protobuf::Service* service);
    void run();

private:
    muduo::net::EventLoop m_eventLoop;

    // 服务对象map
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;


private:
    void onConnection(const muduo::net::TcpConnectionPtr&);
    void onMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    void onRPCCallback(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};