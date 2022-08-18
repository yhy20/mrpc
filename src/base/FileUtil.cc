#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "FileUtil.h"
#include "Logging.h"

namespace mrpc
{
namespace FileUtil
{

ReadSmallFile::ReadSmallFile(StringArg fileName)
    : m_fd(::open(fileName.c_str(), O_RDONLY | O_CLOEXEC)),
      m_errno(0)
{
    m_buf[0] = '\0';
    if(-1 == m_fd)
    {
        m_errno = errno;
    }
}

ReadSmallFile::~ReadSmallFile()
{
    if(m_fd > 0)
    {
        ::close(m_fd);
    }
}

template <typename String>
int ReadSmallFile::readToString(int maxSize, 
                                String* content,
                                int64_t* fileSize,
                                int64_t* modifyTime,
                                int64_t* createTime)
{
    static_assert(sizeof(off_t) == 8, "FILE_OFFSET_BITS = 64");
    assert(content != nullptr);
    int err = m_errno;
    if(m_fd > 0)
    {
        content->clear();

        if(fileSize)
        {
            struct stat buf;
            if(0 == ::fstat(m_fd, &buf))
            {
                if(S_ISREG(buf.st_mode))
                {
                    *fileSize = buf.st_size;
                    content->reserve(static_cast<int>(std::min(static_cast<int64_t>(maxSize), *fileSize)));
                }
                else if(S_ISDIR(buf.st_mode))
                {
                    err = EISDIR;
                }
                if(modifyTime)
                {
                    *modifyTime = buf.st_mtime;
                }
                if(createTime)
                {
                    *createTime = buf.st_ctime;
                }
            }
            else
            {
                err = errno;
            }
        }

        while(content->size() < static_cast<size_t>(maxSize))
        {
            size_t toRead = std::min(static_cast<size_t>(maxSize) - content->size(), sizeof(m_buf));
            ssize_t n = ::read(m_fd, m_buf, toRead);
            if(n > 0)
            {
                content->append(m_buf, n);
            }
            else
            {
                if(n < 0)
                {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

int ReadSmallFile::readToBuffer(int * size)
{
    int err = m_errno;
    if(m_fd > 0)
    {
        ssize_t n = ::pread(m_fd, m_buf, sizeof(m_buf)-1, 0);
        if(n >= 0)
        {
            if(size)
            {
                *size = static_cast<int>(n);
            }
            m_buf[n] = '\0';
        }
        else
        {
            err = errno;
        }
    }
    return err;
}

template int ReadFile(StringArg filename,
                      int maxSize,
                      std::string* content,
                      int64_t* fileSize, 
                      int64_t* modifyTime, 
                      int64_t* createTime);

template int ReadSmallFile::readToString(int maxSize,
                                         std::string* content,
                                         int64_t* fileSize,
                                         int64_t* modifyTime,
                                         int64_t* createTime);

AppendFile::AppendFile(StringArg fileName)
    : m_fp(::fopen(fileName.c_str(), "ae")),  // 'e' for O_CLOEXEC
      m_writtenBytes(0)
{
    assert(m_fp);
    ::setbuffer(m_fp, m_buffer, sizeof(m_buffer));
}

AppendFile::~AppendFile()
{
    ::fclose(m_fp);
}

void AppendFile::append(const char* logLine, const size_t len)
{
    size_t written = 0; 

    while(written != len)
    {
        size_t remain = len - written;
        size_t n = write(logLine + written, remain);
        if(n != remain)
        {
            int err = ferror(m_fp);
            if(err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", Strerror_tl(err));
                break;
            }
        }
        written += n;
    }
    m_writtenBytes += written;
}

void AppendFile::flush()
{
    ::fflush(m_fp);
}

size_t AppendFile::write(const char* logLine , size_t len)
{
    return ::fwrite_unlocked(logLine, 1, len, m_fp);
}

}  // namespace FileUtil
}  // namespace mrpc
