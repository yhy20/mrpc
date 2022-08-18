#include <stdio.h>
#include <unistd.h>

#include "Util.h"
#include "Thread.h"
#include "Logging.h"
#include "EventLoop.h"

using namespace mrpc;
using namespace mrpc::net;

int g_count = 0;
EventLoop* g_loop;

void PrintTid()
{
    printf("[%s] pid = %d, tid = %d\n",
           TimeStamp::Now().toFormattedString().c_str(),
           ::getpid(), CurrentThread::Tid());
}

void Print(const char* msg)
{
    printf("[%s] %s\n",TimeStamp::Now().toFormattedString().c_str(), msg);
    if(++g_count == 20)
    {
        g_loop->quit();
    }
}

void Cancel(TimerId timerId, const char* msg)
{
    g_loop->cancel(timerId);
    printf("[%s] %s cancelled\n",
           TimeStamp::Now().toFormattedString().c_str(), msg);
}

void TestRunAfter(TimeStamp now)
{
    g_loop->runAfter(0, std::bind(Util::PrintTitle, "TestRunAfter"));
    g_loop->runAfter(1, std::bind(Print, "once1"));
    g_loop->runAfter(1.5, std::bind(Print, "once1.5"));
    g_loop->runAfter(2.5, std::bind(Print, "once2.5"));
    g_loop->runAfter(3.5, std::bind(Print, "once3.5"));
    TimerId t45 = g_loop->runAfter(4.5, std::bind(Print, "once4.5"));
    g_loop->runAfter(4.2, std::bind(Cancel, t45, "t45"));
    g_loop->runAfter(4.8, std::bind(Cancel, t45, "t45"));
   
}

void TestRunAt(TimeStamp now)
{
    g_loop->runAt(now, std::bind(Util::PrintTitle, "TestRunAt"));
    g_loop->runAt(AddTime(now, 1), std::bind(Print, "once1"));
    g_loop->runAt(AddTime(now, 1.5), std::bind(Print, "once1.5"));
    g_loop->runAt(AddTime(now, 2.5), std::bind(Print, "once2.5"));
    g_loop->runAt(AddTime(now, 3.5), std::bind(Print, "once3.5"));
    TimerId t45 = g_loop->runAt(AddTime(now, 4.5), std::bind(Print, "once4.5"));
    g_loop->runAt(AddTime(now, 4.2), std::bind(Cancel, t45, "t45"));
    g_loop->runAt(AddTime(now, 4.8), std::bind(Cancel, t45, "t45"));
}

void TestRunEveryAt(TimeStamp now)
{
    g_loop->runAt(now, std::bind(Util::PrintTitle, "TestRunEveryAt"));
    TimerId t3 = g_loop->runEveryAt(now, 3, std::bind(Print, "every3"));
    g_loop->runAt(AddTime(now, 9.001), std::bind(Cancel, t3, "t3"));
}

/// TODO: 有多个同时刻的定时器时，按照入队顺序执行
int main()
{
    PrintTid();
    Print("main");
    EventLoop loop;
    g_loop = &loop;
    Logger::SetLogLevel(Logger::TRACE);

    TimeStamp now = TimeStamp::Now();
    TestRunAfter(now);
    TestRunAt(AddTime(now, 5));
    TestRunEveryAt(AddTime(now, 10));
    loop.runAt(AddTime(now, 20), std::bind(Util::PrintTitle, "runEvery until loop exit"));
    loop.runEveryAt(AddTime(now, 20), 0.25, std::bind(Print, "every0.25"));
    loop.loop();
    Print("main loop exits");

    return 0;
}

