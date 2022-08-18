#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <limits>
#include <algorithm>
#include <type_traits>

#include "LogStream.h" 

#pragma GCC diagnostic ignored "-Wtype-limits"

namespace mrpc
{
namespace details
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

/**
 * @brief Efficient Integer to String Conversions, by Matthew Wilson.
 */
template <typename T>
size_t Convert(char buf[], T value)
{
    T i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

/**
 * @brief Efficient Pointer to Hex String Conversions, by Matthew Wilson.
 */
size_t ConvertHex(char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;

    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

// template <int SIZE>
// thread_local char FixedBuffer<SIZE>::m_data[SIZE];

template <int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
    *m_cur = '\0';
    return m_data;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart() { }

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd() { }

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

}  // namespace details

/**
 * [0,     999]
 * [1.00k, 999k]
 * [1.00M, 999M]
 * [1.00G, 999G]
 * [1.00T, 999T]
 * [1.00P, 999P]
 * [1.00E, inf)
 */
std::string FormatSI(int64_t s)
{
    double n = static_cast<double>(s);
    char buf[64];
    if (s < 1000)
        snprintf(buf, sizeof(buf), "%" PRId64, s);
    else if (s < 9995)
        snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
    else if (s < 99950)
        snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
    else if (s < 999500)
        snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
    else if (s < 9995000)
        snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
    else if (s < 99950000)
        snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
    else if (s < 999500000)
        snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
    else if (s < 9995000000)
        snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
    else if (s < 99950000000)
        snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
    else if (s < 999500000000)
        snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
    else if (s < 9995000000000)
        snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
    else if (s < 99950000000000)
        snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
    else if (s < 999500000000000)
        snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
    else if (s < 9995000000000000)
        snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
    else if (s < 99950000000000000)
        snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
    else if (s < 999500000000000000)
        snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
    else
        snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
    return buf;
}

/**
 * [0, 1023]
 * [1.00Ki, 9.99Ki]
 * [10.0Ki, 99.9Ki]
 * [ 100Ki, 1023Ki]
 * [1.00Mi, 9.99Mi]
 */
std::string FormatIEC(int64_t s)
{
    double n = static_cast<double>(s);
    char buf[64];
    const double Ki = 1024.0;
    const double Mi = Ki * 1024.0;
    const double Gi = Mi * 1024.0;
    const double Ti = Gi * 1024.0;
    const double Pi = Ti * 1024.0;
    const double Ei = Pi * 1024.0;

    if (n < Ki)
        snprintf(buf, sizeof buf, "%" PRId64, s);
    else if (n < Ki*9.995)
        snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
    else if (n < Ki*99.95)
        snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
    else if (n < Ki*1023.5)
        snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

    else if (n < Mi*9.995)
        snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
    else if (n < Mi*99.95)
        snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
    else if (n < Mi*1023.5)
        snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

    else if (n < Gi*9.995)
        snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
    else if (n < Gi*99.95)
        snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
    else if (n < Gi*1023.5)
        snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

    else if (n < Ti*9.995)
        snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
    else if (n < Ti*99.95)
        snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
    else if (n < Ti*1023.5)
        snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

    else if (n < Pi*9.995)
        snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
    else if (n < Pi*99.95)
        snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
    else if (n < Pi*1023.5)
        snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

    else if (n < Ei*9.995)
        snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
    else
        snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
    return buf;
}

/**
 * @brief 确保 s_maxNumericSize 大于所有数值类型的最大十进制位数
 */
void LogStream::staticCheck()
{
    static_assert(s_maxNumericSize - 10 > std::numeric_limits<double>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(s_maxNumericSize - 10 > std::numeric_limits<long double>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(s_maxNumericSize - 10 > std::numeric_limits<long>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(s_maxNumericSize - 10 > std::numeric_limits<long long>::digits10,
                  "kMaxNumericSize is large enough");
}

/**
 * @brief 将各种整形转换为字符串并写入缓冲区
 */
template <typename T>
void LogStream::formatInteger(T v)
{
    if(m_buffer.available() >= s_maxNumericSize)
    {
        size_t len = details::Convert(m_buffer.current(), v);
        m_buffer.add(len);
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

/**
 * @brief 为日志流重载输出指针类型
 */
LogStream& LogStream::operator<<(const void* p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if(m_buffer.available() >= s_maxNumericSize)
    {
		char* buf = m_buffer.current();
		buf[0] = '0';
		buf[1] = 'x';
		size_t len = details::ConvertHex(buf + 2, v);
		m_buffer.add(len + 2);
    } 
	return *this;
}

LogStream& LogStream::operator<<(double  v)
{
	if(m_buffer.available() >= s_maxNumericSize)
	{
		int len = snprintf(m_buffer.current(), s_maxNumericSize, "%.12g", v);
		m_buffer.add(len);
	}
	return *this;
}

template <typename T>
Fmt::Fmt(const char* fmt, T val)
{
  /// 静态断言 T 类型为算术类型
	static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");
	m_length = snprintf(m_buf, sizeof(m_buf), fmt, val);
  /// 检查格式化后字符串的长度小于 buf 长度
	assert(static_cast<size_t>(m_length) < sizeof(m_buf));
}

/// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);

template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);

template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);

template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);

}  // namespace mrpc

