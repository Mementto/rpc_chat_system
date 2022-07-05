#pragma once

#include <google/protobuf/service.h>
#include <string>

class MpRPCController : public google::protobuf::RpcController {
public:
    MpRPCController();
    void Reset();
    bool Failed() const;
    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callBack);

private:
    // RPC方法执行过程中的状态
    bool m_failed;
    // RPC方法执行过程中的错误信息
    std::string m_errText;
};