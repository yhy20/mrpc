// #include <fcntl.h>

// #include <fcntl.h>
// #include <unistd.h>

// #include <sys/stat.h>
// #include <sys/timeb.h>
// #include <sys/syscall.h>

#include <stdarg.h>
#include <cxxabi.h>
#include <stdlib.h>
#include <execinfo.h>

#include <mutex>
#include <thread>
#include <algorithm>

#include "Util.h"

namespace mrpc
{

std::mutex Util::s_mutex;

void Util::SleepSec(int64_t seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Util::SleepMsec(int64_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Util::SleepUSec(int64_t microseconds)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

void Util::SleepNsec(int64_t nanoseconds)
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(nanoseconds));
}

// bool Util::Check(const std::string& str, CheckType type)
// {
//     switch(type)
//     {
//         case YamlConfig: 
//             if(str.find_first_not_of("abcdefghikjlmnopqrstuvwxyzABCDEFGHIKJLMNOPQRSTUVWXYZ_.012345678")
//                 != std::string::npos)
//             {
//                 return false;
//             }
//             else
//             {
//                 return true;
//             }      
//         default:
//             SYNC_ERROR("check type error!");
//     }
//     return false;
// }

bool Util::Error(const char* fileName, int lineNumber, const char* fmt, ...)
{   
    std::lock_guard<std::mutex> lock(s_mutex);
    va_list ap;
    va_start(ap, fmt);
    char* errMsg = nullptr;
    int len = vasprintf(&errMsg, fmt, ap);
    if(-1 == len)
    {
        va_end(ap);
        fprintf(stderr, "[%s:%d]%s\n", fileName, lineNumber, errMsg);
        return false;
    }
    va_end(ap);
    fprintf(stderr, "[%s:%d]%s\n", fileName, lineNumber, errMsg);
    return true;
}

int Util::HardwareThreads(int minThreads /* = 4 */)
{
    int hardwareThreads = static_cast<int>(std::thread::hardware_concurrency());
    return static_cast<int>((hardwareThreads > minThreads ? hardwareThreads : minThreads));

}

void Util::PrintTitle(StringArg title)
{
    printf("<------------%s------------>\n", title.c_str());
}

std::string Util::StackTrace(bool demangle /* = true */)
{
    std::string stackTrace;
    const int maxFrameSize = 200;
    void* frames[maxFrameSize];
    int nptrs = ::backtrace(frames, maxFrameSize);
    char** stacks = ::backtrace_symbols(frames, nptrs);
    if (stacks)
    {
        size_t len = 256;
        char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
        for (int i = 1; i < nptrs; ++i)  // skipping the 0-th, which is this function
        {
        if (demangle)
        {
            // https://panthema.net/2008/0901-stacktrace-demangled/
            // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
            char* leftPar = nullptr;
            char* plus = nullptr;
            for (char* p = stacks[i]; *p; ++p)
            {
            if (*p == '(')
                leftPar = p;
            else if (*p == '+')
                plus = p;
            }

            if (leftPar && plus)
            {
            *plus = '\0';
            int status = 0;
            char* ret = abi::__cxa_demangle(leftPar+1, demangled, &len, &status);
            *plus = '+';
            if (status == 0)
            {
                demangled = ret;  // ret could be realloc()
                stackTrace.append(stacks[i], leftPar+1);
                stackTrace.append(demangled);
                stackTrace.append(plus);
                stackTrace.push_back('\n');
                continue;
            }
            }
        }
        // Fallback to mangled names
        stackTrace.append(stacks[i]);
        stackTrace.push_back('\n');
        }
        free(demangled);
        free(stacks);
    }
    return stackTrace;
}

std::string Util::RandomString(int size)
{
    assert(size > 0);
    std::string str(size, '\0');
    static int count = 0;
    static unsigned int mod = static_cast<unsigned int>('~' - ' ' + 1);
    srand(static_cast<unsigned int>(time(nullptr) + count));
    for(int i = 0; i < size; ++i)
    {
        if(count % 10000 == 0)
        {
            srand(static_cast<unsigned int>(time(nullptr) + count));
        }
        str[i] = static_cast<char>(rand() % mod + ' ');
        ++count;
    }
    count %= 10 * 1000 * 1000;
    return str;
}

bool Util::CreateFile(StringArg fileName, int64_t fileSize, int lineSize)
{
    if(fileSize <= 0) 
    {
        SYNC_ERROR("file size error, file size = %ld\n", fileSize);
        return false;
    }
    
    mode_t old = umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int fd = open(fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
    if(-1 == fd)
    {
        SYNC_ERROR("file open error, paht = %s!", fileName.c_str());
        return false;
    }
#undef RWRWRW
    umask(old);

    /// ' ' 与 '~' 分别是 ASCII 编码中最小和最大的可见字符 
    const int mod = int('~' - ' ') + 1;
    int64_t writtenFileSize = 0;
    char buf[BUFSIZ];
    srand(static_cast<unsigned int>(time(nullptr)));
    while(writtenFileSize < fileSize)
    {
        if(writtenFileSize % 10000 == 0) 
        {
            srand(static_cast<unsigned int>(time(nullptr)));
        }

        int bufSize = (fileSize - writtenFileSize > BUFSIZ ? 
            BUFSIZ :  static_cast<int>(fileSize - writtenFileSize));

        int writtenBufSize = 0;

        while(writtenBufSize < bufSize)
        {
            if((writtenFileSize + writtenBufSize) % lineSize == 0)
                buf[writtenBufSize++] = '\n';
            else
                buf[writtenBufSize++] = static_cast<char>(rand() % mod + ' ');  
        }
        int64_t len = write(fd, buf, writtenBufSize); 
        assert(len == writtenBufSize);
        writtenFileSize += len;
    }
    
    return true;
}

}  // namespace mrpc