#pragma once

#include <string>
#include <vector>

/**
 * @brief offlinemessage表的操作类
 * 
 */
class ConnectionPool;
class OfflineMessageModel {
public:
    OfflineMessageModel();

    /**
     * @brief 增
     * 
     * @return true 
     * @return false 
     */
    bool insert(int, const std::string&);

    /**
     * @brief 根据id删
     * 
     * @return true 
     * @return false 
     */
    bool remove(int);

    /**
     * @brief 根据Id查
     * 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> queryById(int);
private:
    ConnectionPool* m_mysql;
};