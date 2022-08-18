#include "stdint.h"

#include <limits>

#include "Util.h"
#include "LogStream.h"

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

using namespace mrpc;

BOOST_AUTO_TEST_CASE(TestLogStreamBooleans)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    BOOST_CHECK_EQUAL(buf.toString(), std::string(""));
    os << true;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1"));
    os << '\n';
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1\n"));
    os << false;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1\n0"));
}

BOOST_AUTO_TEST_CASE(TestLogStreamIntergers)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    BOOST_CHECK_EQUAL(buf.toString(), std::string(""));
    os << 1;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1"));
    os << 0;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("10"));
    os << -1;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("10-1"));
    os.resetBuffer();
    os << 0 << " " << 123 << 'x' << 0x64;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0 123x100"));
}

BOOST_AUTO_TEST_CASE(TestLogStreamIntegerLimits)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    os << -2147483647;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-2147483647"));
    os << static_cast<int>(-2147483647 - 1);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-2147483647-2147483648 2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-32768"));
    os.resetBuffer();

    os << std::numeric_limits<int16_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("32767"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint16_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("65535"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-2147483648"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint32_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("4294967295"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-9223372036854775808"));
    os.resetBuffer();

    os << std::numeric_limits<int64_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("9223372036854775807"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::min();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0"));
    os.resetBuffer();

    os << std::numeric_limits<uint64_t>::max();
    BOOST_CHECK_EQUAL(buf.toString(), std::string("18446744073709551615"));
    os.resetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a;
    os << b;
    os << c;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("000"));
}

BOOST_AUTO_TEST_CASE(TestLogStreamFloats)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << 0.0;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0"));
    os.resetBuffer();

    os << 1.0;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1"));
    os.resetBuffer();

    os << 0.1;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.1"));
    os.resetBuffer();

    os << 0.05;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.05"));
    os.resetBuffer();

    os << 0.15;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.15"));
    os.resetBuffer();

    double a = 0.1;
    os << a;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.1"));
    os.resetBuffer();

    double b = 0.05;
    os << b;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.05"));
    os.resetBuffer();

    double c = 0.15;
    os << c;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.15"));
    os.resetBuffer();

    os << a+b;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0.15"));
    os.resetBuffer();

    BOOST_CHECK(a+b != c);

    os << 1.23456789;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1.23456789"));
    os.resetBuffer();

    os << 1.234567;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1.234567"));
    os.resetBuffer();

    os << -123.456;
    BOOST_CHECK_EQUAL(buf.toString(), std::string("-123.456"));
    os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(TestLogStreamVoid)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << static_cast<void*>(0);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0x0"));
    os.resetBuffer();

    os << reinterpret_cast<void*>(8888);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("0x22B8"));
    os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(TestLogStreamStrings)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << "Hello ";
    BOOST_CHECK_EQUAL(buf.toString(), std::string("Hello "));
}

BOOST_AUTO_TEST_CASE(TestLogStreamFmts)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << Fmt("%4d", 1);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("   1"));
    os.resetBuffer();

    os << Fmt("%4.2f", 1.2);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1.20"));
    os.resetBuffer();

    os << Fmt("%4.2f", 1.2) << Fmt("%4d", 43);
    BOOST_CHECK_EQUAL(buf.toString(), std::string("1.20  43"));
    os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(TestLogStreamLong)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    for (int i = 0; i < 399; ++i)
    {
        os << "123456789 ";
        BOOST_CHECK_EQUAL(buf.length(), 10*(i+1));
        BOOST_CHECK_EQUAL(buf.available(), 4000 - 10*(i+1));
    }

    os << "abcdefghi ";
    BOOST_CHECK_EQUAL(buf.length(), 3990);
    BOOST_CHECK_EQUAL(buf.available(), 10);

    os << "abcdefghi";
    BOOST_CHECK_EQUAL(buf.length(), 3999);
    BOOST_CHECK_EQUAL(buf.available(), 1);
}

BOOST_AUTO_TEST_CASE(TestFormatSI)
{
  BOOST_CHECK_EQUAL(FormatSI(0), std::string("0"));
  BOOST_CHECK_EQUAL(FormatSI(999), std::string("999"));
  BOOST_CHECK_EQUAL(FormatSI(1000), std::string("1.00k"));
  BOOST_CHECK_EQUAL(FormatSI(9990), std::string("9.99k"));
  BOOST_CHECK_EQUAL(FormatSI(9994), std::string("9.99k"));
  BOOST_CHECK_EQUAL(FormatSI(9995), std::string("10.0k"));
  BOOST_CHECK_EQUAL(FormatSI(10000), std::string("10.0k"));
  BOOST_CHECK_EQUAL(FormatSI(10049), std::string("10.0k"));
  BOOST_CHECK_EQUAL(FormatSI(10050), std::string("10.1k"));
  BOOST_CHECK_EQUAL(FormatSI(99900), std::string("99.9k"));
  BOOST_CHECK_EQUAL(FormatSI(99949), std::string("99.9k"));
  BOOST_CHECK_EQUAL(FormatSI(99950), std::string("100k"));
  BOOST_CHECK_EQUAL(FormatSI(100499), std::string("100k"));
  // FIXME:
  // BOOST_CHECK_EQUAL(FormatSI(100500), string("101k"));
  BOOST_CHECK_EQUAL(FormatSI(100501), std::string("101k"));
  BOOST_CHECK_EQUAL(FormatSI(999499), std::string("999k"));
  BOOST_CHECK_EQUAL(FormatSI(999500), std::string("1.00M"));
  BOOST_CHECK_EQUAL(FormatSI(1004999), std::string("1.00M"));
  // BOOST_CHECK_EQUAL(FormatSI(1005000), string("1.01M"));
  BOOST_CHECK_EQUAL(FormatSI(1005001), std::string("1.01M"));
  BOOST_CHECK_EQUAL(FormatSI(INT64_MAX), std::string("9.22E"));
}

BOOST_AUTO_TEST_CASE(TestFormatIEC)
{
  BOOST_CHECK_EQUAL(FormatIEC(0), std::string("0"));
  BOOST_CHECK_EQUAL(FormatIEC(1023), std::string("1023"));
  BOOST_CHECK_EQUAL(FormatIEC(1024), std::string("1.00Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(10234), std::string("9.99Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(10235), std::string("10.0Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(10240), std::string("10.0Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(10291), std::string("10.0Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(10292), std::string("10.1Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(102348), std::string("99.9Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(102349), std::string("100Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(102912), std::string("100Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(102913), std::string("101Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(1022976), std::string("999Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(1047552), std::string("1023Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(1047961), std::string("1023Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(1048063), std::string("1023Ki"));
  BOOST_CHECK_EQUAL(FormatIEC(1048064), std::string("1.00Mi"));
  BOOST_CHECK_EQUAL(FormatIEC(1048576), std::string("1.00Mi"));
  BOOST_CHECK_EQUAL(FormatIEC(10480517), std::string("9.99Mi"));
  BOOST_CHECK_EQUAL(FormatIEC(10480518), std::string("10.0Mi"));
  BOOST_CHECK_EQUAL(FormatIEC(INT64_MAX), std::string("8.00Ei"));
}