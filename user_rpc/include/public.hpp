#pragma once

#define LOG(str) \
    std::cout << __FILE__ << ":" <<  __LINE__ << \
    " " << __TIMESTAMP__ << " : " << str << std::endl;
    
enum EnMsgType {
    EXCEPTION_ACK = 0,         // 错误应答
    LOGIN_MSG = 1,             // 登录请求
    LOGIN_MSG_ACK = 2,         // 登录应答
    REG_MSG = 3,               // 注册请求
    REG_MSG_ACK = 4,           // 注册应答
    ONE_CHAT_MSG = 5,          // 一对一聊天请求
    ONE_CHAT_MSG_ACK = 6,      // 一对一聊天应答
    GROUP_CHAT_MSG = 7,        // 群聊请求
    GROUP_CHAT_MSG_ACK = 8,    // 群聊应答
    ADD_FRIEND_MSG = 9,        // 添加好友请求
    ADD_FRIEND_MAG_ACK = 10,   // 添加好友应答
    CREATE_GROUP_MSG = 11,     // 创建群组请求
    CREATE_GROUP_MSG_ACK = 12, // 创建群组应答
    ADD_GROUP_MSG = 13,        // 加入群组请求
    ADD_GROUP_MSG_ACK = 14,    // 加入群组应答

    ERGODIC_FRIENDS_MSG = 15,     // 遍历好友请求
    ERGODIC_FRIENDS_MSG_ACK = 16, // 遍历好友应答
    ERGODIC_GROUPS_MSG = 17,      // 遍历群组请求
    ERGODIC_GROUPS_MSG_ACK = 18,  // 遍历群组应答

    LOGOUT_MSG = 20,               // 注销请求
    LOGOUT_MSG_ACK = 21            // 注销请求
};
