#ifndef __RPC_PROVIDER_H__
#define __RPC_PROVIDER_H__

#include <map>
#include <string>
#include <google/protobuf/service.h>
#include "google/protobuf/descriptor.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/base/Timestamp.h>

// 框架提供的 rpc 服务发布对象
class RpcProvider 
{
public:
    // 注册 rpc 服务
    void RegisterService(google::protobuf::Service *service);

    // 启动 rpc 服务节点，开始提供 rpc 远程网络调用服务
    void Run();

private:
    // Acceptor 回调，用于建立新的连接或被动释放连接
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);

    // TcpConnection 回调，用于响应已连接的 TcpConnection 的读写事件
    void OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp time);
    
    // Closure 回调，用于框架 response 报文的序列化和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response);

private:
    // 事件循环
    muduo::net::EventLoop m_loop;

    // 存储 Service 服务对象的相关信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };

    // 存储所有注册的 Service 服务对象和其相关的服务信息
    std::map<std::string, ServiceInfo> m_serviceMap;
};

#endif