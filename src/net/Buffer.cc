#include <errno.h>
#include <sys/uio.h>

#include "Buffer.h"
#include "SocketsOps.h"

namespace mrpc
{
namespace net
{

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extraBuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + m_writerIndex;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    /// 如果 buffer 有足够的空闲空间（writable >= 65536), 则不会将数据读到 extraBuf 栈上缓冲
    /// 如果使用了 extraBuf，则最多会读取 128k - 1 个字节（writable = 65535 and extraBuf = 65536)
    const int iovcnt = (writable < sizeof(extraBuf) ? 2 : 1);
    const ssize_t n = sockets::Readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *savedErrno = errno;
    }
    else if(implicit_cast<size_t>(n) <= writable)
    {
        m_writerIndex += n;
    }
    else
    {
        m_writerIndex = m_buffer.size();
        append(extraBuf, n - writable);
    }
    // if (n == writable + sizeof extraBuf)
    // {
    //   goto line_30;
    // }

    /**
     * 疑问 ？？？
     * socket fd 文件描述符对应的内核态缓冲区可能缓存了收到的多个 TCP 数据包
     * 内核态缓冲区内存储的数据完全有可能超过了 128k - 1 个字节，该问题如何处理
     * 数据处理足够快可以防止该问题的发生
     */

    return n;
}


}  // namespace net
}  // namespace mrpc


