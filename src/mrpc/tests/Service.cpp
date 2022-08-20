#include <iostream>
#include <string>

#include "UserProtoBuf.pb.h"
#include "Mrpc.hpp"
#include "Logger.hpp"

class Service : public rpc_service::UserService
{
public:
    bool Login(std::string& name, std::string& password)
    {
        std::cout << "doing local service: Login!" << std::endl;
        std::cout << "name:" << name << " password:" << password << std::endl;
        return true;
    }

    bool Register(int id, std::string& name, std::string& password)
    {
        std::cout << "doing local service: Register!" << std::endl;
        std::cout << "id:" << id << " name:" << name << " password:" << password << std::endl;
        return true;
    }

    /*
    重写父类 LoginServiceRpc 的虚函数，由 rpc 框架自动
    1. caller => Login(name, password) => muduo => callee =>Login(LoginRequest)
    */
    virtual void Login(google::protobuf::RpcController* controller,
                       const ::rpc_service::LoginRequest* request,
                       rpc_service::LoginResponse* response,
                       google::protobuf::Closure* done) override
    {   
        // RPC 框架将从网络层获取的 LoginRequest（message) 并进行反序列化
        // 在框架内部通过父类 LoginServiceRpc 指针调用子类的 Login 函数
        // 并将反序列化后 message 作为参数传递进来，进行具体的本地业务处理
        std::string name = std::move(request->name());
        std::string password =std::move(request->password());

        // 做本地业务
        bool login_result = Login(name, password);

        // 添加响应信息
        response->set_success(login_result);
        rpc_service::ResultCode* code = response->mutable_code();
        code->set_errcode(0);
        code->set_errmsg("Test local service: Login!");
       
        // 执行回调操作，将响应信息序列化并通过网络发送
        done->Run();
    }

    virtual void Register(google::protobuf::RpcController* controller,
                          const rpc_service::RegisterRequest* request,
                          rpc_service::RegisterResponse* response,
                          google::protobuf::Closure* done) override

    {
        int id = request->id();
        std::string name = std::move(request->name());
        std::string password =std::move(request->password());

        // 做本地业务
        bool register_result = Register(id, name, password);

        // 添加响应信息
        response->set_success(register_result);
        rpc_service::ResultCode* code = response->mutable_code();
        code->set_errcode(0);
        code->set_errmsg("Test local service: Register!");
       
        // 执行回调操作，将响应信息序列化并通过网络发送
        done->Run();
    }
};


int main(int argc, char* argv[])
{
    // LOG_INFO("first log message!");
    // LOG_ERRO("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 调用框架的初始化操作
    Mrpc::Init(argc, argv);

    // 创建一个 rpc 服务发布对象
    RpcProvider provider;

    // 把 LoginService 服务注册到 provider 对象上
    provider.RegisterService(new Service);

    // 启动 rpc 提供服务，Run 调用后进程进入阻塞状态，等待远程 rpc 请求
    provider.Run();

    return 0;
}