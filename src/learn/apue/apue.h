// PS：所有代码只在 linux 下测试运行，不涉及跨平台
// 所有后续的测试代码都只会在一个源文件中引用 apue.h 
// 在头文件中定义全局变量和函数不会导致链接时的二义性

#ifndef __APUE_H__
#define __APUE_H__

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <limits.h>

#include <mutex>
#include <iostream>


#define LOG_ERROR(...) output(__FILE__, __LINE__, ##__VA_ARGS__)

std::mutex g_mtx;

// output 非线程安全，不希望在多线程测试中打印混乱，故加锁
void output(const char* fileName, int lineNumber, const char* format, ...)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    va_list ap;
    va_start(ap, format);
    char* msg = nullptr;
    int len = vasprintf(&msg, format, ap);
    if(-1 == len)
    {
        va_end(ap);
        std::cerr << "[LOG_ERROR]:vasprintf() error!" << std::endl;
        exit(-1);
    }
    va_end(ap);
    printf("[%s:%d][errno:%d][errno message:%s]:%s\n", 
           basename((char*)fileName), 
           lineNumber,
           errno,
           strerror(errno),
           msg);

    free(msg);
    exit(-1);    
}

#endif // __APUE_H__