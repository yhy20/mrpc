#include <time.h>
#include <stdio.h>
#include <assert.h>

#include "LogFile.h"
#include "FileUtil.h"
#include "ProcessInfo.h"

namespace mrpc
{

LogFile::LogFile(StringArg basename,
                 off_t rollSize,
                 bool threadSafe /* = true */,
                 int flushInterval /* = 3 */,
                 int checkEveryN /* = 1024 */)
    : m_basename(basename.c_str()),
      m_rollSize(rollSize),
      m_flushInterval(flushInterval),
      m_checkEveryN(checkEveryN),
      m_count(0),
      m_startOfPeriod(0),
      m_lastRoll(0),
      m_lastFlush(0),
      m_mutex(threadSafe ? new Mutex : nullptr)
{
    assert(m_basename.find('/') == std::string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

/// 根据 threadSafe 设置情况，确定是否加锁并将 len 长度的 logLine 添加到日志文件 
void LogFile::append(const char* logLine, int len)
{
    if(m_mutex)
    {
        LockGuard<Mutex> lock(*m_mutex);
        append_unlocked(logLine, len);
    }
    else
    {
        append_unlocked(logLine, len);
    }
}

void LogFile::flush()
{
    if(m_mutex)
    {
        LockGuard<Mutex> lock(*m_mutex);
        m_file->flush();
    }
    else
    {
        m_file->flush();
    }
}

/// 不加锁，将 len 长度的 logLine 添加到日志文件 
void LogFile::append_unlocked(const char* logLine, int len)
{
    m_file->append(logLine, len);

    /// 文件大小超过 rollSize，则滚动日志文件
    if(m_file->writtenBytes() > m_rollSize)
    {
        rollFile();
    }
    else
    {
        /// 每写入 1024 条日志就 check 一次状态
        ++m_count;
        if(m_count >= m_checkEveryN)
        {   
            /// 重置记录次数
            m_count = 0;
            time_t now = ::time(nullptr);
            time_t thisPeriod = now / s_rollPerSeconds * s_rollPerSeconds;
            /// 时间超过当天，则滚动日志文件
            if(thisPeriod != m_startOfPeriod)
            {
                rollFile();
            }
            /// 每超过 3 秒刷新日志缓存
            else if(now - m_lastFlush > m_flushInterval)
            {
                m_lastFlush = now;
                m_file->flush();
            }
        }
    }
}

/// 滚动日志
bool LogFile::rollFile()
{
    time_t now = 0;
    std::string fileName = GetLogFileName(m_basename, &now);

    /// 获取当天零点的时间
    time_t start = now / s_rollPerSeconds * s_rollPerSeconds;

    if(now > m_lastRoll)
    {
        m_lastRoll = now;
        m_lastFlush = now;
        m_startOfPeriod = start;
        m_file.reset(new FileUtil::AppendFile(fileName));
        return true;
    }

    return false;
}

/// 生成日志文件全称
std::string LogFile::GetLogFileName(const std::string& basename, time_t* now)
{   
    std::string fileName;
    fileName.reserve(basename.size() + 64);
    /// 基础日志名
    fileName = basename;

    /// 添加时间戳
    char timebuf[32];
    struct tm tm;
    *now = time(nullptr);
    localtime_r(now, &tm);
    strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
    fileName += timebuf;

    /// 添加主机名
    //fileName += ProcessInfo::HostName();

    /// 添加进程 ID
    char pidbuf[32];
    snprintf(pidbuf, sizeof(pidbuf), ".%d", ProcessInfo::Pid());
    fileName += pidbuf;

    /// 添加后缀
    fileName += ".log";

    return fileName;
}

}  // namespace mrpc