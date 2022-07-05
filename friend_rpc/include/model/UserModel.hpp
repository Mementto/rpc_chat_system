#pragma once

class User;

/**
 * @brief User表的操作类
 * 
 */
class ConnectionPool;
class UserModel {
public:
    UserModel();

    /**
     * @brief User表的增
     * 
     * @return true 
     * @return false 
     */
    bool insert(User&);

    /**
     * @brief 根据Id查询
     * 
     * @return User 
     */
    User queryById(int);

    /**
     * @brief 更新User的state
     * 
     * @return true 
     * @return false 
     */
    bool updateState(User&);

    /**
     * @brief 重置user状态信息
     * 
     */
    void resetAllState();
private:
    ConnectionPool* m_mysql;
};