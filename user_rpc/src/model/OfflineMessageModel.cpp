#include "OfflineMessageModel.hpp"

#include "CommonConnectionPool.hpp"

OfflineMessageModel::OfflineMessageModel() 
    : m_mysql(ConnectionPool::getConnectionPool()) {}

bool OfflineMessageModel::insert(int id, const std::string& msg) {
    char sql[1024] = { 0 }; 

    sprintf(sql, 
        "insert into offlinemessage values(%d, '%s')", 
        id, 
        msg.c_str());
    
    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        return true;
    }
    return false;
}

bool OfflineMessageModel::remove(int id) {
    char sql[1024] = { 0 }; 

    sprintf(sql, "delete from offlinemessage where userid=%d", id);
    
    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        return true;
    }
    return false;
}

std::vector<std::string> OfflineMessageModel::queryById(int id) {
    char sql[1024] = { 0 };
    
    sprintf(sql, "select message from offlinemessage where userid = %d", id);

    auto conn = m_mysql->getConnection();
    std::vector<std::string> ret;
    MYSQL_RES* res = conn->query(sql);
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            ret.push_back(row[0]);
        }
        mysql_free_result(res);
    }
    return ret;
}