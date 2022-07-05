#include "../include/ChatService.hpp"
#include "../include/public.hpp"
#include "../include/model/User.hpp"
#include "../include/ChatException.hpp"
#include "../include/RPCHeader.pb.h"
#include "../rpc/include/MpRPCApplication.hpp"
#include "../rpc/include/MpRPCChannel.hpp"

#include <muduo/base/Logging.h>
#include <iostream>


ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    m_msgHandlerMap[LOGIN_MSG] = std::bind(&ChatService::login, this, 
                                           std::placeholders::_1, 
                                           std::placeholders::_2, 
                                           std::placeholders::_3);

    m_msgHandlerMap[LOGOUT_MSG] = std::bind(&ChatService::logout, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3);

    m_msgHandlerMap[REG_MSG] = std::bind(&ChatService::reg, this, 
                                         std::placeholders::_1, 
                                         std::placeholders::_2, 
                                         std::placeholders::_3);

    m_msgHandlerMap[ONE_CHAT_MSG] = std::bind(&ChatService::one2oneChat, this, 
                                              std::placeholders::_1, 
                                              std::placeholders::_2, 
                                              std::placeholders::_3);

    m_msgHandlerMap[ADD_FRIEND_MSG] = std::bind(&ChatService::addFrined, this, 
                                                std::placeholders::_1, 
                                                std::placeholders::_2, 
                                                std::placeholders::_3);

    m_msgHandlerMap[CREATE_GROUP_MSG] = std::bind(&ChatService::createGroup, this, 
                                                  std::placeholders::_1,
                                                  std::placeholders::_2,
                                                  std::placeholders::_3);

    m_msgHandlerMap[ADD_GROUP_MSG] = std::bind(&ChatService::addGroup, this, 
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);

    m_msgHandlerMap[GROUP_CHAT_MSG] = std::bind(&ChatService::groupChat, this, 
                                                std::placeholders::_1,
                                                std::placeholders::_2,
                                                std::placeholders::_3);

    if (m_redis.connect()) {
        m_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this,
                                              std::placeholders::_1,
                                              std::placeholders::_2));
    }
    
}

void jsonException(const TcpConnectionPtr& conn, int ackNum, Timestamp time) {

}

void userException(const TcpConnectionPtr& conn, json& js, Timestamp time) {

}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    try {
        int id = js["id"].get<int>();
        std::string pwd = js["password"];
    
        User user = m_userModel.queryById(id);
        if (user.getId() == id && user.getPwd() == pwd) {
            if (user.getState() == "online") {

                // 不可重复登录
                json res;
                res["msg_id"] = LOGIN_MSG_ACK;
                res["errno"] = 2;
                res["err_msg"] = "该账号已登录！";
                conn->send(res.dump());
            } else {

                // 登录成功
                m_userConnMap.insert(id, conn);

                m_redis.subscribe(id);

                user.setState("online");
                m_userModel.updateState(user);

                json res;
                res["msg_id"] = LOGIN_MSG_ACK;
                res["errno"] = 0;
                res["id"] = user.getId();
                res["name"] = user.getName();

                // 返回离线消息
                std::vector<std::string> vec = 
                    m_offlineMessageModel.queryById(user.getId());
                if (!vec.empty()) {
                    res["offline_msg"] = vec;
                    if(!m_offlineMessageModel.remove(user.getId())) {
                        LOG_ERROR << "offlinemessage table delete error! userid: " 
                                << user.getId();
                    }
                }

                conn->send(res.dump());
                queryAllFriends(conn, js, time);
                queryAllGroups(conn, js, time);
            }
        } else {
            json res;
            res["msg_id"] = LOGIN_MSG_ACK;
            res["errno"] = 1;
            res["err_msg"] = "用户名或者密码错误！";
            conn->send(res.dump());
        }
    } catch (json::type_error type) {
        throw ChatException(LOGIN_MSG_ACK, type);
    } catch (ChatException e) {
        throw e;
    }
}

void ChatService::logout(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int id = js["id"].get<int>();
    int key = -1;
    {
        auto it = m_userConnMap.begin();
        for (; it != m_userConnMap.end() && it->second != conn; ++it);
        if (it != m_userConnMap.end()) {
            key = it->first;
        }
    }
    if (key != -1) {
        m_userConnMap.erase(key);
        User user(id, "", "", "offline");
        m_userModel.updateState(user);
    }
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    try {
        std::string name = js["name"];
        std::string pwd = js["password"];

        User user(-1, name, pwd);

        if (m_userModel.insert(user)) {
            json res;
            res["msg_id"] = REG_MSG_ACK;
            res["errno"] = 0;
            res["id"] = user.getId();
            conn->send(res.dump());
        } else {
            json res;
            res["msg_id"] = REG_MSG_ACK;
            res["errno"] = 1;
            conn->send(res.dump());
        }
    } catch (json::type_error type) {
        throw ChatException(REG_MSG_ACK, type);
    }
}

