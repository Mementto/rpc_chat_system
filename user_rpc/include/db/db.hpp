#pragma once

#include <string>
#include <mysql/mysql.h>

/**
 * @brief 数据库操作类
 * 
 */
class MySQL {
public:

    /**
     * @brief 数据库连接初始化
     * 
     */
    MySQL();
    
    /**
     * @brief 数据库连接初始化释放
     * 
     */
    ~MySQL();

    /**
     * @brief 连接数据库
     * 
     * @return true 
     * @return false 
     */
    bool connect();

    /**
     * @brief 数据库更新
     * 
     * @param sql 
     * @return true 
     * @return false 
     */
    bool update(std::string sql);

    /**
     * @brief 数据库查询
     * 
     * @param sql 
     * @return MYSQL_RES* 
     */
    MYSQL_RES* query(std::string sql);

    /**
     * @brief 获取mysql连接对象
     * 
     * @return MySQL* 
     */
    MYSQL* getConnection();

private:
    MYSQL *m_conn;
};