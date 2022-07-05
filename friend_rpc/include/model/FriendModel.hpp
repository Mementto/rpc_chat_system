#pragma once
#include "User.hpp"

#include <vector>

class ConnectionPool;
class FriendModel {
public:

    FriendModel();

    /**
     * @brief 绑定用户Id和好友Id
     * 
     * @param userId 用户Id
     * @param friendId 好友Id
     */
    bool insert(int userId, int friendId);

    /**
     * @brief 根据用户Id遍历全部的好友信息
     * 
     * @param userId 用户Id
     */
    std::vector<User> queryAllFriendByUserId(int userId);
private:
    ConnectionPool* m_mysql;
};