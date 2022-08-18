#ifndef __MRPC_BASE_FILEUTIL_H__
#define __MRPC_BASE_FILEUTIL_H__

#include "sys/types.h"

#include "noncopyable.h"
#include "StringPiece.h"

namespace mrpc
{
namespace FileUtil
{

/**
 * @brief 读取小容量文件（< 64KB）
 */
class ReadSmallFile : noncopyable
{
public:
    /**
     * @brief 构造函数
     * @param[in] fileName 文件名
     */
    ReadSmallFile(StringArg fileName);
    
    /**
     * @brief 析构函数，关闭文件句柄
     */
    ~ReadSmallFile();

    /**
     * @brief 
     * @param[in] 
     * @param[in] 
     * @param[in] 
     */
    template <typename String>
    int readToString(int maxSize, 
                     String* content,
                     int64_t* fileSize,
                     int64_t* modifyTime,
                     int64_t* createTime);
    
    int readToBuffer(int* size);
    const char* buffer() const { return m_buf; }

    /// 缓冲区大小
    static const int s_bufferSize = 64 * 1024;

private:
    int     m_fd;                   // 文件句柄
    int     m_errno;                // 错误码
    char    m_buf[s_bufferSize];    // 数据缓冲区
};

/**
 * @brief read the file content, returns errno if error happens.
 */
template<typename String>
int ReadFile(StringArg fileName,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
    ReadSmallFile file(fileName);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

/**
 * @brief 写磁盘 I/O 文件，非线程安全
 */
class AppendFile : noncopyable
{
public:
    /**
     * @brief 构造函数
     * @param[in] fileName 文件名
     */
    explicit AppendFile(StringArg fileName);

    /**
     * @brief 析构函数
     */
    ~AppendFile();

    /**
     * @brief 将 len 长度的 logLine 添加到日志文件
     */
    void append(const char* logLine, size_t len);

    /**
     * @brief 冲洗文件
     */
    void flush();

    /**
     * @brief 返回已写入的字节数
     */
    off_t writtenBytes() const { return m_writtenBytes; }

private:
    /**
     * @brief 不加锁，将缓存数据写入文件流
     */
    size_t write(const char* logLine, size_t len);

private:
    FILE*   m_fp;                   // 文件 I/O 流
    char    m_buffer[64 * 1024];    // 文件 I/O 缓冲区
    off_t   m_writtenBytes;         // 已写入的字节数
};

}  // namespace FileUtil
}  // namespace mrpc

#endif  // __MRPC_BASE_FILEUTIL_H__