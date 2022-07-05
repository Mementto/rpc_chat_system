#pragma once

#include <string>

/**
 * @brief User表的ORM类
 * 
 */
class User {
public:

    User(int id = -1, 
         std::string name = "", 
         std::string pwd = "", 
         std::string state = "offline")
        : m_id(id)
        , m_name(name)
        , m_password(pwd)
        , m_state(state) {}

    void setId(int id) { m_id = id; }
    void setName(std::string name) { m_name = name; }
    void setPwd(std::string pwd) { m_password = pwd; }
    void setState(std::string state) { m_state = state; }

    int getId() { return m_id; }
    std::string getName() { return m_name; }
    std::string getPwd() { return m_password; }
    std::string getState() { return m_state; }

private:
    int m_id;
    std::string m_name;
    std::string m_password;
    std::string m_state;

};