#include <yaml-cpp/yaml.h>
#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

#include "public.hpp"
#include "CommonConnectionPool.hpp"

ConnectionPool* ConnectionPool::getConnectionPool() {
    static ConnectionPool pool;
    return &pool;
}

ConnectionPool::ConnectionPool() {
    if (!loadConfigFile()) {
        return;
    }

    for (int i = 0; i < m_initSize; ++i) {
        Connection* ptr = new Connection();
        ptr->connect(m_ip, m_port, m_username, m_passwd, m_db_name);
        ptr->refreshAliveTime();
        m_connectionQueue.push(ptr);
        ++m_connectionCnt; 
    }

    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

void ConnectionPool::produceConnectionTask() {
    for (;;) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        while (!m_connectionQueue.empty()) {
            m_cv.wait(lock);
        }

        // 连接数量没有到达上限
        if (m_connectionCnt < m_maxSize) {
            Connection* ptr = new Connection();
            ptr->connect(m_ip, m_port, m_username, m_passwd, m_db_name);
            ptr->refreshAliveTime();
            m_connectionQueue.push(ptr);
            ++m_connectionCnt; 
        }

        // 通知消费者线程可以连接了
        m_cv.notify_all();
    }
}

void ConnectionPool::scannerConnectionTask() {
    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(m_maxIDleTime));
        std::unique_lock<std::mutex> lock(m_queueMutex);
        while (m_connectionCnt > m_initSize) {
            Connection* p = m_connectionQueue.front();
            if (p->getAliveTime() > (m_maxIDleTime * 1000)) {
                m_connectionQueue.pop();
                --m_connectionCnt;
                delete p;
            } else {
                // 队头都没有超时，后面的肯定也没有
                break;
            }
        }
    }
}

std::shared_ptr<Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    while (m_connectionQueue.empty()) {
        if (std::cv_status::timeout == 
            m_cv.wait_for(lock, std::chrono::milliseconds(m_connectionTimeout))) {

            if (m_connectionQueue.empty()) {
                LOG("connect time out");
                return nullptr;
            }
        }
    }

    // 用户调用完智能指针后，系统将调用析构函数（以下的lambda表达式）来将连接归还连接池
    std::shared_ptr<Connection> sp(m_connectionQueue.front(), 
        [&](Connection* ptr) {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            ptr->refreshAliveTime();
            m_connectionQueue.push(ptr);
        });
    
    m_connectionQueue.pop();
    m_cv.notify_all();
    return sp;
}

const std::string ConnectionPool::GetFilePath() {
    static std::string path = "/home/mementto/cpp_proj/chat/config/mysql.yml";
    return path;
}

bool ConnectionPool::loadConfigFile() {
    YAML::Node node;
    try {
        node = YAML::LoadFile(GetFilePath());
    } catch (YAML::BadFile) {
        LOG("yaml {open file error}");
        return false;
    }
    if (node.size() == 0) {
        LOG("yaml {file content is null}");
        return false;
    }
    if (node.begin()->first.Scalar() != "mysql") {
        LOG("yaml {mysql key error}");
        return false;
    }
    auto info = node.begin()->second;
    for (auto iter = info.begin(); iter != info.end(); ++iter) {
        if (iter->first.Scalar() == "host_name") {
            m_ip = iter->second.Scalar();
        } else if (iter->first.Scalar() == "username") {
            m_username = iter->second.Scalar();
        } else if (iter->first.Scalar() == "password") {
            m_passwd = iter->second.Scalar();
        } else if (iter->first.Scalar() == "port") {
            std::string str = iter->second.Scalar();
            m_port = std::stoi(iter->second.Scalar());
        } else if (iter->first.Scalar() == "db_name") {
            m_db_name = iter->second.Scalar();
        } else if (iter->first.Scalar() == "init_size") {
            m_initSize = std::stoi(iter->second.Scalar());
        } else if (iter->first.Scalar() == "max_size") {
            m_maxSize = std::stoi(iter->second.Scalar());
        } else if (iter->first.Scalar() == "max_idle_time") {
            m_maxIDleTime = std::stoi(iter->second.Scalar());
        } else {
            m_connectionTimeout = std::stoi(iter->second.Scalar());
        }
    }
    return true;
}