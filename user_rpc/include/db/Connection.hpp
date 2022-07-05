#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <string>
#include <ctime>
#include <mysql/mysql.h>

class Connection {
public:
    // 初始化数据库连接
    Connection();

    // 释放数据库连接资源
    ~Connection();

    // 连接数据库
    bool connect(std::string ip, 
                 unsigned short port, 
                 std::string user, 
                 std::string password, 
                 std::string dbname);

    // 更新操作 insert、delete、update
    bool update(std::string sql);

    // 查询操作 select
    MYSQL_RES* query(std::string sql);

    // 刷新一下连接的空闲起始的时间点
    void refreshAliveTime() { m_avliveTime = clock(); }

    // 返回存活时间
    clock_t getAliveTime() { return clock() - m_avliveTime; }

    MYSQL* getConnection() { return m_conn; }
private:
    MYSQL* m_conn;
    clock_t m_avliveTime; // 记录进入空闲状态那一刻的时间
};

#endif