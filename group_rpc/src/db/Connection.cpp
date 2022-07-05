#include "Connection.hpp"
#include "public.hpp"
#include <iostream>

Connection::Connection() {
    m_conn = mysql_init(nullptr);
}

Connection::~Connection() {
    if (m_conn != nullptr)
    mysql_close(m_conn);
}

bool Connection::connect(std::string ip, 
                         unsigned short port, 
                         std::string user, 
                         std::string password, 
                         std::string dbname) {

    MYSQL *p = mysql_real_connect(m_conn, ip.c_str(), user.c_str(), 
                                  password.c_str(), dbname.c_str(), 
                                  port, nullptr, 0);
    return p != nullptr;
}

bool Connection::update(std::string sql) {
    if (mysql_query(m_conn, sql.c_str())) {
        LOG("更新失败:" + sql);
        return false;
    }
    return true;
}

MYSQL_RES* Connection::query(std::string sql) {
    if (mysql_query(m_conn, sql.c_str())) {
        LOG("查询失败:" + sql);
        return nullptr;
    }
    return mysql_use_result(m_conn);
}

