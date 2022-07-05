#pragma once
#include "json.hpp"
#include "redis/redis.hpp"
#include "model/UserModel.hpp"
#include "model/OfflineMessageModel.hpp"
#include "map/unlock_unordered_map.hpp"

#include <unordered_map>
#include <functional>
#include <mutex>
#include <muduo/net/TcpConnection.h>

using namespace muduo;
using namespace nlohmann;
using namespace muduo::net;

/**
 * @brief 处理消息事件的回调方法类型
 * 
 */
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ConnectionPool;
class ChatService {
public:
    /**
     * @brief 单例
     * 
     * @return ChatService* 
     */
    static ChatService* instance();

    /**
     * @brief 获取消息处理事件函数
     * 
     * @return MsgHandler 消息处理事件函数
     */
    MsgHandler getHandler(int);

    /**
     * @brief 处理客户端异常退出情况
     * 
     */
    void clientExceptionQuit(const TcpConnectionPtr&);

    /**
     * @brief 向客户端传回异常信息
     * 
     */
    void sendExceptionInfo(const TcpConnectionPtr&);

    /**
     * @brief 重置user状态信息
     * 
     */
    void resetUserState();

private:
    ChatService();

    /**
     * @brief json异常
     * 
     */
    void jsonException(const TcpConnectionPtr&, int, Timestamp);

    /**
     * @brief 用户操作异常
     * 
     */
    void userException(const TcpConnectionPtr&, json&, Timestamp);
    /**
     * @brief 登录
     * 
     */
    void login(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 登出
     * 
     */
    void logout(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 注册
     * 
     */
    void reg(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 一对一聊天
     * 
     */
    void one2oneChat(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 添加好友
     * 
     */
    void addFrined(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 遍历所有好友
     * 
     */
    void queryAllFriends(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 创建群组
     * 
     */
    void createGroup(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 添加群组
     * 
     */
    void addGroup(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 遍历所有群组
     * 
     */
    void queryAllGroups(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief 群聊天
     * 
     */
    void groupChat(const TcpConnectionPtr&, json&, Timestamp);

    /**
     * @brief redis订阅事件回调
     * 
     */
    void handleRedisSubscribeMessage(int userId, std::string msg);

    /**
     * @brief 存储消息Id和其对应的业务
     * 
     */
    std::unordered_map<int, MsgHandler> m_msgHandlerMap;

    /**
     * @brief 存储消息Id和其对应的异常处理操作
     * 
     */
    std::unordered_map<int, MsgHandler> m_exceptionHandlerMap;
    
    /**
     * @brief 存储用户Id和其对应的连接
     * 
     */
    mementto::unlock_unordered_map<int, TcpConnectionPtr> m_userConnMap;

    /**
     * @brief 连接的互斥锁
     * 
     */
    std::mutex m_connMutex;

    /**
     * @brief 用户Model
     * 
     */
    UserModel m_userModel;

    /**
     * @brief 离线信息存储Model
     * 
     */
    OfflineMessageModel m_offlineMessageModel;

    /**
     * @brief redis缓存对象
     * 
     */
    Redis m_redis;
};