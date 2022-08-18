#ifndef __MRPC_BASE_UTIL_H__
#define __MRPC_BASE_UTIL_H__

#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include <mutex>
#include <vector>
#include <string>

#include "StringPiece.h"

namespace mrpc
{

/// TODO: add log level
#define SYNC_INFO(...) Util::Error(__FILE__, __LINE__, ##__VA_ARGS__);
#define SYNC_ERROR(...) Util::Error(__FILE__, __LINE__, ##__VA_ARGS__);


class Util 
{
public:
    Util() = delete;
    ~Util() = delete;

public:
    static void SleepSec(int64_t seconds);

    static void SleepMsec(int64_t milliseconds);

    static void SleepUSec(int64_t microseconds);

    static void SleepNsec(int64_t nanoseconds);  

    static bool Error(const char* fileName, int lineNumber, const char* format, ...);

    static int HardwareThreads(int minThreads = 4);

    static void PrintTitle(StringArg title);

    static std::string StackTrace(bool demangle = true);

    static std::string RandomString(int size);

    static bool CreateFile(StringArg fileName, int64_t fileSize, int lineSize);

private:
    static std::mutex s_mutex;
};

}  // namespace mrcp

#endif  // __MRPC_BASE_UTIL_H__