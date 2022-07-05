#include "../include/GroupService.hpp"
#include "../include/model/User.hpp"

uint8_t GroupsService::queryAllGroups(int userId, std::vector<Group>& groups) {
    groups = m_groupModel.queryGroupsByUserId(userId);
    if (groups.empty()) {
        return 1;
    } else {
        return 0;
    }
}

void GroupsService::queryAllGroups(::google::protobuf::RpcController* controller,
                    const ::mprpc::QueryAllGroupsRequest* request,
                    ::mprpc::QueryAllGroupsResponse* response,
                    ::google::protobuf::Closure* done) {

    int userId = request->userid();

    std::vector<Group> groups;
    u_int8_t res = queryAllGroups(userId, groups);

    mprpc::ResultCode* code = response->mutable_result();
    code->set_errcode(res);

    if (!res) {
        for (auto& group : groups) {
            mprpc::GroupInfo* gs = response->add_groups();
            gs->set_groupid(group.getId());
            gs->set_groupname(group.getName());
            gs->set_groupdesc(group.getDesc());
            for (auto& member : group.getUsers()) {
                mprpc::GroupMember* gm = gs->add_member();
                gm->set_userid(member.getId());
                gm->set_name(member.getName());
                gm->set_role(member.getRole());
                gm->set_state(member.getState());
            }
        }
    }

    done->Run();
}

uint8_t GroupsService::createGroup(int userId, const std::string& name, const std::string& desc) {
    Group group(-1, name, desc);
    if (m_groupModel.createGroup(group)) {
        m_groupModel.addGroup(userId, group.getId(), "creator");
        return 0;
    } else {
        return 1;
    }
}

void GroupsService::createGroup(::google::protobuf::RpcController* controller,
                    const ::mprpc::CreateGroupRequest* request,
                    ::mprpc::CreateGroupResponse* response,
                    ::google::protobuf::Closure* done) {
    
    int userId = request->userid();
    const std::string& name = request->name();
    const std::string& desc = request->desc();

    uint8_t res = createGroup(userId, name, desc);

    mprpc::ResultCode* code = response->mutable_result();
    code->set_errcode(res);
    if (res) {
        code->set_errmsg("创建群组失败！");
    } else {
        code->set_errmsg("创建群组成功！");
    }
    done->Run();
}

uint8_t GroupsService::addGroup(int userId, int groupId) {
    if (!m_groupModel.addGroup(userId, groupId, "normal")) {
        return 1;
    } else {
        return 0;
    }
}

void GroupsService::addGroup(::google::protobuf::RpcController* controller,
                    const ::mprpc::AddGroupRequest* request,
                    ::mprpc::AddGroupResponse* response,
                    ::google::protobuf::Closure* done) {

    int userId = request->userid();
    int groupId = request->groupid();

    uint8_t res = addGroup(userId, groupId);

    mprpc::ResultCode* code = response->mutable_result();
    code->set_errcode(res);
    if (res) {
        code->set_errmsg("加入群组失败！");
    } else {
        code->set_errmsg("加入群组成功！");
    }
    done->Run();
}

std::vector<int> GroupsService::groupChat(int userId, int groupId) {
    return m_groupModel.queryGroupUsers(userId, groupId);
}

void GroupsService::groupChat(::google::protobuf::RpcController* controller,
                    const ::mprpc::GroupChatRequest* request,
                    ::mprpc::GroupChatResponse* response,
                    ::google::protobuf::Closure* done) {
    
    int userId = request->userid();
    int groupId = request->groupid();

    std::vector<int> groupUserIds = groupChat(userId, groupId);

    for (auto id : groupUserIds) {
        response->add_userid(id);
    }

    done->Run();

}