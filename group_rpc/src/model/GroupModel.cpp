#include "GroupModel.hpp"

#include "CommonConnectionPool.hpp"

GroupModel::GroupModel() 
    : m_mysql(ConnectionPool::getConnectionPool()) {}

bool GroupModel::createGroup(Group& group) {
    char sql[1024] = {0};
    sprintf(sql, 
            "insert into allgroup(groupname, groupdesc) values('%s', '%s')", 
            group.getName().c_str(), 
            group.getDesc().c_str());                                                                                                                                                                                                                                    

   auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        group.setId(mysql_insert_id(conn->getConnection()));
        return true;
    }
    return false;
}

bool GroupModel::addGroup(int userId, int groupId, std::string role) {
    char sql[1024] = {0};
    sprintf(sql, 
            "insert into groupuser values(%d, %d, '%s')", 
            groupId, 
            userId, 
            role.c_str());
    auto conn = m_mysql->getConnection();
    if (conn->update(sql)) {
        return true;
    }
    return false;    
}

std::vector<Group> GroupModel::queryGroupsByUserId(int userId) {
    char sql[1024] = {0};
    sprintf(sql,
            "select a.id, a.groupname, a.groupdesc \
                from allgroup a inner join groupuser b on a.id = b.groupid \
                where userid = %d",
            userId);
    std::vector<Group> groupVec;
    auto conn = m_mysql->getConnection();
    MYSQL_RES* res = conn->query(sql);
    if (res != nullptr) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            Group group(atoi(row[0]), row[1], row[2]);
            groupVec.push_back(group);
        }
        mysql_free_result(res);
    }

    for (auto& group : groupVec) {
        sprintf(sql, 
                "select a.id, a.name, a.state, b.grouprole \
                    from user a inner join groupuser b on b.userid = a.id \
                    where b.groupid = %d", 
                group.getId());
        auto conn = m_mysql->getConnection();
        MYSQL_RES* res = conn->query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser user(atoi(row[0]), row[1], row[2], row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupVec;
}

std::vector<int> GroupModel::queryGroupUsers(int userId, int groupId) {
    char sql[1024] = {0};
    sprintf(sql,
            "select userid from groupuser where groupid = %d and userid != %d", 
            groupId, 
            userId);
    std::vector<int> ret;
    auto conn = m_mysql->getConnection();
    MYSQL_RES* res = conn->query(sql);
    if (res != nullptr) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr) {
            ret.push_back(atoi(row[0]));
        }
        mysql_free_result(res);
    }
    return ret;
}