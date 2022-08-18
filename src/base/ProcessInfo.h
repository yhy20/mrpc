#ifndef __MRPC_BASE_PROCESSINFO_H__
#define __MRPC_BASE_PROCESSINFO_H__

#include <sys/types.h>

#include <vector>

#include "TimeStamp.h"
#include "StringPiece.h"

namespace mrpc
{
namespace ProcessInfo
{

pid_t Pid();
std::string PidString();
uid_t Uid();
std::string UserName();
uid_t Euid();
TimeStamp StartTime();

int ClockTickPerSecond();

int PageSize();

bool IsDebugBuild();

std::string HostName();

std::string ProcName();

StringPiece ProcName(const std::string& stat);

std::string ProcStatus();

std::string ProcStat();

std::string ThreadStat();

std::string ExePath();

int OpenedFiles();

int MaxOpenFiles();

/// 什么情况下 cputime < walltime 或 cputime > walltime
struct CpuTime
{
    double userSeconds;
    double systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }
    double total() const { return userSeconds + systemSeconds; }
};

CpuTime GetCpuTime();

int ThreadsNum();

// std::vector<pid_t> Threads();

}  // namespace ProcessInfo
}  // namespace mrpc

#endif  // __MRPC_BASE_PROCESSINFO_H__