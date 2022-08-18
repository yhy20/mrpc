#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "CurrentThread.h"

using namespace mrpc;

void TestThreadLocal()
{
    printf("pid = %d, tid = %d\n", ::getpid(), CurrentThread::Tid());
    printf("TidString = %s\n", CurrentThread::TidString());
    printf("TidStringLength = %d\n", CurrentThread::TidStringLength());
    printf("ThreadName = %s\n", CurrentThread::ThreadName());
    std::cout << "IsMainThread: " << (CurrentThread::IsMainThread() ? "True" : "False") << std::endl;
}

namespace
{
    thread_local int x = 0;
} 

void Print()
{
    printf("pid = %d, tid = %d, x = %d\n", getpid(), CurrentThread::Tid(), x);
}

/// 在线程中 fork 进程的思考
void TestFork()
{
    printf("parent %d\n", ::getpid());
    Print();
    x = 1;
    Print();
    pid_t p = fork();

    /// child
    if(p == 0)
    {
        printf("child %d\n", ::getpid());
        Print();
        x = 2;
        Print();

        if (fork() == 0)
        {
            printf("grandchlid %d\n", getpid());
            Print();
            x = 3;
            Print();
        }
    }
    /// parent
    else
    {
        Print();
    }
}

int main()
{
    TestThreadLocal(); 
    // TestFork();

    return 0;
}