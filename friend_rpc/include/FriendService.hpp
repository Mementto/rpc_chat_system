#pragma once
#include "RPCHeader.pb.h"
#include "../include/model/UserModel.hpp"
#include "../include/model/FriendModel.hpp"

class FriendsService : public mprpc::FriendsServiceRPC {
public:
    uint8_t addFriends(int userId, int friendsId);
    void addFriends(::google::protobuf::RpcController* controller,
                       const ::mprpc::AddFriendsRequest* request,
                       ::mprpc::AddFriendsResponse* response,
                       ::google::protobuf::Closure* done);

    uint8_t queryAllFriends(int userId, std::vector<User>& users);
    void queryAllFriends(::google::protobuf::RpcController* controller,
                       const ::mprpc::QueryAllFriendsRequest* request,
                       ::mprpc::QueryAllFriendsResponse* response,
                       ::google::protobuf::Closure* done);

private:
    UserModel m_userModel;
    FriendModel m_friendModel;

};