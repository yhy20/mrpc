#ifndef __MPRPC_H__
#define __MPRPC_H__

#include <memory>
#include "MrpcChannel.hpp"
#include "MrpcController.hpp"
#include "ConfigFileReader.hpp"
#include "RpcProvider.hpp"
#include "Logger.hpp" 

// mprpc 框架的基础类，负责一些初始化操作 
class Mrpc
{
public:
    static void Init(int argc, char* argv[]);
    static Mrpc& GetInstance();
    static CConfigFileReader& GetConfig();
   
private:
    Mrpc(){}
    ~Mrpc(){}
    Mrpc(Mrpc& rhs) = delete;
    Mrpc(Mrpc&& rhs) = delete;
    Mrpc& operator=(const Mrpc& rhs) = delete;
    static void ShowHelp(const char* name);

private:
    static CConfigFileReader m_config;
};

#endif