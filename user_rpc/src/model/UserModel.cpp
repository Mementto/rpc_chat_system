#include "UserModel.hpp"
#include "User.hpp"

#include "CommonConnectionPool.hpp"

#include <iostream>

UserModel::UserModel() : 
    m_mysql(ConnectionPool::getConnectionPool()) {}

bool UserModel::insert(User& user) {
    char sql[1024] = { 0 }; 

    sprintf(sql, 
        "insert into user(name, password, state) values('%s', '%s', '%s')", 
        user.getName().c_str(), 
        user.getPwd().c_str(), 
        user.getState().c_str());
    
    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        user.setId(mysql_insert_id(conn->getConnection()));
        return true;
    }
    return false;
}

User UserModel::queryById(int id) {
    char sql[1024] = { 0 };
    
    sprintf(sql, "select * from user where id = %d", id);

    auto conn = m_mysql->getConnection();
    MYSQL_RES* res = conn->query(sql);
    if (res) {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            User user(atoi(row[0]), row[1], row[2], row[3]);
            mysql_free_result(res);
            return user;
        }
    }
    
    return User();
}

bool UserModel::updateState(User& user) {
    char sql[1024] = { 0 };

    sprintf(sql, 
        "update user set state = '%s' where id = %d", 
        user.getState().c_str(), 
        user.getId());

    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        return true;
    }
    return false;
}

void UserModel::resetAllState() {
    char sql[1024] = { "update user set state = 'offline' where state = 'online'" };
    auto conn = m_mysql->getConnection();
    conn->update(sql);
}