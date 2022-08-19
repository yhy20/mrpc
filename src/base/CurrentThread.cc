#include <unistd.h>
#include <cxxabi.h>
#include <stdlib.h>
#include <execinfo.h>

#include <thread>
#include <iostream>

#include "CurrentThread.h"

namespace mrpc
{
namespace CurrentThread
{
namespace internal
{

thread_local int t_tid = 0;
thread_local char t_tidString[32] = { 0 };
thread_local int t_tidStringLength = 5;
thread_local const char* t_threadName = "UNKNOWN";  // 线程初始名称为未知
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int!");

}  // namespace internal

/// 主线程 id 即进程 id
bool IsMainThread()
{
    return Tid() == ::getpid();
}

}  // namespace CurrentThread
}  // namespace mrpc