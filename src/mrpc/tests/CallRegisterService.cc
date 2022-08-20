#include "iostream"
#include "Mrpc.hpp"
#include "UserProtoBuf.pb.h"


int main(int argc, char* argv[])
{
    // 初始化读取配置文件
    Mrpc::Init(argc, argv);
 
    // 创建 Service 服务调用类
    rpc_service::UserService_Stub stub(new MrpcChannel) ;

    rpc_service::RegisterRequest register_request;
    register_request.set_id(100);
    register_request.set_name("yhy");
    register_request.set_password("123456");

    rpc_service::RegisterResponse register_response;

    // 发起 rpc 方法的调用，阻塞直到
    stub.Register(nullptr, &register_request, &register_response, nullptr);
    // rpc 调用完成， 读取响应的结果
    if(0 == register_response.code().errcode())
    {
        std::cout << "rpc register response success!" << std::endl;
        std::cout << register_response.code().errmsg() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error!" << std::endl;
    }

    return 0;
}
