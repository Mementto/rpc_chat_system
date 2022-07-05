#pragma once

#include <google/protobuf/service.h>
#include <string>

class MpRPCChannel : public google::protobuf::RpcChannel {
public:
    /**
     * @brief 接受所有通过stab代理对象调用的方法，
     *        统一作RPC方法调用的数据序列化和网络发送
     * 
     * @param method 
     * @param controller 
     * @param request 
     * @param response 
     * @param done 
     */
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, 
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response, 
                    google::protobuf::Closure* done);
};