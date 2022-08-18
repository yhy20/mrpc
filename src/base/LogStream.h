#ifndef __MRPC_BASE_LOGSTREAM_H__
#define __MRPC_BASE_LOGSTREAM_H__

#include <assert.h>
#include <string.h>

#include "Types.h"
#include "noncopyable.h"
#include "StringPiece.h"

namespace mrpc
{
namespace details
{

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

/**
 * @brief 固定容量的缓冲类，在日志系统中作为一条日志的 buffer
 */
template <int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : m_cur(m_data) 
    {
        setCookie(cookieStart);
    }

    ~FixedBuffer()
    {
        setCookie(cookieEnd);
    }

    /**
     * @brief 向缓冲添加数据
     */
    void append(const char* buf, size_t len)
    {
        if(implicit_cast<size_t>(available()) > len)
        {
            memcpy(m_cur, buf, len);
            m_cur += len;
        }
    }

    const char* data() const { return m_data; }
    int length() const { return static_cast<int>(m_cur - m_data); }

    char* current() { return m_cur; }
    int available() const { return static_cast<int>(end() - m_cur); }
    void add(size_t len) { m_cur += len; }

    void reset() { m_cur = m_data; }
    void bzero() { memZero(m_data, sizeof(m_data)); }

    /**
     * @brief for used by GDB
     */
    const char* debugString();
    void setCookie(void(*cookie)()) { m_cookie = cookie; }
    /**
     * @brief for used by unit test
     */
    std::string toString() const { return std::string(m_data, length()); }
    StringPiece toStringPiece() const { return StringPiece(m_data, length()); }

private:
    const char* end() const { return m_data + sizeof(m_data); }
    static void cookieStart();
    static void cookieEnd();

private:
    void (*m_cookie)();
    // thread_local static char m_data[SIZE];
    char m_data[SIZE];
    char* m_cur;
};

}  // namespace details

class LogStream : noncopyable
{   
    typedef LogStream self;
public:
    typedef details::FixedBuffer<details::kSmallBuffer> Buffer;

    self& operator<<(bool v)
    {
        m_buffer.append(v ? "1" : "0", 1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);

    self& operator<<(int);
    self& operator<<(unsigned int);

    self& operator<<(long);
    self& operator<<(unsigned long);

    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator<<(double);
    // self& operator<<(long double);


    self& operator<<(char v)
    {
        m_buffer.append(&v, 1);
        return *this;
    }

    // self& operator<<(signed char);
    // self& operator<<(unsigned char);

    /**
     * @brief 日志流类输出字符串，会有 stelen 的性能损失
     */
    self& operator<<(const char* v)
    {
        if(v)
        {
            m_buffer.append(v, strlen(v));
        }
        else
        {
            m_buffer.append("(null)", 6);
        }   
        return *this;
    }

    self& operator<<(const unsigned char* v)
    {
        return operator<<(reinterpret_cast<const char*>(v));
    }

    self& operator<<(const std::string& v)
    {
        m_buffer.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const StringPiece& v)
    {
        m_buffer.append(v.data(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v)
    {
        *this << v.toStringPiece();
        return *this;
    }

    void append(const char* data, int len) { m_buffer.append(data, len); }
    const Buffer& buffer() const { return m_buffer; }
    void resetBuffer() { m_buffer.reset(); }

private:
    /**
     * @brief 确保 s_maxNumericSize 大于所有数值类型的最大十进制位数
     */
    void staticCheck();

    template <typename T>
    void formatInteger(T);
    
private:
    Buffer m_buffer;
    static const int s_maxNumericSize = 48;
};

/**
 * @brief 在编译时确定各种算术类型格式化字符串后的长度
 */
class Fmt
{
public:
    template <typename T>
    Fmt(const char* fmt, T val);

    const char* data() const { return m_buf; }
    int length() const { return m_length; }
private:
    char m_buf[32];
    int  m_length;
};

/**
 * @brief 重载输出 Fmt 
 */
inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}

/**
 * @brief Format quantity n in SI units (k, M, G, T, P, E) based on 1000.
 *        The returned string is atmost 5 characters long.     
 *        Requires n >= 0
 */
std::string FormatSI(int64_t n);

/**
 * @brief Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei) based on 1024.
 *        The returned string is atmost 6 characters long.
 *        Requires n >= 0
 */
std::string FormatIEC(int64_t n);

}  // namespace mrpc

#endif  // __MRPC_BASE_LOGSTREAM_H__