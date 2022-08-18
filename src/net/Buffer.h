#ifndef __MRPC_NET_BUFFER_H__
#define __MRPC_NET_BUFFER_H__

#include <assert.h>
#include <string.h>

#include <vector>
#include <algorithm>

#include "Types.h"
#include "Endian.h"
#include "copyable.h"
#include "StringPiece.h"

namespace mrpc
{
namespace net
{
/**
 * 下面是 Buffer class 的布局
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 */

/**
 * @brief TcpConnection Input, Output Buffer
 * @details 以 org.jboss.netty.buffer.ChannelBuffer 为原型
 */
class Buffer : public copyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : m_buffer(kCheapPrepend + initialSize),
          m_readerIndex(kCheapPrepend),
          m_writerIndex(kCheapPrepend)
    {
    
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    /// implicit copy-ctor, move-ctor, assignment and dtor are fine
    /// NOTE: implicit move-ctor is added in g++ 4.6

public:
    void swap(Buffer& rhs)
    {
        m_buffer.swap(rhs.m_buffer);
        std::swap(m_readerIndex, rhs.m_readerIndex);
        std::swap(m_writerIndex, rhs.m_writerIndex);
    }

    size_t readableBytes() const { return m_writerIndex - m_readerIndex; }
    size_t writableBytes() const { return m_buffer.size() - m_writerIndex; }
    size_t prependableBytes() const { return m_readerIndex; }

    const char* readBegin() const { return begin() + m_readerIndex; }
    const char* writeBegin() const { return begin() + m_writerIndex; }
    char* writeBegin() { return begin() + m_writerIndex; }

    void addWriteSize(size_t len)
    {
        assert(len <= writableBytes());
        m_writerIndex += len;
    }

    void subWriteSize(size_t len)
    {
        assert(len <= readableBytes());
        m_writerIndex -= len;
    }

    const char* findCRLF() const
    {
        const char* crlf = std::search(readBegin(), writeBegin(), kCRLF, kCRLF + 2);
        return crlf == writeBegin() ? nullptr : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(readBegin() <= start);
        assert(start <= writeBegin());
        const char* crlf = std::search(start, writeBegin(), kCRLF, kCRLF + 2);
        return crlf == writeBegin() ? nullptr : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = ::memchr(readBegin(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    const char* findEOL(const char* start)
    {
        assert(readBegin() <= start);
        assert(start <= writeBegin());
        const void* eol = ::memchr(start, '\n', writeBegin() - start);
        return static_cast<const char*>(eol);
    }

    ssize_t readFd(int fd, int* savedErrno);

    /**
     * @brief 返回主机序表示的 int64_t 数据，但不会改变 readableBytes.
     *        要求buf->readableBytes() >= sizeof(int64_t)
     */
    int64_t peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, readBegin(), sizeof(be64));
        return sockets::NetworkToHost64(be64);
    }

    /**
     * @brief 返回主机序表示的 int32_t 数据，但不会改变 readableBytes.
     *        要求buf->readableBytes() >= sizeof(int32_t)
     */
    int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, readBegin(), sizeof(be32));
        return sockets::NetworkToHost32(be32);
    }

    /**
     * @brief 返回主机序表示的 int16_t 数据，但不会改变 readableBytes.
     *        要求buf->readableBytes() >= sizeof(int16_t)
     */
    int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, readBegin(), sizeof(be16));
        return sockets::NetworkToHost16(be16);
    }

    /**
     * @brief 返回主机序表示的 int8_t 数据，但不会改变 readableBytes.
     *        要求buf->readableBytes() >= sizeof(int8_t)
     */
    int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *readBegin();
        return x;
    }

    int64_t readInt64()
    {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    int64_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if(len < readableBytes())
        {
            m_readerIndex += len;
        }
        else 
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* pos)
    {
        assert(readBegin() <= pos);
        assert(pos <= writeBegin());
        retrieve(pos - readBegin());
    }

    void retrieveAll()
    {
        m_readerIndex = kCheapPrepend;
        m_writerIndex = kCheapPrepend;
    }

    void retrieveInt64()
    {
        retrieve(sizeof(int64_t));
    }

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    } 

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(readBegin(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    StringPiece toStringPiece() const
    {
        return StringPiece(readBegin(), static_cast<int>(readableBytes()));
    }

    void ensureWritableBytes(size_t len)
    {
        if(writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, writeBegin());
        addWriteSize(len);
    }

    void append(const void* data, size_t len)
    {
        append(static_cast<const char*>(data), len);
    }

    void append(const StringPiece& str)
    {
        append(str.data(), str.size());
    }

    /**
     * @brief 使用网络序添加 int64_t 数据
     */
    void appendInt64(int64_t x)
    {
        int64_t be64 = sockets::HostToNetwork64(x);
        append(&be64, sizeof(be64));
    }

    /**
     * @brief 使用网络序添加 int32_t 数据
     */
    void appendInt32(int32_t x)
    {
        int32_t be32 = sockets::HostToNetwork32(x);
        append(&be32, sizeof(be32));
    }
    
    /**
     * @brief 使用网络序添加 int16_t 数据
     */
    void appendInt16(int16_t x)
    {
        int16_t be16 = sockets::HostToNetwork16(x);
        append(&be16, sizeof(be16));

    }

    /**
     * @brief 添加 int8_t 数据，无字节序问题
     */
    void appendInt8(int8_t x)
    {
        append(&x, sizeof(x));
    }

    void prepend(const void* data, size_t len)
    {
        assert(len <= prependableBytes());
        m_readerIndex -= len;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + len, begin() + m_readerIndex);
    }

    void prependInt64(int64_t x)
    {
        int64_t be64 = sockets::HostToNetwork64(x);
        prepend(&be64, sizeof(be64));
    }

    void prependInt32(int32_t x)
    {
        int32_t be32 = sockets::HostToNetwork32(x);
        prepend(&be32, sizeof(be32));
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = sockets::HostToNetwork16(x);
        prepend(&be16, sizeof(be16));
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof(x));
    }

    void shrink(size_t reserve)
    {
        Buffer other;
        other.ensureWritableBytes(readableBytes()+reserve);
        other.append(toStringPiece());
        swap(other);
    }

    size_t internalCapacity() const
    {
        return m_buffer.capacity();
    }

private:
    char* begin() { return &*m_buffer.begin(); }
    const char* begin() const { return &*m_buffer.begin(); }
    void makeSpace(size_t len)
    {
        /// 若总空闲空间不足，则 resize, 若 resize 的大小不超过 capacity, 则不会重新分配空间
        if(writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            m_buffer.resize(m_writerIndex + len);
        }
        /// 若总空闲空间足够，则挪动可以读取的数据数据，腾出空间
        else
        {
            assert(kCheapPrepend < m_readerIndex);
            size_t readable = readableBytes();
            std::copy(begin() + m_readerIndex,
                      begin() + m_writerIndex,
                      begin() + kCheapPrepend);
            m_readerIndex = kCheapPrepend;
            m_writerIndex = m_readerIndex + readable;
            assert(readable = readableBytes());
        }
    }

private:
    std::vector<char>   m_buffer;
    size_t              m_readerIndex;
    size_t              m_writerIndex;

    static const char   kCRLF[];
};

}  // namespace net
}  // namespace mrpc

#endif  // __MRPC_NET_BUFFER_H__