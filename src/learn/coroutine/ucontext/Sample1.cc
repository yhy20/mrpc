#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

#include <string>
#include <iostream>

int g_count = 0;
ucontext_t g_ctx;

void Task()
{   
    static int taskCount = 0;
    taskCount++;
    g_count++;
    if(g_count < 10)
    {
        setcontext(&g_ctx);
    } 
    std::cout << "taskCount = " << taskCount << std::endl;
}

/// 思考如何保存局部函数的栈变量的信息
int main()
{
    int mainCount = 0;
    getcontext(&g_ctx);
    mainCount++;
    g_count++;
    Task();
    std::cout << "g_count = " << g_count << std::endl;
    std::cout << "mainCount = " << mainCount << std::endl;
    return 0;
}