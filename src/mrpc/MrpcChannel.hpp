#ifndef __MRPC_CHANNEL_H__
#define __MRPC_CHANNEL_H__

#include <google/protobuf/service.h>

class MrpcChannel : public google::protobuf::RpcChannel
{
public:
    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller, 
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response, 
                            google::protobuf::Closure* done) override;
};

#endif 