//// PS：所有代码只在 linux 下测试运行，不涉及跨平台

#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <wchar.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <signal.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <mutex>
#include <iostream>


#define LOG_ERROR(...) Output(__FILE__, __LINE__, false, ##__VA_ARGS__)
#define LOG_FATAL(...) Output(__FILE__, __LINE__, true,##__VA_ARGS__)

/// 所有后续的测试代码都只会在一个源文件中引用 header.h 
/// 故在头文件中定义全局变量和函数不会导致链接时的二义性
std::mutex g_mtx;

// output 非线程安全，不希望在多线程测试中打印混乱，故加锁
void Output(const char* fileName, int lineNumber, bool crash, const char* format, ...)
{
    std::lock_guard<std::mutex> lock(g_mtx);
    va_list ap;
    va_start(ap, format);
    char* errMsg = nullptr;
    int len = vasprintf(&errMsg, format, ap);
    if(-1 == len)
    {
        va_end(ap);
        std::cerr << "[LOG_ERROR]:vasprintf() error!" << std::endl;
        if(crash) exit(EXIT_FAILURE);
    }
    va_end(ap);
    fprintf(stderr, "[%s][%s:%d][errno:%d][errno message:%s]:%s\n",
            (!crash) ? "ERROR" : "FATAL",
            basename((char*)fileName), 
            lineNumber,
            errno,
            strerror(errno),
            errMsg);

    free(errMsg);
    if(crash) exit(EXIT_FAILURE);    
}

class ClockTime
{
public:
    void start()
    {
        m_startTime = clock();
    }

    void stop()
    {
        m_stopTime = clock();
    }

    double duration()
    {
        return static_cast<double>(m_stopTime - m_startTime) / (CLOCKS_PER_SEC);
    }   

private:
    clock_t m_startTime;
    clock_t m_stopTime;
};

/// TODO: 确定 chrono 使用方法 
class WallTime
{
public:
    void start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    void stop()
    {
        m_stopTime = std::chrono::high_resolution_clock::now();
    }

    double duration()
    {
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(m_stopTime - m_startTime);
        return static_cast<double>(nanoseconds.count()) / s_nanosecondsPerSecond;
    }

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_stopTime;
    static const int s_nanosecondsPerSecond = 1000 * 1000 * 1000;
};

bool CreateFile(const std::string& fileName, int64_t fileSize, int lineSize)
{
    if(fileSize <= 0) 
    {
        LOG_ERROR("File size must > 0, file size = %ld\n", fileSize);
        return false;
    }
    
    mode_t old = umask(0022);
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
    int fd = open(fileName.c_str(), O_WRONLY | O_TRUNC | O_CREAT, RWRWRW);
    if(-1 == fd)
    {
        LOG_ERROR("file open error, path = %s!", fileName.c_str());
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

#endif // __HEADER_H__