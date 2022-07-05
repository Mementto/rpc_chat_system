#pragma once
#include "RPCHeader.pb.h"
#include "../include/model/UserModel.hpp"
#include "../include/model/GroupModel.hpp"

class GroupsService : public mprpc::GroupsServiceRPC {
public:

    uint8_t queryAllGroups(int userId, std::vector<Group>& groups);
    void queryAllGroups(::google::protobuf::RpcController* controller,
                       const ::mprpc::QueryAllGroupsRequest* request,
                       ::mprpc::QueryAllGroupsResponse* response,
                       ::google::protobuf::Closure* done);

    uint8_t createGroup(int userId, const std::string& name, const std::string& desc);
    void createGroup(::google::protobuf::RpcController* controller,
                       const ::mprpc::CreateGroupRequest* request,
                       ::mprpc::CreateGroupResponse* response,
                       ::google::protobuf::Closure* done);

    uint8_t addGroup(int userId, int groupId);
    void addGroup(::google::protobuf::RpcController* controller,
                       const ::mprpc::AddGroupRequest* request,
                       ::mprpc::AddGroupResponse* response,
                       ::google::protobuf::Closure* done);

    std::vector<int> groupChat(int userId, int groupId);
    void groupChat(::google::protobuf::RpcController* controller,
                       const ::mprpc::GroupChatRequest* request,
                       ::mprpc::GroupChatResponse* response,
                       ::google::protobuf::Closure* done);
private:
    GroupModel m_groupModel;
};