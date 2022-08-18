#include <stdio.h>
#include <cxxabi.h>

#include <limits>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "Util.h"
#include "Time.h"
#include "LogStream.h"
#include "TimeStamp.h"
#include "StringPiece.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace mrpc;

#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace 
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17, "wrong number of digitsHex");

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

}  // namespace

template <typename T>
void ConvertPrint()
{
    Util::PrintTitle(abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr));
    T v1 = std::numeric_limits<T>::max();
    T v2 = std::numeric_limits<T>::min();
    char value[64];
    memset(value, 0, sizeof(value));
    Convert(value, v1);
    std::cout << "v1 = " << value << std::endl;
    memset(value, 0, sizeof(value));
    Convert(value, v2);
    std::cout << "v2 = " << value << std::endl;
    char pointer[32] = { 0 };
    pointer[0] = '0';
    pointer[1] = 'x';
    
    uintptr_t p = reinterpret_cast<uintptr_t>(&v1);
    ConvertHex(pointer + 2, p);
    std::cout << "p = " << &v1 << std::endl;
    std::cout << "p = " << pointer << std::endl;
}

void ConvertTest()
{
    ConvertPrint<int>();
    ConvertPrint<unsigned>();
    ConvertPrint<long>();
    ConvertPrint<unsigned long>();
    ConvertPrint<long long>();
    ConvertPrint<unsigned long long>(); 
}

const size_t g_max = 10 * 1000 * 1000;

template <typename T>
void BenchPrintf(const char* fmt)
{   
    Util::PrintTitle("BenchPrintf");
    WallTime wall;
    ClockTime timer;
    wall.start();
    timer.start();
    TimeStamp start(TimeStamp::Now());

    char buf[32];
    for(size_t i = 0; i < g_max; ++i)
        snprintf(buf, sizeof(buf), fmt, (T)(i));
    
    TimeStamp stop(TimeStamp::Now());
    wall.stop();
    timer.stop();

    printf("CPU time = %f\n", timer.duration());
    printf("TimeDiff = %f\n", TimeDifference(stop, start));
    printf("Walltime = %f\n", wall.duration());
    
}

template <typename T>
void BenchStringStream()
{   
    Util::PrintTitle("BenchStringStream");
    WallTime wall;
    ClockTime cpu;
    wall.start();
    cpu.start();
    TimeStamp start(TimeStamp::Now());

    std::ostringstream os;
    for(size_t i = 0; i < g_max; ++i)
    {
        os << (T)(i);
        os.seekp(0, std::ios_base::beg);
    }

    wall.stop();
    cpu.stop();
    TimeStamp stop(TimeStamp::Now());

    printf("CPU time = %f\n", cpu.duration());  
    printf("Walltime = %f\n", wall.duration());
    printf("Timediff = %f\n", TimeDifference(stop, start));
}

template <typename T>
void BenchLogStream()
{
    Util::PrintTitle("BenchLogStream");
    WallTime wall;
    ClockTime cpu;
    wall.start();
    cpu.start();
    TimeStamp start(TimeStamp::Now());

    LogStream os;
    for(size_t i = 0; i < g_max; ++i)
    {
        os << (T)(i);
        os.resetBuffer();
    }

    wall.stop();
    cpu.stop();
    TimeStamp stop(TimeStamp::Now());

    printf("CPU time = %f\n", cpu.duration());  
    printf("Walltime = %f\n", wall.duration());
    printf("Timediff = %f\n", TimeDifference(stop, start));
}

void BenchTest()
{
    Util::PrintTitle("Test LogStream << int");
    BenchPrintf<int>("%d");
    BenchStringStream<int>();
    BenchLogStream<int>();
    
    Util::PrintTitle("Test LogStream << double");
    BenchPrintf<double>("%.12g");
    BenchStringStream<double>();
    BenchLogStream<double>();

    Util::PrintTitle("Test LogStream << int64_t");
    BenchPrintf<int64_t>("%" PRId64);
    BenchStringStream<int64_t>();
    BenchLogStream<int64_t>();

    Util::PrintTitle("Test LogStream << void*");
    BenchPrintf<void*>("%p");
    BenchStringStream<void*>();
    BenchLogStream<void*>();
}

int main()
{
    ConvertTest();
    BenchTest();

    return 0;
}