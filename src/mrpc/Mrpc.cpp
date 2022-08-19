#include <unistd.h>
#include <libgen.h>
#include <iostream>
#include <string>
#include "Mrpc.hpp"

CConfigFileReader Mrpc::m_config;

void Mrpc::ShowHelp(const char* name)
{
    std::cout << "Usage: " << name << " -i <config file>"<<std::endl;
}

void Mrpc::Init(int argc, char* argv[])
{
    if(argc < 2)
    {
        ShowHelp(basename(argv[0]));
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch(c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            // std::cout<<"invalid args!"<<std::endl;
            // ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            // ShowArgsHelp();
            // std::cout<<"no <config file>!"<<std::endl;
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // // TODO：config 类如何设计更好
    // // 加载配置文件
    m_config.loadFile(config_file.c_str());
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