void ChatService::one2oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    try {
        int id = js["to_id"].get<int>();

        auto toIter = m_userConnMap.find(id);
        if (toIter != m_userConnMap.end()) {
            js["msg_id"] = ONE_CHAT_MSG_ACK;
            toIter->second->send(js.dump());
            return;
        }

        User user = m_userModel.queryById(id);
        if (user.getState() == "online") {
            js["msg_id"] = ONE_CHAT_MSG_ACK;
            m_redis.publish(id, js.dump());
            return;
        }

        // 目标用户不在线，存储离线消息
        if(!m_offlineMessageModel.insert(id, js.dump())) {
            json res;
            res["msg_id"] = ONE_CHAT_MSG_ACK;
            res["errno"] = 405;
            res["err_msg"] = "消息发送失败！服务器无法离线接收";
            conn->send(res.dump());
        }
    } catch (json::type_error type) {
        throw ChatException(ONE_CHAT_MSG_ACK, type);
    }
}

void ChatService::addFrined(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::FriendsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::AddFriendsRequest request;
    try {
        int userId = js["id"].get<int>();
        int friendId = js["friend_id"].get<int>();
    
        request.set_userid(userId);
        request.set_friendid(friendId);

        mprpc::AddFriendsResponse response;
        MpRPCController controller;
        stub.addFriends(&controller, &request, &response, nullptr);

        json res;
        res["msg_id"] = ADD_FRIEND_MAG_ACK;

        if (controller.Failed()) {
            res["errno"] = 2;
            res["err_msg"] = controller.ErrorText();
        } else {
            res["err_msg"] = response.result().errmsg();
            if (0 == response.result().errcode()) {
                res["errno"] = 0;
            } else {
                res["errno"] = 2;
            }
        }

        conn->send(res.dump());
    } catch (json::type_error type) {
        throw ChatException(ADD_FRIEND_MAG_ACK, type);
    }
}

