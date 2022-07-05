#ifndef __COMMON_CONNECTION_POOL_H__
#define __COMMON_CONNECTION_POOL_H__
#include <memory>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "Connection.hpp"

class ConnectionPool {
public:
    static ConnectionPool* getConnectionPool();
    std::shared_ptr<Connection> getConnection();

private:
    ConnectionPool();                        // 单例1 构造函数私有化
    const std::string GetFilePath();
    bool loadConfigFile();                   // 从配置文件中加载配置项

    //运行在独立的线程中，专门负责生产新的链接
    void produceConnectionTask();

    //运行在独立的线程中，专门扫描队列里的线程空闲时间
    void scannerConnectionTask();

private:
    std::string m_ip;                        // ip
    unsigned short m_port;                   // 端口号
    std::string m_username;                  // 用户名
    std::string m_passwd;                    // 密码
    std::string m_db_name;                   // 数据库名称
    int m_initSize;                          // 连接池初始连接量
    int m_maxSize;                           // 连接池最大连接量
    int m_maxIDleTime;                       // 连接池最大空闲时间
    int m_connectionTimeout;                 // 连接池获取连接超时时间
    std::atomic_int m_connectionCnt;         // 连接池现有连接数量
    std::mutex m_queueMutex;                 // 维护连接队列的线程安全互斥锁
    std::condition_variable m_cv;            // 条件变量，用于线程之间通信

    std::queue<Connection*> m_connectionQueue; // 存储Mysql连接的队列

};

#endif