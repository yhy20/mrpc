#ifndef __MRPC_BASE_ASYNCLOGGING_H__
#define __MRPC_BASE_ASYNCLOGGING_H__

#include <atomic>
#include <vector>

#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"
#include "StringPiece.h"
#include "BlockingQueue.h"
#include "CountDownLatch.h"
#include "BoundedBlockingQueue.h"

namespace mrpc
{
/**
 * @brief 异步日志类
 */
class AsyncLogging : noncopyable
{
public:
    /**
     * @brief 异步日志后端构造函数
     * @param[in] basename 日志名
     * @param[in] rollSize 滚动大小
     * @param[in] flushInterval 冲洗时间间隔
     */
    AsyncLogging(StringArg basename,
                 off_t rollSize,
                 int flushInterval = 3);

    /**
     * @biref 析构函数
     */
    ~AsyncLogging()
    {
        if(m_running)
        {
            stop();
        }
    }

    /**
     * @brief 将 len 长度的 logLine 添加到日志文件
     */
    void append(const char* logLine, int len);

    /**
     * @brief 启动异步日志
     */
    void start()
    {
        /// 写日志循环正常工作
        m_running = true;
        /// 启动日志线程
        m_thread.start();
        /// 等待日志线程启动完毕
        m_latch.wait();
    }

    /**
     * @brief 结束异步日志
     */
    void stop()
    {   
        /// 停止写日志循环，m_running = false 后写入的数据都丢失
        m_running = false;
        /// 直接唤醒等待的条件变量
        m_bufferFull.notify();
        /// 
        m_thread.join();
    }

private:
    /**
     * @brief 写日志异步线程任务
     */
    void ThreadTask();

private:
    typedef details::FixedBuffer<details::kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const std::string   m_basename;         // 基本日志文件名
    const off_t         m_rollSize;         // 日志文件大小超过 m_rollSize，则滚动日志
    const int           m_flushInterval;    // 刷新间隔时间，在 flushInterval 秒内缓冲区没写满，仍将缓冲区的数据写到日志文件

    Mutex               m_mutex;            // 互斥锁
    Condition<Mutex>    m_bufferFull;       // 等待缓冲区满或超时的条件变量
    CountDownLatch      m_latch;            // 到计数器，用于等待日志线程启动完毕

    BufferPtr           m_currentBuffer;    // 当前缓冲区
    BufferPtr           m_nextBuffer;       // 预备缓冲区
    BufferVector        m_buffers;          // 缓冲取队列

    std::atomic<bool>   m_running;          // 日志线程是否运行
    Thread              m_thread;           // 日志线程
};

}  // namespace mrpc

#endif  // __MRPC_BASE_ASYNCLOGGING_H__