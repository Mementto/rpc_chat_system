#pragma once
#include "User.hpp"

class GroupUser: public User {
public:
    GroupUser(int id = -1, 
              std::string name = "", 
              std::string state = "offline", 
              std::string role = "")
        : User(id, name, "", state)
        , m_role(role) {}

    void setRole(std::string role) { m_role = role; }
    std::string getRole() { return m_role; }
private:
    std::string m_role;
};