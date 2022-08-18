#include <thread>
#include <atomic>

#include "Util.h"
#include "Logging.h"
#include "LogFile.h"
#include "TimeZone.h"
#include "ThreadPool.h"

using namespace mrpc;

std::atomic<int> g_count(0);

void BasicTest()
{
    Util::PrintTitle("BaiscTest");
    LOG_TRACE << "LOG_TRACE";
    LOG_DEBUG << "LOG_DEBUG";
    LOG_INFO << "LOG_INFO";
    LOG_WARN << "LOG_WARN";
    LOG_ERROR << "LOG_ERROR";
    LOG_SYSERR << "LOG_SYSERROR";
    CLOG_TRACE("%s, x = %d, f = %f", "CLOG_TRACE", 1, 12.34);
    CLOG_DEBUG("%s, x = %d, f = %f", "CLOG_DEBUG", 2, 12.34);
    CLOG_INFO("%s, x = %d, f = %f", "CLOG_INFO", 3, 12.34);
    CLOG_WARN("%s, x = %d, f = %f", "CLOG_WARN", 4, 12.34);
    CLOG_ERROR("%s, x = %d, f = %f", "CLOG_ERROR",5, 12.34);
    CLOG_SYSERR("%s, x = %d, f = %f", "CLOG_SYSERR", 6, 12.34);
}

void ThreadTask(int num)
{
    g_count.fetch_add(1, std::memory_order_relaxed);
    LOG_INFO << "Thread" << num << " run!";
}

void LogThreadSafeTest()
{
    Util::PrintTitle("LogThreadSafeTest");
    ThreadPool pool("pool");
    pool.start(5);
    for(int i = 0; i < 100; ++i)
    {
        pool.run(std::bind(ThreadTask, i + 1));
    }
    
    while(pool.queueSize() > 0)
    {
        std::this_thread::yield();
    }
    pool.stop();
    std::cout << "g_count = " << g_count << std::endl;
}

/// 测试写 fd
int fd = 0;
/// 测试写 FILE* 
FILE* g_file;
/// 测试写 LogFile
std::unique_ptr<LogFile> g_logFile;

int g_total;
int fd_total;
int fp_total;

void DummyOutput(const char* msg, int len)
{
    g_total += len;
    if(fd > 0)
    {
        ssize_t written = write(fd, msg, len);
        fd_total += static_cast<int>(written);
    }
    else if(g_file)
    {
        ssize_t written = fwrite(msg, 1, len, g_file); 
        fp_total += static_cast<int>(written);
    }
    else if (g_logFile)
    {
        g_logFile->append(msg, len);
    } 
}

void Benchmark(const char* type)
{
    Logger::SetOutput(DummyOutput);
    g_total = 0;
    fd_total = 0;
    fp_total = 0;
    int n = 1000 * 1000;
    /// TODO: 探究 longLog 下写入速度 1000Mb 的原因
    const bool longLog = false;
    std::string empty = " ";
    std::string longStr(3000, 'x');
    longStr += " ";
    TimeStamp start(TimeStamp::Now());
    for(int i = 0; i < n; ++i)
    {
        // LOG_INFO << "LOG Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
        //          << (longLog ? longStr : empty)
        //          << i + 1;
        CLOG_INFO("CLOG Hello 0123456789 abcdefghijklmnopqrstuvwxyz %d", i + 1);
    }
    TimeStamp stop(TimeStamp::Now());
    double seconds = TimeDifference(stop, start);
    printf("%12s: g_total = %d bytes, fd_total = %d bytes, fp_total = %d bytes\n" \
           "%12s: duration = %f seconds, %10.2f msg/s, %.2f MiB/s\n",
           type, g_total, fd_total, fp_total,
           "speed", seconds, n / seconds, g_total / seconds / (1024 * 1024));

}

void BenchTest()
{   
    Util::PrintTitle("BenchTest");
    /// 空操作
    Benchmark("nop");

    /// 直接写 fd，相当于行缓冲
    fd = open("/dev/null", O_WRONLY);
    Benchmark("fd");
    assert(g_total == fd_total);
    close(fd);
    fd = -1;

    // char buffer[64 * 1024];
    /// TODO: 测试最佳 buffer，目前 2*BUFSIZ 优于 BUFSIZ
    char buffer[2 * BUFSIZ];

    /// 写 /dev/null 特殊文件
    g_file = fopen("/dev/null", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    assert(g_total == fp_total);
    Benchmark("/dev/null");
    fclose(g_file);

    /// 写 /tmp/log 普通文件
    g_file = fopen("/tmp/log", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    Benchmark("/tmp/log");
    fclose(g_file);

    /// 非线程安全写 LogFile
    g_file = NULL;
    g_logFile.reset(new LogFile("test_log_st", 500*1000*1000, false));
    Benchmark("test_log_st");

    /// 线程安全写 LogFile
    g_logFile.reset(new LogFile("test_log_mt", 500*1000*1000, true));
    Benchmark("test_log_mt");
    g_logFile.reset();
}

/**
 * @brief SetTimeZone 函数非线程安全，在多线程环境下，必须要在打印日志前调用
 */
void TimeZoneTest()
{
    Util::PrintTitle("TimeZoneTest");
    g_file = stdout;

    Util::SleepSec(1);
    Util::PrintTitle("beijing");
    TimeZone beijing(8 * 3600, "CST");
    Logger::SetTimeZone(beijing);
    LOG_TRACE << "TRACE CST";
    LOG_DEBUG << "DEBUG CST";
    LOG_INFO << "INFO CST";
    LOG_WARN << "WARN CST";
    LOG_ERROR << "ERROR CST";

    /// 等待一秒，让时间戳刷新
    Util::SleepSec(1);      

    Util::PrintTitle("newyork");
    TimeZone newyork("/usr/share/zoneinfo/America/New_York");
    Logger::SetTimeZone(newyork);
    LOG_TRACE << "TRACE NYT";
    LOG_DEBUG << "DEBUG NYT";
    LOG_INFO << "INFO NYT";
    LOG_WARN << "WARN NYT";
    LOG_ERROR << "ERROR NYT";
    g_file = nullptr;
    Benchmark("nops");
} 

int main()
{      
    Logger::SetLogLevel(Logger::TRACE);
    // BasicTest();
    // LogThreadSafeTest();
    BenchTest();
    // TimeZoneTest();
    return 0;
}