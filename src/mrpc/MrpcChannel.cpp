#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "MrpcChannel.hpp"
#include "RpcHeader.pb.hpp"
#include "MrpcController.hpp"
#include "Mrpc.hpp"

/*
    header_size + service_name + args_size + args
*/
void MrpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                             google::protobuf::RpcController* controller, 
                             const google::protobuf::Message* request_body_proto,
                             google::protobuf::Message* response_proto, 
                             google::protobuf::Closure* done)
{
    // 获取 RPC 请求的 service 和 method 类型
    const google::protobuf::ServiceDescriptor* request_service_desc = method->service();
    std::string request_service_name = request_service_desc->name();
    std::string request_method_name = method->name();

    // 反序列化 RPC 正文的数据，并获取正文字符串的长度
    std::string request_body_str;
    if(!request_body_proto->SerializeToString(&request_body_str))
    {
        controller->SetFailed("serialize request error!");
        return;
    }
    uint32_t request_body_size = request_body_str.size();

    // 初始化 RPC 请求 header 的数据，用于服务器端识别客户端请求的服务和方法
    mrpc::RpcHeader request_header_proto;
    request_header_proto.set_service_name(request_service_name);
    request_header_proto.set_method_name(request_method_name);
    request_header_proto.set_body_size(request_body_size); 

    // 反序列化 RPC 请求 header 的数据
    std::string request_header_str;
    if(!request_header_proto.SerializeToString(&request_header_str))
    {
        controller->SetFailed("serialize request error!");
        return;
    }
    uint32_t request_header_size = request_header_str.size();

    // 组织待发送的 RPC 请求字符串
    std::string request_str;
    request_str.insert(0, std::string((char*)&request_header_size, 4));
    request_str += request_header_str;
    request_str += request_body_str;

    // for debug
    ///////////////////////////////////////////////////////////////////////////
    std::cout << "request_header_size = " << request_header_size << std::endl;
    std::cout << "request_header_str = " << request_header_str << std::endl;
    std::cout << "request_service_name = " << request_service_name << std::endl;
    std::cout << "request_method_name = " << request_method_name << std::endl;
    std::cout << "request_body_size = " << request_body_size << std::endl;
    std::cout << "request_body_str = " << request_body_str << std::endl;
    ///////////////////////////////////////////////////////////////////////////

    // 如果成功创建了 socket 套接字，则在发生异常时或程序正常返回时
    // 需要释放 socket 套接字资源，此处使用智能指针自动释放资源，防止遗忘
    std::shared_ptr<int> clientfd(new int(-1), [](int* fd)->void {
        if(*fd != -1) close(*fd);
	});

    // 创建套接字
    *clientfd = socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == *clientfd)
    {
        controller->SetFailed("create socket error! errno:" + std::to_string(errno));
        return;
    }

    // 
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(sockaddr_in));
    serv_addr.sin_family = AF_INET;

    // 从服务配置中心寻找 ip 
    serv_addr.sin_port = htons(atoi(Mrpc::GetConfig().getConfigName("rpcserverport")));
    serv_addr.sin_addr.s_addr = inet_addr(Mrpc::GetConfig().getConfigName("rpcserverip"));

    // 连接远端 RPC 服务节点
    if(-1 == connect(*clientfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)))
    {
        // 日志
        controller->SetFailed("connect server error! errno:" + std::to_string(errno));
        return;
    }

    // 发送 RPC 调用请求信息
    if(-1 == send(*clientfd, request_str.c_str(), request_str.size(), 0))
    {
        controller->SetFailed("send request error! errno:" + std::to_string(errno));
        return;
    }

    // 接收远端 RPC 返回的响应信息
    char recv_buf[1024] = {0};
    int recv_size = recv(*clientfd, recv_buf, 1024, 0);
    if(-1 == recv_size)
    {
        controller->SetFailed("receive response error! errno:" + std::to_string(errno));
        return;
    }

    // 反序列化响应信息
    std::string response_str(recv_buf, 0, recv_size);
    if(!response_proto->ParseFromString(response_str))
    {
        controller->SetFailed("parse response error! errno:" + std::to_string(errno));
        return;
    }
}