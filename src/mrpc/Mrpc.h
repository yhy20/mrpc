#ifndef __MPRPC_H__
#define __MPRPC_H__

#include <memory>

#include "Logging.h"
#include "RpcProvider.h"
#include "MrpcChannel.h"
#include "StringPiece.h"
#include "MrpcController.h"
#include "ConfigFileReader.h"

namespace mrpc
{

// mprpc 框架的基础类，负责一些初始化操作 
class Mrpc : noncopyable
{
public:
    static void Init(StringArg configFile);
    static Mrpc& GetInstance();
    static CConfigFileReader& GetConfig();
   
private:
    Mrpc(){}
    ~Mrpc(){}
    static void ShowHelp(const char* name);

private:
    static CConfigFileReader m_config;
};

}  // namespace mrpc

#endif  // __MPRPC_H__