#ifndef __MRPC_CONTROLLER__
#define __MRPC_CONTROLLER__

#include <google/protobuf/service.h>
#include <string>

class MrpcController : public google::protobuf::RpcController
{
public:
    MrpcController();
    virtual void Reset() override;
    virtual bool Failed() const override;
    virtual std::string ErrorText() const override;
    virtual void SetFailed(const std::string& reason) override;

    // 目前未实现具体的功能
    virtual void StartCancel() override;
    virtual bool IsCanceled() const override;
    virtual void NotifyOnCancel(google::protobuf::Closure* callback) override;
private:
    bool m_failed; // RPC方法执行过程中的状态
    std::string m_errText ; // RPC方法执行过程中的错误信息
};

#endif