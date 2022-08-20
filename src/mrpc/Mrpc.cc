#include <unistd.h>
#include <libgen.h>
#include <iostream>
#include <string>

#include "Mrpc.h"

namespace mrpc
{

CConfigFileReader Mrpc::m_config;

void Mrpc::Init(StringArg configFile)
{
    m_config.loadFile(configFile.c_str());
    // std::cout << m_config.getConfigName("rpcserverip")<< std::endl;
    // std::cout << m_config.getConfigName("rpcserverport")<< std::endl;
    // std::cout << m_config.getConfigName("zookeeperip")<< std::endl;
    // std::cout << m_config.getConfigName("rpcserverport")<< std::endl;
}

Mrpc& Mrpc::GetInstance()
{
    static Mrpc rpc;
    return rpc;
}

CConfigFileReader& Mrpc::GetConfig()
{
    return m_config;
}

}  // namespace mrpc
