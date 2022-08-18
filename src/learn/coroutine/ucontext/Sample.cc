#include <stdio.h>
#include <unistd.h>
#include <ucontext.h>

#include <string>
#include <iostream>

int g_count = 0;

std::string g_str;

/// 由下面代码可以看出，协程保存的上下文信息只有代码的执行信息，不包括进程的资源信息
int main()
{   
    /// int getcontext (ucontext_t *ucp); 获取当前协程的 context 并保存到 ucp 指向的内存。
    /// int setcontext (const ucontext_t *ucp); 设置 ucp 指向的 context 为运行上下文并开始执行。
    int count = 0;
    g_str.reserve(100);
    ucontext_t ctx;
    getcontext(&ctx);
    printf("count = %d\n", count);
    g_str += std::to_string(count);

    if(count++ < 10)
    {
        setcontext(&ctx);
    }

    std::cout << "g_str = " << g_str << std::endl;
}