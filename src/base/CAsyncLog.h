#ifndef __ASYNC_LOG_H__
#define __ASYNC_LOG_H__

#include <stdio.h>
#include <string>
#include <thread>
#include <memory>
#include <atomic>

#include "ThreadSafeQueue.h"

namespace mrpc
{

enum LOG_LEVEL
{   
    LOG_LEVEL_TRACE,     
    LOG_LEVEL_DEBUG,     
    LOG_LEVEL_INFO,      
    LOG_LEVEL_WARNING,  
    LOG_LEVEL_ERROR,     
    LOG_LEVEL_SYSERROR, 
    LOG_LEVEL_FATAL,     
    LOG_LEVEL_CRITICAL 
};

// 打印详细的文件位置信息和行位置信息
#define LOGT(...)    CAsyncLog::output(LOG_LEVEL_TRACE, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGD(...)    CAsyncLog::output(LOG_LEVEL_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGI(...)    CAsyncLog::output(LOG_LEVEL_INFO, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGW(...)    CAsyncLog::output(LOG_LEVEL_WARNING, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGE(...)    CAsyncLog::output(LOG_LEVEL_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGSYSE(...) CAsyncLog::output(LOG_LEVEL_SYSERROR, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGF(...)    CAsyncLog::output(LOG_LEVEL_FATAL, __FILE__, __LINE__, ##__VA_ARGS__)        
#define LOGC(...)    CAsyncLog::output(LOG_LEVEL_CRITICAL, __FILE__, __LINE__, ##__VA_ARGS__)    

// 不打印详细的文件位置信息和行位置信息
#define LOGT_N(...)    CAsyncLog::output(LOG_LEVEL_TRACE, ##__VA_ARGS__)
#define LOGD_N(...)    CAsyncLog::output(LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#define LOGI_N(...)    CAsyncLog::output(LOG_LEVEL_INFO, ##__VA_ARGS__)
#define LOGW_N(...)    CAsyncLog::output(LOG_LEVEL_WARNING, ##__VA_ARGS__)
#define LOGE_N(...)    CAsyncLog::output(LOG_LEVEL_ERROR, ##__VA_ARGS__)
#define LOGSYSE_N(...) CAsyncLog::output(LOG_LEVEL_SYSERROR, ##__VA_ARGS__)
#define LOGF_N(...)    CAsyncLog::output(LOG_LEVEL_FATAL, ##__VA_ARGS__)        
#define LOGC_N(...)    CAsyncLog::output(LOG_LEVEL_CRITICAL, ##__VA_ARGS__)    

/**
 * @brief 一个基于线程安全队列实现的轻量备用的异步日志类
 */
class CAsyncLog
{
public:
    // 初始化并开始日志工作线程
    static bool init(const char* fileName = nullptr, bool toConsole = true , bool truncateLongLine = false, int64_t rollSize = 10 * 1024 * 1024);
    // 停止日志类工作线程
    static void uninit();
    // 设置日志级别
    static void setLevel(LOG_LEVEL level);
    // 判断日志工作线程是否在运行
    static bool isRunning();
    // 不输出线程 ID 号和所在函数签名、行号的 output 函数
    static bool output(LOG_LEVEL level, const char* format, ...);
    // 输出线程 ID 号和所在函数签名、行号的 output 函数
    static bool output(LOG_LEVEL level, const char* sourceCodeFileName, int lineNumber, const char* format, ...);

private:
    CAsyncLog() = delete;
    CAsyncLog(const CAsyncLog& rhs) = delete;
    CAsyncLog& operator=(const CAsyncLog& rhs) = delete;
    ~CAsyncLog() = delete;

private:
    static void makeLinePrefix(LOG_LEVEL level, std::string& strPrefix);
    static bool createNewFile(const char* logFileName);
    static bool writeToFile(const std::string& data);
    static void crash();
    static void getTime(char* timeStamp, int timeStampSize, bool isForFileName = false);
    static void writeLogTask();

private:
    static bool                             m_toConsole;            // 控制日志是否写到控制台
    static FILE*                            m_logFilePtr;           // 日志文件指针
    static std::string                      m_logFileName;          // 日志文件名
    static std::string                      m_processPID;           // 日志进程的 PID
    static bool                             m_truncateLongLog;      // 长日志是否截断  
    static LOG_LEVEL                        m_currentLevel;         // 当前日志级别 
    static int64_t                          m_fileRollSize;         // 单个日志文件记录的最大字节数
    static int64_t                          m_currentWrittenSize;   // 当前已经写入日志的字节数
    static ThreadSafeQueue<std::string>     m_logMsgQueue;          // 日志消息队列
    static std::unique_ptr<std::thread>     m_writeThread;          // 向磁盘 I/O 中写日志的工作线程
    static std::atomic<bool>                m_exit;                 // 退出标志
    static bool                             m_running;              // 运行标志
};

}  // namespace mrpc
#endif /*__ASYNC_LOG_H__*/