#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>

#include <sstream>

#include "Logging.h"
#include "TimeZone.h"
#include "TimeStamp.h"
#include "CurrentThread.h"

#include <iostream>

namespace mrpc
{

/// 线程局部变量，存放线程发生错误时，从 errno 获取的错误信息
thread_local char t_errnobuf[512];  
/// 线程局部变量，存放线程以秒为间隔的时间戳字符串
thread_local char t_time[64];
/// 线程局部变量，存放上一次生成时间戳字符串 t_time 的秒时间
thread_local time_t t_lastSecond;

const char* Strerror_tl(int savedErrno)
{
    /// strerror_r 使用线程局部数组 t_errnobuf 存储错误信息，是线程安全的
    return strerror_r(savedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel InitLogLevel()
{
    if(::getenv("MRPC_LOG_TRACE"))
        return Logger::TRACE;
    else if(::getenv("MRPC_LOG_DEBUG"))
        return Logger::DEBUG;
    else 
        return Logger::INFO;
}

Logger::LogLevel g_logLevel = InitLogLevel();

const char* LogLevelName[static_cast<int>(Logger::SYSFA) + 1] = 
{
    "[TRACE]",
    "[DEBUG]",
    "[INFO ]",
    "[WARN ]",
    "[ERROR]",
    "[SYSER]",
    "[FATAL]",
    "[SYSFA]"
};

/**
 * @brief helper class for known string length at compile time
 */
class T
{
public:
    T(const char* str, unsigned len)
        : m_str(str),
          m_len(len)
    {
        assert(strlen(str) == m_len);
    }

    const char* m_str;
    const unsigned m_len;
};

inline LogStream& operator<<(LogStream& s, T v)
{
    s.append(v.m_str, v.m_len);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.append(v.m_data, v.m_size);
    return s;
}

/**
 * @brief 默认到标准输出
 */
void DefaultOutput(const char* msg, int len)
{   
    ///TODO: 关于 fwrite 行缓冲的问题
    // write(STDOUT_FILENO, msg, len);
    size_t n = fwrite(msg, 1, len, stdout);
    // std::cout << msg;
    /// FIXME check n

    (void)n;
}

void DefaultFlush()
{
    fflush(stdout);
}

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;
TimeZone g_logTimeZone;

Logger::Impl::Impl(LogLevel level, int savedErrno, 
                   const SourceFile& file, int line)
    : m_level(level),
      m_basename(file),
      m_line(line),
      m_time(TimeStamp::Now()),
      m_stream()
{
    /// LogLevel 信息
    m_stream << T(LogLevelName[level], 7);

    /// 设定时区的时间戳信息
    formatTime();

    /// 线程 id 信息
    CurrentThread::Tid();
    m_stream << T(CurrentThread::TidString(), CurrentThread::TidStringLength());
    
    /// 源文件、行号信息
    m_stream << '[' << m_basename << ':' << m_line << ']';

    /// errno 信息
    if(savedErrno != 0)
    {
        m_stream << "[errno=" << savedErrno << ":errno message="
                 << Strerror_tl(savedErrno) << ']';
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondsSinceEpoch = m_time.microSecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / TimeStamp::s_microSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch % TimeStamp::s_microSecondsPerSecond);

    /// 以秒为时间间隔生成时间戳字符串，减少时间戳计算的次数
    if(seconds != t_lastSecond)
    {
        t_lastSecond = seconds;
        struct tm tm_time;
        if(g_logTimeZone.valid())
        {
            tm_time = g_logTimeZone.toLocalTime(seconds);
        }
        else
        {
            // 原作者这里不一样
            ::gmtime_r(&seconds, &tm_time);
        }
        int len = snprintf(t_time, sizeof(t_time), "[%4d-%02d-%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
            tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 20);
        (void)len;
    }

    if(g_logTimeZone.valid())
    {
        Fmt us(":%06d]", microseconds);

        assert(us.length() == 8);
        m_stream << T(t_time, 20) << T(us.data(), 8);
    }
    else
    {
        Fmt us(":%06dZ]", microseconds);

        assert(us.length() == 9);
        m_stream << T(t_time, 20) << T(us.data(), 9);
    }
}

void Logger::Impl::format(const char* fmt, va_list ap)
{
    /// TODO: vsnprintf 性能下降问题
    LogStream::Buffer& buf = const_cast<LogStream::Buffer&>(m_stream.buffer());
    int len = vsnprintf(buf.current(), buf.available(), fmt, ap);
    buf.add(len);
    /// FIXEME: check len
    (void)len;
}

void Logger::Impl::finish()
{
    m_stream << "\n";
}

Logger::Logger(SourceFile file, int line)
    : m_impl(INFO, 0, file, line) { }

Logger::Logger(SourceFile file, int line, LogLevel level)
    : m_impl(level, 0, file, line) { }

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : m_impl(level, 0, file, line)
{
    m_impl.m_stream << '[' << func << ']';
}

Logger::~Logger()
{
    m_impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());

    // FATAL 级别日志，终止进程
    if(m_impl.m_level == FATAL || m_impl.m_level == SYSFA)
    {
        g_flush();
        abort();
    }
}

void Logger::format(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    m_impl.format(fmt, ap);
    va_end(ap);
}

void Logger::SetLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::SetTimeZone(const TimeZone& tz)
{
    g_logTimeZone = tz;
}

void Logger::SetOutput(OutputFunc out)
{
    g_output = out;
}

}  // namespace mrpc

