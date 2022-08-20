#include <thread>
#include "RpcProvider.h"
#include "RpcHeader.pb.h"
#include "Mrpc.hpp"

namespace mrpc
{

// 注册 rpc 服务
void RpcProvider::RegisterService(google::protobuf::Service *service)
{
    ServiceInfo serviceInfo;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor* serviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string serviceName = serviceDesc->name();
    // 获取服务对象方法的数量
    int methodCount = serviceDesc->method_count();

    // debug
    // std::cout << "service_name = " << service_name << std::endl;

    // 定义服务方法描述变量
    const google::protobuf::MethodDescriptor* methodDesc;
    for(int i = 0; i < methodCount; ++i)
    {   
        // 获取服务对象指定下标的服务方法的描述
        methodDesc = serviceDesc->method(i);
        std::string methodName = methodDesc->name();
        serviceInfo.m_methodMap.insert({methodName, methodDesc});
       
        // debug
        // std::cout << "method_name = " << method_name << std::endl;
    }   

    // 注册 Service 服务对象
    service_info.m_service = service;
    m_serviceMap.insert({serviceName, std::move(serviceInfo)});
}

// 启动 rpc 服务节点，开始提供 rpc 远程网络调用服务
void RpcProvider::Run()
{
    std::string ip = Mrpc::GetConfig().getConfigName("rpcserverip");
    uint16_t port = atoi(Mrpc::GetConfig().getConfigName("rpcserverport"));
    net::InetAddress address(ip, port);

    // 创建 TcpServer 对象
    net::TcpServer server(&m_loop, address, "RpcProvider");

    // 注册 Acceptor 回调
    server.setConnectionCallback(
        std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1)
    );

    // 注册 TcpConnection 回调                
    server.setMessageCallback(
        std::bind(&RpcProvider::OnMessage, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );

    // 设置线程池中线程的数目
    unsigned int hardware_threads = std::thread::hardware_concurrency();
    hardware_threads = (hardware_threads < 4) ? 4 : hardware_threads; 
    server.setThreadNum(hardware_threads);

    std::cout << "RpcProvider service start at ip:" << ip << " port:" << port << std::endl;
    // 启动网络服务
    server.start();
    m_loop.loop();
}


// 与新 socket 建立连接的 Acceptor 回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {   
        // 与 rpc client 断开连接
        conn->shutdown();
    }
}

// 已建立连接用户的读写事件回调，如果客户端发起 rpc 调用请求，则 OnMessage 方法会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp time)
{
    // 从网络上接收的远程 rpc 调用请求的字符流
    std::string request_str = buffer->retrieveAllAsString();

    // rpc 调用的服务器、客户端应当协商好通信使用的 protobuf 数据类型
    // 通过定义 protobuf 的 message 类型进行序列化和反序列化 （service_name, method_name, args）

    // 从字符流中读取前 4 个字节
    uint32_t request_header_size = 0;
    request_str.copy((char*)&request_header_size, 4, 0);
    
    // 根据 header_size 读取数据头的原始字符流，反序列化数据，得到 rpc 请求的详细信息
    std::string request_header_str = request_str.substr(4, request_header_size);
    mrpc::RpcHeader request_header_proto;
    std::string request_service_name;
    std::string request_method_name;
    uint32_t request_body_size;
    if(!request_header_proto.ParseFromString(request_header_str))
    {
        // 数据头反序列化失败
        // 记录日志
        exit(-1);
    }
    
    // 数据头反序列化成功
    request_service_name = std::move(request_header_proto.service_name());
    request_method_name = std::move(request_header_proto.method_name());
    request_body_size = request_header_proto.body_size();


    // 获取 rpc 方法参数的字符流数据
    std::string  request_body_str = request_str.substr(4 + request_header_size, request_body_size);

    // for debug
    ///////////////////////////////////////////////////////////////////////////
    std::cout << "request_header_size = " << request_header_size << std::endl;
    std::cout << "request_header_str = " << request_header_str << std::endl;
    std::cout << "request_service_name = " << request_service_name << std::endl;
    std::cout << "request_method_name = " << request_method_name << std::endl;
    std::cout << "request_body_size = " << request_body_size << std::endl;
    std::cout << "request_body_str = " << request_body_str << std::endl;
    ///////////////////////////////////////////////////////////////////////////

    // 获取 service 对象与 method 对象
    auto service_itr = m_serviceMap.find(request_service_name);
    if(service_itr == m_serviceMap.end())
    {
        // 异常，记录日志
        exit(-1);
    }  

    auto method_itr = service_itr->second.m_methodMap.find(request_method_name);
    if(method_itr == service_itr->second.m_methodMap.end())
    {
        // 异常，记录日志
        exit(-1);
    }

    google::protobuf::Service * service = service_itr->second.m_service;
    const google::protobuf::MethodDescriptor* method = method_itr->second; 

    // 生成 rpc 方法的请求 request 和 响应参数
    google::protobuf::Message * request_body_proto= service->GetRequestPrototype(method).New();
    if(!request_body_proto->ParseFromString(request_body_str))
    {
        // 异常，记录日志
        exit(-1);
    }
    google::protobuf::Message *response_proto = service->GetResponsePrototype(method).New();

    // TODO：待解决问题，模板类型推导有误
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, 
                                      const muduo::net::TcpConnectionPtr&,
                                      google::protobuf::Message*>
                                      (this, &RpcProvider::SendRpcResponse, conn, response_proto);

    service->CallMethod(method, nullptr, request_body_proto, response_proto, done);
}

// Closure 回调，用于框架 response 报文的序列化和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response_proto)
{
    std::string response_str;
    if(!response_proto->SerializeToString(&response_str))
    {
        // 序列化异常，记录日志，也需要 shutdown
        conn->shutdown();
        exit(-1);
    }
    conn->send(response_str);
    conn->shutdown();
}

}  // namespace mrpc