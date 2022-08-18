#ifndef __MRPC_BASE_LOGFILE_H__
#define __MRPC_BASE_LOGFILE_H__

#include <memory>

#include "Mutex.h"
#include "StringPiece.h"

namespace mrpc
{
namespace FileUtil
{

class AppendFile;

}  // namespace FileUtil

/**
 * @brief 日志文件类
 */
class LogFile : noncopyable
{
public:
    /**
     * @brief 构造函数，创建一个日志文件
     * @param[in] basename 基础日志文件名
     * @param[in] rollSize 文件滚动大小
     * @param[in] threadSafe 是否线程安全写日志文件
     * @param[in] flushInterval 刷新缓冲区时间间隔
     * @param[in] checkEveryN 检查次数间隔
     */
    LogFile(StringArg basename,
            off_t rollSize,
            bool threadSafe = true,
            int flushInterval = 3,
            int checkEveryN = 1024);

    /**
     * 析构函数
     */
    ~LogFile();

    /**
     * @brief 根据 threadSafe 设置情况，确定是否加锁并将 len 长度的 logLine 添加到日志文件 
     */ 
    void append(const char* logLine, int len);

    /**
     * @brief 冲洗日志
     */
    void flush();
    
    /**
     * @brief 滚动文件
     */
    bool rollFile();

private:
    /**
     * @brief 不加锁，将 len 长度的 logLine 添加到日志文件 
     */
    void append_unlocked(const char* logLine, int len);
    /**
     * @brief 生成完整日志文件名
     */
    static std::string GetLogFileName(const std::string& basename, time_t* now);

private:
    const std::string       m_basename;         // 基础日志文件名
    const off_t             m_rollSize;         // 日志文件滚动大小
    const int               m_flushInterval;    // 刷新缓冲区时间间隔
    const int               m_checkEveryN;      // 日志文件状态检查间隔次数

    int                     m_count;            // 记录写日志次数
    time_t                  m_startOfPeriod;    // 当天零点时刻
    time_t                  m_lastRoll;         // 上一次滚动事件
    time_t                  m_lastFlush;        // 上一次冲洗时间
    std::unique_ptr<Mutex>  m_mutex;            // 互斥锁
    
    /// 日志文件
    std::unique_ptr<FileUtil::AppendFile>   m_file;    

    /// 一天的秒数，每天零点日志文件滚动一次
    const static int s_rollPerSeconds = 60 * 60 * 24;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_LOGFILE_H__