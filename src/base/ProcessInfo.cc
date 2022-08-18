#include <pwd.h>
#include <unistd.h>
#include <sys/times.h>

#include "FileUtil.h"
#include "ProcessInfo.h"
#include "CurrentThread.h"

namespace mrpc
{

namespace details
{

thread_local int t_numOpenedFiles = 0;

TimeStamp g_startTime = TimeStamp::Now();

/// assume those won't change during the life time of a process.
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));

}  // namespace details

namespace ProcessInfo
{

pid_t Pid()
{
    return ::getpid();
}

std::string PidString()
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", Pid());
    return buf;
}

uid_t Uid()
{
    return ::getuid();
}

std::string UserName()
{
    struct passwd pwd;
    struct passwd* result = nullptr;
    char buf[8192];
    const char* name = "unknownuser";

    getpwuid_r(Uid(), &pwd, buf, sizeof(buf), &result);

    if(result)
    {
        name = pwd.pw_name;
    }
    return name;
}

uid_t Euid()
{
    return ::geteuid();
}

TimeStamp StartTime()
{
    return details::g_startTime;
}

int ClockTicksPerSecond()
{
    return details::g_clockTicks;
}

int pageSize()
{
    return details::g_pageSize;
}

bool IsDebugBuild()
{
#ifdef NDEBUG
    return false;
#else   
    return true;
#endif    
}

std::string HostName()
{
    char buf[256];
    if(::gethostname(buf, sizeof(buf)) == 0)
    {
        buf[sizeof(buf) -1] = '\0';
        return buf;
    }
    else
    {
        return "unknownhost";
    }
}

std::string ProcName()
{
    return ProcName(ProcStat()).as_string();
}

StringPiece ProcName(const std::string& stat)
{
    StringPiece name;
    size_t lp = stat.find('(');
    size_t rp = stat.rfind(')');
    if (lp != std::string::npos && rp != std::string::npos && lp < rp)
    {
        name.set(stat.data()+lp+1, static_cast<int>(rp-lp-1));
    }
    return name;
}

std::string ProcStatus()
{
    std::string result;
    FileUtil::ReadFile("/proc/self/status", 65536, &result);
    return result;
}

std::string ProcStat()
{
    std::string result;
    FileUtil::ReadFile("/proc/self/stat", 65536, &result);
    return result;
}

std::string ThreadStat()
{
    char buf[64];
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/stat", CurrentThread::Tid());
    std::string result;
    FileUtil::ReadFile(buf, 65536, &result);
    return result;
}

std::string ExePath()
{
    std::string result;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf));
    if (n > 0)
    {
        result.assign(buf, n);
    }
    return result;
}

CpuTime GetCpuTime()
{
    CpuTime time;
    struct tms tms;
    if(::times(&tms) >= 0)
    {
        const double ctps = static_cast<double>(ClockTicksPerSecond());
        time.userSeconds = static_cast<double>(tms.tms_utime) / ctps;
        time.systemSeconds = static_cast<double>(tms.tms_stime) / ctps;
    }
    return time;
}

int ThreadsNum()
{
    int result = 0;
    std::string status = ProcStatus();
    size_t pos = status.find("Threads:");
    if (pos != std::string::npos)
    {
        result = ::atoi(status.c_str() + pos + 8);
    }
    return result;
}

// std::vector<pid_t> Threads()
// {
//     std::vector<pid_t> result;
//     t_pids = &result;
//     scanDir("/proc/self/task", taskDirFilter);
//     t_pids = NULL;
//     std::sort(result.begin(), result.end());
//     return result;
    
// }

}  // namespace ProcessInfo
}  // namesapce mrpc