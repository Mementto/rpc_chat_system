#include "../../include/model/FriendModel.hpp"
#include "../../include/db/CommonConnectionPool.hpp"

FriendModel::FriendModel() 
: m_mysql(ConnectionPool::getConnectionPool()) {}

bool FriendModel::insert(int userId, int friendId) {
    char sql[1024] = { 0 }; 

    sprintf(sql, 
            "insert into friend(userid, friendid) values(%d, %d)", 
            userId, 
            friendId);
    
    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        return true;
    }
    return false;
}

std::vector<User> FriendModel::queryAllFriendByUserId(int userId) {
    char sql[1024] = { 0 };

    sprintf(sql, 
            "select a.id, a.name, a.state "
                "from user a inner join friend b "
                "on b.friendid = a.id where b.userid = %d", 
            userId);
    
    std::vector<User> vec;
    auto conn = m_mysql->getConnection();
    MYSQL_RES* res = conn->query(sql);
    if (res) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            User user;
            user.setId(atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
}
    return vec;
}