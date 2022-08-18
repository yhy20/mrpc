#include <stdio.h>

#include "LogFile.h"
#include "TimeStamp.h"
#include "AsyncLogging.h"

namespace mrpc
{
    
AsyncLogging::AsyncLogging(StringArg basename,
                           off_t rollSize,
                           int flushInterval /* = 3 */)
    : m_basename(basename.c_str()),
      m_rollSize(rollSize),
      m_flushInterval(flushInterval),
      m_mutex(),
      m_bufferFull(m_mutex),
      m_latch(1),
      m_currentBuffer(new Buffer),
      m_nextBuffer(new Buffer),
      m_buffers(),
      m_running(false),
      m_thread(std::bind(&AsyncLogging::ThreadTask, this), "AsyncLogging")
{
    m_currentBuffer->bzero();
    m_nextBuffer->bzero();
    m_buffers.reserve(16);
}

void AsyncLogging::append(const char* logLine, int len)
{
    LockGuard<Mutex> lock(m_mutex);
    /// 缓冲区未满，将本条日志添加进入缓冲区
    if(m_currentBuffer->available() > len)
    {
        m_currentBuffer->append(logLine, len);
    }
    /// 当前缓冲取已满
    else
    {
        /// 将写满的缓冲区放入缓冲队列
        m_buffers.push_back(std::move(m_currentBuffer));

        /// 将预备缓冲取移动给当前缓冲区
        if(m_nextBuffer)
        {
            m_currentBuffer = std::move(m_nextBuffer);
        }
        /// 若前端写入数据非常快，将两块缓冲区都写满，则分配一块新的 buffer，这是很少发生的情况
        else
        {
            m_currentBuffer.reset(new Buffer);
        }

        /// 将日志添加进入缓冲区
        m_currentBuffer->append(logLine, len);

        /// 通知后端线程将已满的缓冲区写到日志文件
        m_bufferFull.notify();
    }
}

/// 写日志异步线程任务
void AsyncLogging::ThreadTask()
{
    assert(m_running == true);
    m_latch.countDown();
    
    /// 日志文件
    LogFile output(m_basename, m_rollSize, false);

    /**
     * newBuffer1 与 newBuffer2 分别是 m_currentBuffer 和 m_nextBuffer 的预备缓冲区
     * 由于缓冲区的替换操作都发生在临界区内部，预先分配 newBuffer1 与 newBuffer2 对象
     * 可以在替换时直接 move 替换，防止在临界区内部即时创建缓冲区对象，尽可能减少临界区
     * 代码的运行时间，提高代码并发性，提高程序性能。
     */
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    /// 创建缓冲区队列并预留空间
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);

    while(m_running)
    {
        /// 断言判断每轮循环开始时缓冲区与缓冲队列的初始状态
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            LockGuard<Mutex> lock(m_mutex);
            /// 缓冲队列无数据，说明当前缓冲区未写满，等待 3 秒
            if(m_buffers.empty())   
            {   
                /// 等待过程中会被缓冲区已满信号唤醒
                m_bufferFull.waitForSeconds(m_flushInterval);
            }

            /// 将当前缓冲区加入缓冲队列，若条件变量被唤醒则缓冲区已写满，若条件变量超时则缓冲区未写满
            m_buffers.push_back(std::move(m_currentBuffer));

            /// 前后端双缓队列冲交换
            buffersToWrite.swap(m_buffers);

            /// 用 newBuffer1 与 newBuffer2 分别替换 m_currentBuffer 和 m_nextBuffer
            m_currentBuffer = std::move(newBuffer1);
            if(!m_nextBuffer)
            {
                m_nextBuffer = std::move(newBuffer2);
            }
        }

        /// 出临界区后，buffersToWrite 缓冲队列理论上非空
        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                    TimeStamp::Now().toFormattedString().c_str(),
                    buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        /// 将缓冲队列数据写入日志文件
        for (const auto& buffer : buffersToWrite)
        {   
             /**
             * 关于日志文件是否使用二级缓冲区的讨论
             * 
             * 
             */

            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }

        /// 丢弃缓冲队列中多余的部分
        if (buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        /// 重置 newBuffer1
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        /// 重置 newBuffer2
        if(!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }

    output.flush();
}

}  // namespace mrpc





