#include <time.h>
#include <ctime>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "CAsyncLog.h"

namespace mrpc
{

#define MAX_LINE_LENGTH 256
#define DEFAULT_ROLL_SIZE 10 * 1024 * 1024

bool                            CAsyncLog::m_toConsole = true;  
FILE*                           CAsyncLog::m_logFilePtr = nullptr;               // 日志文件
std::string                     CAsyncLog::m_logFileName;                        // 日志文件名
std::string                     CAsyncLog::m_processPID = "";                    // 日志文件对应进程的 PID
bool                            CAsyncLog::m_truncateLongLog = false;            // 长日志是否截断
LOG_LEVEL                       CAsyncLog::m_currentLevel = LOG_LEVEL_INFO;      // 当前日志级别 
int64_t                         CAsyncLog::m_fileRollSize = DEFAULT_ROLL_SIZE;   // 单个日志文件记录的最大字节数
int64_t                         CAsyncLog::m_currentWrittenSize = 0;             // 当前已经写入日志的字节数
ThreadSafeQueue<std::string>    CAsyncLog::m_logMsgQueue;                        // 日志消息队列
std::unique_ptr<std::thread>    CAsyncLog::m_writeThread;                        // 向磁盘 I/O 中写日志的工作线程
std::atomic<bool>               CAsyncLog::m_exit(false);                        // 退出标志
bool                            CAsyncLog::m_running = false;                    // 运行标志

// 若不指定日志文件名，则默认将日志信息打印到控制台
// 若指定日志文件名，则将日志信息输出到日志文件且同时将日志信息打印到控制台
// 通过 toConsole 标志可以控制指定日志文件名时，是否向控制台打印信息
bool CAsyncLog::init(const char* fileName /* = nullptr */, 
                     bool toConsole /* = true */,
                     bool truncateLongLog /* = false */, 
                     int64_t fileRollSize /* = 10 * 1024 * 1024 */)
{
    if (!(fileName == nullptr || fileName[0] == 0))
        m_logFileName = fileName;

    m_toConsole = toConsole; 
    m_truncateLongLog = truncateLongLog;
    m_fileRollSize = fileRollSize;

    // 获取启动日志进程的 PID，方便快速查看同一个进程的不同日志文件
    char pid[8];
    snprintf(pid, sizeof(pid), "%05d", (int)::getpid());
    m_processPID = pid;

    // 启动工作线程
    m_writeThread.reset(new std::thread(writeLogTask));

    return true;
}

void CAsyncLog::uninit()
{   
    m_exit = true;

    // 最后一次退出消息入队，唤醒工作线程中的出队函数
    m_logMsgQueue.push("Exit!");

    // 等待工作线程结束
    if(m_writeThread->joinable())
        m_writeThread->join();
    
    // 关闭日志文件
    if(m_logFilePtr != nullptr)
    {
        fclose(m_logFilePtr);
        m_logFilePtr = nullptr;
    }
}

void CAsyncLog::setLevel(LOG_LEVEL level)
{
    if(level < LOG_LEVEL_TRACE || level > LOG_LEVEL_CRITICAL)
    {
        std::cerr << "imput log level error!" << std::endl;
        return;
    }
        
    m_currentLevel = level;
}

bool CAsyncLog::isRunning()
{
    return m_running;
}

void CAsyncLog::getTime(char* timeStamp, int timeStampLength, bool isForFileName /* = false */)
{
    struct timeb tb;
    ftime(&tb);

    time_t now = tb.time;
    tm time;
    localtime_r(&now, &time);

    // 给日志文件名使用的时间戳信息
    if(isForFileName)
    {
        strftime(timeStamp, timeStampLength, "%Y%m%d%H%M%S", &time);
    }
    // 给单条日志使用的时间戳信息
    else 
    {
        snprintf(timeStamp, timeStampLength, 
                "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", 
                time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                time.tm_hour, time.tm_min, time.tm_sec, tb.millitm);
    }
}

void CAsyncLog::makeLinePrefix(LOG_LEVEL level, std::string& strPrefix)
{
    // 添加当前日志级别信息
    strPrefix = "[INFO]";
    if (level == LOG_LEVEL_TRACE)
        strPrefix = "[TRACE]";
    else if (level == LOG_LEVEL_DEBUG)
        strPrefix = "[DEBUG]";
    else if (level == LOG_LEVEL_WARNING)
        strPrefix = "[WARN]";
    else if (level == LOG_LEVEL_ERROR)
        strPrefix = "[ERROR]";
    else if (level == LOG_LEVEL_SYSERROR)
        strPrefix = "[SYSE]";
    else if (level == LOG_LEVEL_FATAL)
        strPrefix = "[FATAL]";
    else if (level == LOG_LEVEL_CRITICAL)
        strPrefix = "[CRITICAL]";

    // 添加时间信息
    char timeStamp[64] = {0};
    getTime(timeStamp, sizeof(timeStamp));
    strPrefix += timeStamp;

    // 添加当前线程 ID 信息
    char threadID[32] = {0};
    std::ostringstream os;
    os << std::this_thread::get_id();
    snprintf(threadID, sizeof(threadID), "[%s]", os.str().c_str());
    strPrefix += threadID;
}

bool CAsyncLog::output(LOG_LEVEL level, const char* format, ...)
{
    if(level != LOG_LEVEL_CRITICAL)
    {
        if(level < m_currentLevel)
            return false;
    }

    // 添加日志级别信息和打印日志的时间信息
    std::string strLine;
    makeLinePrefix(level, strLine);

    // 获取可变参数格式化后字符串的长度
    va_list ap;
    va_start(ap, format);
    int logMsgLength = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    // 容量需要算上 vsnprintf 函数自动添加的一个 \0
    char logMsg[logMsgLength + 1];
    va_list aq;
    va_start(aq, format);
    vsnprintf(logMsg, sizeof(logMsg), format, aq);
    va_end(aq);

    // 如果日志开启截断，长日志只取前 MAX_LINE_LENGTH 个字符
    if(m_truncateLongLog && logMsgLength > MAX_LINE_LENGTH)
        logMsg[MAX_LINE_LENGTH] = 0;
    
    strLine += logMsg;

    // 在末尾加一个换行符
    strLine += '\n';
    
    // 非 FATAL 级日志放入异步消息队列让写磁盘工作线程去处理
    if(level != LOG_LEVEL_FATAL)
    {   
        m_logMsgQueue.movePush(std::move(strLine));
    }
    // FATAL 级日志采取同步写日志方法，并在写完日志后立即 crash 程序
    else
    {
        if(m_toConsole) std::cout << strLine << std::endl;

        if(!m_logFileName.empty())
        {   
            if(m_logFilePtr == nullptr)
            {
                // 生成新文件名
                char timeStamp[64];
                getTime(timeStamp, sizeof(timeStamp), true);
                std::string newLogFileName(m_logFileName);
                newLogFileName += ".";
                newLogFileName += timeStamp;
                newLogFileName += ".";
                newLogFileName += m_processPID;
                newLogFileName += ".log";

                // 创建新的日志文件
                if(!createNewFile(newLogFileName.c_str()))
                {   
                    std::cerr << "create new file fail !" << std::endl;
                    return false;
                }
            }
            writeToFile(strLine);
        }
        // 主动 crash 整个程序
        crash();
    }

    return true;
}

bool CAsyncLog::output(LOG_LEVEL level, const char* sourceCodeFileName, int lineNumber, const char* format, ...)
{
    if(level != LOG_LEVEL_CRITICAL)
    {
        if(level < m_currentLevel)
            return false;
    }

    // 添加日志级别信息和打印日志的时间信息
    std::string strLine;
    makeLinePrefix(level, strLine);

    // 添加打印日志时的位置信息
    char position[512] = {0};
    snprintf(position, sizeof(position), "[%s:%d]", sourceCodeFileName, lineNumber);
    strLine += position;

    // 获取可变参数格式化后字符串的长度
    va_list ap;
    va_start(ap, format);
    int logMsgLength = vsnprintf(NULL, 0, format, ap);
    va_end(ap);

    // 容量需要算上 vsnprintf 函数自动添加的一个 \0
    char logMsg[logMsgLength + 2];
    va_list aq;
    va_start(aq, format);
    vsnprintf(logMsg, sizeof(logMsg), format, aq);
    va_end(aq);

    // 如果日志开启截断，长日志只取前 MAX_LINE_LENGTH 个字符
    // TODO: bug 修复
    if(m_truncateLongLog && logMsgLength > MAX_LINE_LENGTH)
        logMsg[MAX_LINE_LENGTH] = 0;
    
    logMsg[logMsgLength] = '\n';
    logMsg[logMsgLength + 1] = 0;
    strLine += logMsg;
    
    // 非 FATAL 级日志放入异步消息队列让写磁盘工作线程去处理
    if(level != LOG_LEVEL_FATAL)
    {   
        m_logMsgQueue.movePush(std::move(strLine));
    }
    // FATAL 级日志采取同步写日志方法，并在写完日志后立即 crash 程序
    else
    {
        if(m_toConsole) std::cout << strLine << std::endl;

        if(!m_logFileName.empty())
        {   
            if(m_logFilePtr == nullptr)
            {
                // 生成新文件名
                char timeStamp[64];
                getTime(timeStamp, sizeof(timeStamp), true);
                std::string newLogFileName(m_logFileName);
                newLogFileName += ".";
                newLogFileName += timeStamp;
                newLogFileName += ".";
                newLogFileName += m_processPID;
                newLogFileName += ".log";

                // 创建新的日志文件
                if(!createNewFile(newLogFileName.c_str()))
                {   
                    std::cerr << "create new file fail !" << std::endl;
                    return false;
                }
            }
            writeToFile(strLine);
        }
        // 主动 crash 整个程序
        crash();
    }

    return true;
}

bool CAsyncLog::createNewFile(const char* logFileName)
{
    if(m_logFilePtr != nullptr)
    {
        fclose(m_logFilePtr);
    }

    m_logFilePtr = fopen(logFileName, "w+");
    return m_logFilePtr != nullptr;
}

bool CAsyncLog::writeToFile(const std::string& strLine)
{   
    uint32_t alreadyWrite = 0;
    uint32_t toWrite = strLine.length(); 

    // 为了防止长文件一次性写不完，放在一个循环里面分批写
    while(alreadyWrite < toWrite)
    {
        size_t writeSize = fwrite(strLine.c_str() + alreadyWrite, 1, toWrite - alreadyWrite, m_logFilePtr);
        if(writeSize <= 0)
            return false;

        alreadyWrite += writeSize; 
    }

    fflush(m_logFilePtr);
    return true;
}

void CAsyncLog::crash()
{
    char *p = nullptr;
    *p = 0;
}

void CAsyncLog::writeLogTask()
{
    m_running = true;   

    while(true)
    {
        // 如果需要写入日志文件
        if(!m_logFileName.empty())
        {   
            // 第一次写日志或日志文件大小超过了 rollSize 均建立新的文件
            if(m_logFilePtr == nullptr || m_currentWrittenSize >= m_fileRollSize)
            {
                // 重置写入日志文件的大小
                m_currentWrittenSize = 0;

                // 生成新文件名
                char timeStamp[64];
                getTime(timeStamp, sizeof(timeStamp), true);
                std::string newLogFileName(m_logFileName);
                newLogFileName += ".";
                newLogFileName += timeStamp;
                newLogFileName += ".";
                newLogFileName += m_processPID;
                newLogFileName += ".log";

                // 创建新的日志文件
                if(!createNewFile(newLogFileName.c_str()))
                {   
                    if(m_logFilePtr != nullptr) 
                        fclose(m_logFilePtr);

                    std::cerr << "create new file fail!" << std::endl;
                    break;
                }
            }
        }

        // 从消息队列中取出日志信息
        std::string strLine;
        m_logMsgQueue.waitAndPop(strLine);

        // 这样结束工作线程是存在一定问题的
        // 如果多个业务线程不停的快速向消息队列中写日志
        // 可能会导致一直无法结束写日志线程
        if(m_exit && m_logMsgQueue.empty()) break;

        // 下面的写法会直接结束写日志线程，无论消息队列中是否有数据
        // if(m_exit) break;

        // 打印到控制台
        if(m_toConsole) std::cout << strLine << std::endl;

        // 打印到日志文件
        if(!m_logFileName.empty())
        {
            if(!writeToFile(strLine))
            {
                fclose(m_logFilePtr);
                std::cerr << "write log message to file fail!" << std::endl;
                break;
            }

            m_currentWrittenSize += strLine.size();
        }
    }

    m_running = false;
}

}  // namespace mrpc