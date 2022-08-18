#include <stdio.h>
#include <assert.h>

#include <vector>

#include "Util.h"
#include "TimeStamp.h"

using namespace mrpc;

/**
 * @brief 64 位寄存器传值
 */
void PassByValue(TimeStamp time)
{
    Util::PrintTitle("PassByValue");
    printf("string = %s\n", time.toString().c_str());
    printf("formattedstring = %s\n\n", time.toFormattedString().c_str());
}

/**
 * @brief 引用传递
 */
void PassByConstReference(const TimeStamp& time)
{
    Util::PrintTitle("PassByConstReference");
    printf("%s\n", time.toString().c_str());
    printf("formattedstring = %s\n\n", time.toFormattedString().c_str());
}

/**
 * @brief 测试所有重载的操作符
 */
void TestOperator()
{
    Util::PrintTitle("TestOperator");
    auto t1 = TimeStamp::Now();
    Util::SleepMsec(100);
    auto t2 = TimeStamp::Now();
    assert(t1 < t2);
    assert(!(t1 > t2));
    assert(t1 != t2);
    assert(!(t1 == t2));
    assert(t1 <= t2);
    assert(!(t1 >= t2));
    assert(t1 == t1);
    (void)t1;
    (void)t2;
    printf("All test passed!\n");
}

/**
 * @brief 性能测试
 */
void Benchmark()
{
    Util::PrintTitle("Benchmark");

    const int num = 10 * 1000 * 1000;

    std::vector<TimeStamp> stamps;
    stamps.reserve(num);
    for(int i = 0; i < num; ++i)
    {
        stamps.push_back(TimeStamp::Now());
    }
    
    printf("start time = %s\n", stamps.front().toFormattedString().c_str());
    printf("stop  time = %s\n", stamps.back().toFormattedString().c_str());
    printf("diff = %f\n", TimeDifference(stamps.back(), stamps.front()));

    int increment[100] = { 0 };
    int64_t start = stamps.front().microSecondsSinceEpoch();
    for(int i = 1; i < num; ++i)
    {
        int64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;
        if(inc < 0)
        {
            printf("reverse!\n");
        }
        else if(inc < 100)
        {
            ++increment[inc];
        }
        else
        {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }
    
    for(int i = 0; i < 100; ++i)
    {
        printf("%2d: %d\n", i, increment[i]);
    }
}

int main()
{
    TimeStamp now(TimeStamp::Now());
    PassByValue(now);
    PassByConstReference(now);
    TestOperator();
    Benchmark();
    return 0;
}

