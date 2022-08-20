#include "iostream"
#include "Mrpc.hpp"
#include "UserProtoBuf.pb.h"
#include "MrpcChannel.hpp"

int main(int argc, char* argv[])
{
    // 初始化读取配置文件
    Mrpc::Init(argc, argv);
    
    // 创建 Service 服务调用对象
    rpc_service::UserService_Stub stub(new MrpcChannel);

    // 创建 rpc 调用请求对象并填充响应信息
    rpc_service::LoginRequest request;
    request.set_name("yhy");
    request.set_password("123456");

    // 创建 rpc 调用响应对象和控制对象
    rpc_service::LoginResponse response;
    MrpcController controller;

    // 发起 rpc 方法的调用，进程阻塞直到收到远端的响应信息
    stub.Login(&controller, &request, &response, nullptr);

    // 检查调用的控制信息
    if(controller.Failed())
    {
        // 调用发生异常
        exit(EXIT_FAILURE);
    }

    // rpc 调用完成，读取远端响应的结果
    if(0 == response.code().errcode())
    {
        std::cout << "rpc login response success!" << std::endl;
        std::cout << response.code().errmsg() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error!" << std::endl;
    }

    return 0;
}