void ChatService::queryAllFriends(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::FriendsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::QueryAllFriendsRequest request;
    try {
        int userId = js["id"].get<int>();
        
        request.set_userid(userId);

        mprpc::QueryAllFriendsResponse response;
        MpRPCController controller;
        stub.queryAllFriends(&controller, &request, &response, nullptr);

        json res;
        res["id"] = userId;
        res["msg_id"] = ERGODIC_FRIENDS_MSG_ACK;
        res["err_id"] = response.result().errcode();
        if (!controller.Failed()) {
            if (response.result().errcode() == 0) {
                std::vector<std::string> ret; 
                for (auto& user : response.friends()) {
                    json js;
                    js["id"] = user.userid();
                    js["name"] = user.name();
                    js["state"] = user.state();
                    ret.push_back(js.dump());
                }
                res["friends"] = ret;
            } else {
                res["friends"] = std::vector<std::string>();
            }
        }

        conn->send(res.dump());

    } catch (json::type_error type) {
        throw ChatException(ERGODIC_FRIENDS_MSG_ACK, type);
    }
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::GroupsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::CreateGroupRequest request;
    try {
        int userId = js["id"].get<int>();
        std::string name = js["group_name"];
        std::string desc = js["group_desc"];

        request.set_userid(userId);
        request.set_name(name);
        request.set_desc(desc);

        mprpc::CreateGroupResponse response;
        MpRPCController controller;
        stub.createGroup(&controller, &request, &response, nullptr);

        json res;
        res["msg_id"] = CREATE_GROUP_MSG_ACK;
        if (controller.Failed()) {
            res["errno"] = 2;
            res["err_msg"] = controller.ErrorText();
        } else {
            res["err_msg"] = response.result().errmsg();
            if (0 == response.result().errcode()) {
                res["errno"] = 0;
            } else {
                res["errno"] = 2;
            }
        }

        conn->send(res.dump());

    } catch (json::type_error type) {
        throw ChatException(CREATE_GROUP_MSG_ACK, type);
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::GroupsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::AddGroupRequest request;
    try {
        int userId = js["id"].get<int>();
        int groupId = js["group_id"].get<int>();

        request.set_userid(userId);
        request.set_groupid(groupId);

        mprpc::AddGroupResponse response;
        MpRPCController controller;
        stub.addGroup(&controller, &request, &response, nullptr);

        json res;
        res["msg_id"] = ADD_GROUP_MSG_ACK;
        if (controller.Failed()) {
            res["errno"] = 2;
            res["err_msg"] = controller.ErrorText();
        } else {
            res["err_msg"] = response.result().errmsg();
            if (0 == response.result().errcode()) {
                res["errno"] = 0;
            } else {
                res["errno"] = 2;
            }
        }

        conn->send(res.dump());

    } catch (json::type_error type) {
        throw ChatException(ADD_GROUP_MSG_ACK, type);
    }
}

void ChatService::queryAllGroups(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::GroupsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::QueryAllGroupsRequest request;
    try {
        int userId = js["id"].get<int>();
        
        request.set_userid(userId);

        mprpc::QueryAllGroupsResponse response;
        MpRPCController controller;
        stub.queryAllGroups(&controller, &request, &response, nullptr);

        json res;
        res["id"] = userId;
        res["msg_id"] = ERGODIC_GROUPS_MSG_ACK;
        res["err_id"] = response.result().errcode();
        if (!controller.Failed()) {
            if (response.result().errcode() == 0) {
                std::vector<std::string> groups; 
                for (auto& group : response.groups()) {
                    json jsGroup;
                    jsGroup["group_id"] = group.groupid();
                    jsGroup["group_name"] = group.groupname();
                    jsGroup["group_desc"] = group.groupdesc();

                    std::vector<std::string> users;
                    for (auto& member : group.member()) {
                        json jsMem;
                        jsMem["user_id"] = member.userid();
                        jsMem["user_name"] = member.name();
                        jsMem["user_state"] = member.state();
                        jsMem["user_role"] = member.role();
                        users.push_back(jsMem.dump());
                    }
                    jsGroup["group_user_arr"] = users;
                    groups.push_back(jsGroup.dump());
                }
                res["group_arr"] = groups;
            } else {
                res["group_arr"] = std::vector<std::string>();
            }
        }

        conn->send(res.dump());

    } catch (json::type_error type) {
        throw ChatException(ERGODIC_FRIENDS_MSG_ACK, type);
    }
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    mprpc::GroupsServiceRPC_Stub stub(new MpRPCChannel());
    mprpc::GroupChatRequest request;
    try {
        int userId = js["id"].get<int>();
        int groupId = js["group_id"].get<int>();

        request.set_userid(userId);
        request.set_groupid(groupId);

        mprpc::GroupChatResponse response;
        MpRPCController controller;
        stub.groupChat(&controller, &request, &response, nullptr);

        for (int id : response.userid()) {
            auto it = m_userConnMap.find(id);
            if (it != m_userConnMap.end()) {
                js["msg_id"] = GROUP_CHAT_MSG_ACK;
                it->second->send(js.dump());
            } else {
                User user = m_userModel.queryById(id);
                if (user.getState() == "online") {
                    js["msg_id"] = GROUP_CHAT_MSG_ACK;
                    m_redis.publish(id, js.dump());
                } else {
                    m_offlineMessageModel.insert(id, js.dump());
                }
            }
        }

    } catch (json::type_error type) {
        throw ChatException(GROUP_CHAT_MSG_ACK, type);
    }
}

MsgHandler ChatService::getHandler(int msgId) {
    auto iter = m_msgHandlerMap.find(msgId);
    if (iter == m_msgHandlerMap.end()) {
        return [=](const TcpConnectionPtr&, json&, Timestamp) {
            LOG_ERROR << "msgid: " << msgId << " could not find handler!";
            };
    } else {
        return iter->second;
    }
}

void ChatService::clientExceptionQuit(const TcpConnectionPtr& conn) {
    User user;

    // 删除map表

    int key = -1;
    {
        auto it = m_userConnMap.begin();
        for (; it != m_userConnMap.end() && it->second != conn; ++it);
        if (it != m_userConnMap.end()) {
            key = it->first;
        }
    }
    if (key != -1) {
        user.setId(key);
        m_userConnMap.erase(key);
    }

    // 更新数据库
    if (user.getId() != -1) {
        user.setState("offline");
        m_userModel.updateState(user);
    }
}

void ChatService::sendExceptionInfo(const TcpConnectionPtr& conn) {
    json res;
    res["msg_id"] = EXCEPTION_ACK;
    res["errno"] = 301;
    res["err_msg"] = "数据错误！";
    conn->send(res.dump());
}

void ChatService::resetUserState() {
    m_userModel.resetAllState();
}

void ChatService::handleRedisSubscribeMessage(int userId, std::string msg) {
    auto it = m_userConnMap.find(userId);
    if (it != m_userConnMap.end()) {
        it->second->send(msg);
        return;
    }

    m_offlineMessageModel.insert(userId, msg);
}