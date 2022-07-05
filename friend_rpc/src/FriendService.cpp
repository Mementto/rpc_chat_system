#include "../include/FriendService.hpp"
#include "../include/model/User.hpp"

uint8_t FriendsService::addFriends(int userId, int friendsId) {
    User user = m_userModel.queryById(userId);
    if (user.getId() != userId) {
        return 1;
    }
    if (!m_friendModel.insert(userId, friendsId)) {
        return 2;
    }
    return 0;

}

void FriendsService::addFriends(::google::protobuf::RpcController* controller,
                       const ::mprpc::AddFriendsRequest* request,
                       ::mprpc::AddFriendsResponse* response,
                       ::google::protobuf::Closure* done) {
    
    int userId = request->userid();
    int friendId = request->friendid();

    uint8_t res = addFriends(userId, friendId);

    mprpc::ResultCode* code = response->mutable_result();
    code->set_errcode(res);
    switch (res) {
        case 0:
            code->set_errmsg("好友添加成功！");
            break;
        case 1:
            code->set_errmsg("用户不存在！");
            break;
        case 2:
            code->set_errmsg("好友添加失败！");
            break;
        default:
            code->set_errmsg("未知错误！");
            break;
    }
    done->Run();

}

uint8_t FriendsService::queryAllFriends(int userId, std::vector<User>& users) {
    users = m_friendModel.queryAllFriendByUserId(userId);
    if (users.empty()) {
        return 1;
    } else {
        return 0;
    }
}

void FriendsService::queryAllFriends(::google::protobuf::RpcController* controller,
                    const ::mprpc::QueryAllFriendsRequest* request,
                    ::mprpc::QueryAllFriendsResponse* response,
                    ::google::protobuf::Closure* done) {
    int userId = request->userid();
    
    std::vector<User> users;
    u_int8_t res = queryAllFriends(userId, users);

    mprpc::ResultCode* code = response->mutable_result();
    code->set_errcode(res);

    if (!res) {
        for (auto& user : users) {
            mprpc::FriendInfo* friends = response->add_friends();
            friends->set_name(user.getName());
            friends->set_userid(user.getId());
            friends->set_state(user.getState());
        }
    }

    done->Run();
}
