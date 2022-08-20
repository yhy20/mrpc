#ifndef __MRPC_BASE_LOGGING_H__
#define __MRPC_BASE_LOGGING_H__

#include "LogStream.h"
#include "TimeStamp.h"

namespace mrpc
{

class TimeZone;
/**
 * @brief 日志前端类
 */
class Logger
{
public:
    /// TODO: LogLeveltoString()
    /// TODO: StringtoLogLevel()
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        SYSER,
        FATAL,
        SYSFA
    };
    
    /**
     * @brief compile time calculation of basename of source file
     */
    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char (&str)[N])
            : m_data(str),
              m_size(N - 1)
        {
/**
 * USE_FULL_FILENAME 使用 __FILE__ 的绝对路径名（默认使用 basename）
 * 日志打印出绝对路径名可以方便的使用 vscode 的快速跳转功能至文件功能
 */
#ifdef USE_FULL_FILENAME
            /// do nothing
#else
            const char* slash = strrchr(m_data, '/');
            if(slash)
            {
                m_data = slash + 1;
                m_size -= static_cast<int>(m_data - str);
            }
#endif 
        }
    
        explicit SourceFile(const char* fileName)
            : m_data(fileName)
        {
#ifdef USE_FULL_FILENAME
            m_size = static_cast<int>(strlen(m_data));
#else
            const char* slash = strrchr(fileName, '/');
            if(slash)
            {
                m_data = slash + 1;
            }
            m_size = static_cast<int>(strlen(m_data));
#endif
        }

        const char* m_data;
        int m_size;
    };

    /**
     * @brief 构造函数，默认日志级别 INFO
     */
    Logger(SourceFile file, int line);

    /**
     * @brief 构造函数，自定义日志级别
     */
    Logger(SourceFile file, int line, LogLevel level);

    /**
     * @brief 
     */
    Logger(SourceFile file, int line, LogLevel level, const char* func);

    // /**
    //  * @brief 错误日志，决定是否 toAbort
    //  */
    // Logger(SourceFile file, int line, LogLevel level);

    ~Logger();

    LogStream& stream() { return m_impl.m_stream; }

    void format(const char* fmt, ...);

    static LogLevel GetLogLevel();
    
    /**
     * @brief 
     */
    static void SetLogLevel(LogLevel level);
     /**
     * @brief 设置时区，非线程安全，必须在多线程打印日志前调用
     * @param[in] 时区
     */
    static void SetTimeZone(const TimeZone& tz);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();

    static void SetOutput(OutputFunc);
    static void SetFlush(FlushFunc);
   

private:
    class Impl
    {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();
        void format(const char* fmt, va_list al);
        
        LogLevel    m_level;
        SourceFile  m_basename;
        int         m_line;
        TimeStamp   m_time;
        LogStream   m_stream;
    };

    Impl m_impl;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::GetLogLevel()
{
    return g_logLevel;
}

/**
 * @brief 线程安全函数，获取 errno 对应的错误信息
 * @param[in] savedErrno errno 错误码保存的副本 
 * @details When using threads, errno is a per-thread value.
 *          所以 errno 本身是线程安全的，使用保存的副本，防止 errno 值被覆盖
 */
const char* Strerror_tl(int savedErrno);

template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr)
{
    if(ptr == nullptr)
    {
        Logger(file, line, Logger::FATAL).stream() << names;
    }
    return ptr;
}

#define CHECK_NOTNULL(val) \
    ::mrpc::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

}  // namespace mrpc

/// C++ 风格日志宏

#define LOG_TRACE if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::TRACE) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::DEBUG) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::INFO) \
    mrpc::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::WARN) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::WARN).stream()
#define LOG_ERROR if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::ERROR) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::ERROR).stream()
#define LOG_SYSERR if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::SYSER) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSER).stream()
#define LOG_FATAL mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::FATAL).stream()
#define LOG_SYSFATAL mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSFA).stream()

/// C 风格日志宏

#define CLOG_TRACE(fmt, ...) if(mrpc::Logger::GetLogLevel() <= mrpc::Logger::TRACE) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::TRACE, __func__).format(fmt, ##__VA_ARGS__)
#define CLOG_DEBUG(fmt, ...) if(mrpc::Logger::GetLogLevel() <= mrpc::Logger::DEBUG) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::DEBUG, __func__).format(fmt, ##__VA_ARGS__)
#define CLOG_INFO(fmt, ...)  if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::INFO) \
    mrpc::Logger(__FILE__, __LINE__).format(fmt, ##__VA_ARGS__)
#define CLOG_WARN(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::WARN) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::WARN).format(fmt, ##__VA_ARGS__)
#define CLOG_ERROR(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::ERROR) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::ERROR).format(fmt, ##__VA_ARGS__)
#define CLOG_SYSERR(fmt, ...) if (mrpc::Logger::GetLogLevel() <= mrpc::Logger::SYSER) \
    mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSER).format(fmt, ##__VA_ARGS__)
#define CLOG_FATAL(fmt, ...) mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::FATAL).format(fmt, ##__VA_ARGS__)
#define CLOG_SYSFATAL(fmt, ...) mrpc::Logger(__FILE__, __LINE__, mrpc::Logger::SYSFA).format(fmt, ##__VA_ARGS__)


#endif  // __MRPC_BASE_LOGGING_H__