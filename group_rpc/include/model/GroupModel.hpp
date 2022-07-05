#pragma once
#include "Group.hpp"

#include <string>
#include <vector>

class ConnectionPool;
class GroupModel {
public:
    GroupModel();

    /**
     * @brief 创建群组
     * 
     * @param group 群对象
     * @return true 
     * @return false 
     */
    bool createGroup(Group& group);

    /**
     * @brief 加群
     * 
     * @param userId 要加入的用户
     * @param groupId 群编号
     * @param role 用户的角色
     * @return true 
     * @return false 
     */
    bool addGroup(int userId, int groupId, std::string role);

    /**
     * @brief 查询该用户的所有加入的群
     * 
     * @param userId 该用户编号
     * @return std::vector<Group> 
     */
    std::vector<Group> queryGroupsByUserId(int userId);

    /**
     * @brief 查询该用户该群下的所有用户
     * 
     * @param userId 用户Id
     * @param groupId 群Id
     * @return std::vector<int> 
     */
    std::vector<int> queryGroupUsers(int userId, int groupId);
private:
    ConnectionPool* m_mysql;
};