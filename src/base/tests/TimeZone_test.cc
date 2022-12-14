#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "Util.h"
#include "TimeZone.h"

using namespace mrpc;

/**
 * @brief Date to tm
 */
struct tm GetTm(int year, int month, int day,
                int hour, int minute, int seconds)
{
    struct tm utc;
    memset(&utc, 0, sizeof(utc));
    utc.tm_year = year - 1900;
    utc.tm_mon = month - 1;
    utc.tm_mday = day;
    utc.tm_hour = hour;
    utc.tm_min = minute;
    utc.tm_sec = seconds;
    return utc;
}

/**
 * @brief '%F %T' format str to tm
 */
struct tm  GetTm(const char* str)
{
    struct tm utc;
    memset(&utc, 0, sizeof(utc));
    /// date +'%F %T'
    /// 2022-07-26 13:25:49
    strptime(str, "%F %T", &utc);
    return utc;
}

/**
 * @brief Date to time_t
 */
time_t GetUTC(int year, int month, int day,
              int hour, int minute, int seconds)
{
    struct tm utc = GetTm(year, month, day, hour, minute, seconds);
    return timegm(&utc);
}

/**
 * @brief '%F %T' format str to time_t
 */
time_t GetUTC(const char* str)
{
    struct tm utc = GetTm(str);
    return timegm(&utc);
}

struct TestCase
{
    const char* utc;    // UTC(GMT) 时间
    const char* local;  // 测试样例预置的正确的 local 时间
    bool isdst;         // 是否使用夏令时
};

/**
 * @brief 显示 tm 结构体的全部数据，用于观察和 debug
 */
void PrintTm(struct tm* t)
{
    printf("tm_sec = %d\n", t->tm_sec);
    printf("tm_min = %d\n", t->tm_min);
    printf("tm_hour = %d\n", t->tm_hour);
    printf("tm_mday = %d\n", t->tm_mday);
    printf("tm_mon = %d\n", t->tm_mon);
    printf("tm_year = %d\n", t->tm_year);
    printf("tm_wday = %d\n", t->tm_wday);
    printf("tm_yday = %d\n", t->tm_yday);
    printf("tm_isdst = %d\n", t->tm_isdst);
    printf("tm_gmtoff = %ld\n", t->tm_gmtoff);
    printf("tm_zone = %s\n", t->tm_zone);
}

/**
 * @brief 生成 1970 至 2021 年约 50 组测试样例数据
 * @details 更换系统时区生成不同地区的测试数据用于后续测试
 * @details timedatectl 调整系统时区(e.g., timedatectl set-timezone Asia/Shanghai)
 */
void GenerateTestCase() 
{
    const time_t left = 1 * 1000 * 1000;
    const time_t right = 1650 * 1000 * 1000;
    time_t diff = (right - left) / 50;
    for(time_t t = left; t < right; t += diff)
    {
        struct tm utc, local;
        gmtime_r(&t, &utc);
        localtime_r(&t, &local);
        char buf1[30], buf2[40];
        strftime(buf1, sizeof(buf1), "%F %T", &utc);
        strftime(buf2, sizeof(buf2), "%F %T%z(%Z)", &local);
        printf("%s %s %d\n", buf1, buf2, local.tm_isdst);
    }
}

/**
 * @brief 简单测试 time_t 和 tm 的相互转化
 * @details mktime 用于 UTC tm 会计算时区的时差导致错误
 */
void TestConvert()
{
    Util::PrintTitle("TestConvert");
    time_t t = time(nullptr);
    struct tm utc, local;
    gmtime_r(&t, &utc);
    localtime_r(&t, &local);

    Util::PrintTitle("utc");
    PrintTm(&utc);
    Util::PrintTitle("local");
    PrintTm(&local);

    Util::PrintTitle("tm to time_t");
    /// timegm 函数只是将 struct tm 结构转成 time_t 纪元时，不使用时区信息
    time_t utctime = timegm(&utc);
    /// mktime 将 struct tm 结构转成 time_t 纪元时，使用时区信息
     time_t localtime = mktime(&local);
    /// timelocal 函数是 GNU 的扩展，功能与 posix 函数 mktime 相当
    //time_t localtime = timelocal(&local);
    printf("t = %ld\n", t);
    printf("utctime = %ld\n", utctime);
    printf("localtime = %ld\n", localtime);
}

/**
 * @brief 测试单个用例
 */
void SingleCaseTest(const TimeZone& tz, TestCase tc)
{
    // 由 '%F %T' 格式字符串获取 secondsSinceEpoch 时间
    time_t utc = GetUTC(tc.utc);

    /// 将 UTC 纪元时转换至 tz 时区对应的当地时间，在字符串格式化后于正确答案对比
    {
        /// 将 secondsSinceEpoch 时间转换到 TimeZone 对应时区时间
        struct tm local = tz.toLocalTime(utc);
        char buf[256];
        /// date +'%F %T%z(%Z)'
        /// 2022-07-26 13:27:15+0800(CST)
        strftime(buf, sizeof(buf), "%F %T%z(%Z)", &local);

        if (strcmp(buf, tc.local) != 0 || tc.isdst != local.tm_isdst)
        {
            /// 打印错误信息查看
            if(strcmp(buf, tc.local) != 0) printf("strcmp\n"); 
            if(tc.isdst != local.tm_isdst) printf("isdst\n"); 
            printf("WRONG: ");
        }
        printf("%s -> %s\n", tc.utc, buf);
    }

    /// 将 local time 通过 fromLocalTime 函数转化为 UTC 纪元时间，并与正确答案对比
    {   
        struct tm local = GetTm(tc.local);
        local.tm_isdst = tc.isdst;
        time_t result = tz.fromLocalTime(local);
        if (result != utc)
        {
            struct tm local2 = tz.toLocalTime(result);
            char buf[256];
            strftime(buf, sizeof(buf), "%F %T%z(%Z)", &local2);

            printf("WRONG fromLocalTime: %ld %ld %s\n",
                static_cast<long>(utc), static_cast<long>(result), buf);
        }
    }
}

/**
 * @brief 测试 UTC 相关函数功能
 */
void TestUTC()
{
    // const int kRange = 100 * 1000 * 1000;
    const int kRange = 100 * 1000;
    for (time_t t = -kRange; t <= kRange; t += 11)
    {
        struct tm* t1 = gmtime(&t);
        struct tm t2 = TimeZone::toUtcTime(t, true);
        // PrintTm(t1);
        // PrintTm(&t2);

        char buf1[80], buf2[80];
        strftime(buf1, sizeof(buf1), "%F %T %u %j", t1);
        strftime(buf2, sizeof(buf2), "%F %T %u %j", &t2);
        printf("%s\n", buf1);
        printf("%s\n", buf2);
        if (strcmp(buf1, buf2) != 0)
        {
            printf("'%s' != '%s'\n", buf1, buf2);
            assert(0);
        }
        time_t t3 = timegm(t1);
        time_t t4 = TimeZone::fromUtcTime(t2);
        if (t3 != t4)
        {
            printf("%ld != %ld\n", static_cast<long>(3), static_cast<long>(t4));
            assert(0);
        }
    }
}

/**
 * @brief 测试服务器的本地所在时区，Asia/Shanghai
 */
void TestLocal()
{
    TimeZone tz("/usr/share/zoneinfo/Asia/Shanghai");
    TestCase cases[] = 
    {
        { "1970-01-12 13:46:40", "1970-01-12 21:46:40+0800(CST)", false },
        { "1971-01-29 06:53:20", "1971-01-29 14:53:20+0800(CST)", false },
        { "1972-02-15 00:00:00", "1972-02-15 08:00:00+0800(CST)", false },
        { "1973-03-02 17:06:40", "1973-03-03 01:06:40+0800(CST)", false },
        { "1974-03-19 10:13:20", "1974-03-19 18:13:20+0800(CST)", false },
        { "1975-04-05 03:20:00", "1975-04-05 11:20:00+0800(CST)", false },
        { "1976-04-20 20:26:40", "1976-04-21 04:26:40+0800(CST)", false },
        { "1977-05-07 13:33:20", "1977-05-07 21:33:20+0800(CST)", false },
        { "1978-05-24 06:40:00", "1978-05-24 14:40:00+0800(CST)", false },
        { "1979-06-09 23:46:40", "1979-06-10 07:46:40+0800(CST)", false },
        { "1980-06-25 16:53:20", "1980-06-26 00:53:20+0800(CST)", false },
        { "1981-07-12 10:00:00", "1981-07-12 18:00:00+0800(CST)", false },
        { "1982-07-29 03:06:40", "1982-07-29 11:06:40+0800(CST)", false },
        { "1983-08-14 20:13:20", "1983-08-15 04:13:20+0800(CST)", false },
        { "1984-08-30 13:20:00", "1984-08-30 21:20:00+0800(CST)", false },
        { "1985-09-16 06:26:40", "1985-09-16 14:26:40+0800(CST)", false },
        { "1986-10-02 23:33:20", "1986-10-03 07:33:20+0800(CST)", false },
        { "1987-10-19 16:40:00", "1987-10-20 00:40:00+0800(CST)", false },
        { "1988-11-04 09:46:40", "1988-11-04 17:46:40+0800(CST)", false },
        { "1989-11-21 02:53:20", "1989-11-21 10:53:20+0800(CST)", false },
        { "1990-12-07 20:00:00", "1990-12-08 04:00:00+0800(CST)", false },
        { "1991-12-24 13:06:40", "1991-12-24 21:06:40+0800(CST)", false },
        { "1993-01-09 06:13:20", "1993-01-09 14:13:20+0800(CST)", false },
        { "1994-01-25 23:20:00", "1994-01-26 07:20:00+0800(CST)", false },
        { "1995-02-11 16:26:40", "1995-02-12 00:26:40+0800(CST)", false },
        { "1996-02-28 09:33:20", "1996-02-28 17:33:20+0800(CST)", false },
        { "1997-03-16 02:40:00", "1997-03-16 10:40:00+0800(CST)", false },
        { "1998-04-01 19:46:40", "1998-04-02 03:46:40+0800(CST)", false },
        { "1999-04-18 12:53:20", "1999-04-18 20:53:20+0800(CST)", false },
        { "2000-05-04 06:00:00", "2000-05-04 14:00:00+0800(CST)", false },
        { "2001-05-20 23:06:40", "2001-05-21 07:06:40+0800(CST)", false },
        { "2002-06-06 16:13:20", "2002-06-07 00:13:20+0800(CST)", false },
        { "2003-06-23 09:20:00", "2003-06-23 17:20:00+0800(CST)", false },
        { "2004-07-09 02:26:40", "2004-07-09 10:26:40+0800(CST)", false },
        { "2005-07-25 19:33:20", "2005-07-26 03:33:20+0800(CST)", false },
        { "2006-08-11 12:40:00", "2006-08-11 20:40:00+0800(CST)", false },
        { "2007-08-28 05:46:40", "2007-08-28 13:46:40+0800(CST)", false },
        { "2008-09-12 22:53:20", "2008-09-13 06:53:20+0800(CST)", false },
        { "2009-09-29 16:00:00", "2009-09-30 00:00:00+0800(CST)", false },
        { "2010-10-16 09:06:40", "2010-10-16 17:06:40+0800(CST)", false },
        { "2011-11-02 02:13:20", "2011-11-02 10:13:20+0800(CST)", false },
        { "2012-11-17 19:20:00", "2012-11-18 03:20:00+0800(CST)", false },
        { "2013-12-04 12:26:40", "2013-12-04 20:26:40+0800(CST)", false },
        { "2014-12-21 05:33:20", "2014-12-21 13:33:20+0800(CST)", false },
        { "2016-01-06 22:40:00", "2016-01-07 06:40:00+0800(CST)", false },
        { "2017-01-22 15:46:40", "2017-01-22 23:46:40+0800(CST)", false },
        { "2018-02-08 08:53:20", "2018-02-08 16:53:20+0800(CST)", false },
        { "2019-02-25 02:00:00", "2019-02-25 10:00:00+0800(CST)", false },
        { "2020-03-12 19:06:40", "2020-03-13 03:06:40+0800(CST)", false },
        { "2021-03-29 12:13:20", "2021-03-29 20:13:20+0800(CST)", false }
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

/**
 *  @brief 测试纽约所在时区，America/New_York
 */
void TestNewYork()
{
    Util::PrintTitle("TestNewYork");
    TimeZone tz("/usr/share/zoneinfo/America/New_York");
    TestCase cases[] =
    {
        { "1970-01-12 13:46:40", "1970-01-12 08:46:40-0500(EST)", false },
        { "1971-01-29 06:53:20", "1971-01-29 01:53:20-0500(EST)", false },
        { "1972-02-15 00:00:00", "1972-02-14 19:00:00-0500(EST)", false },
        { "1973-03-02 17:06:40", "1973-03-02 12:06:40-0500(EST)", false },
        { "1974-03-19 10:13:20", "1974-03-19 06:13:20-0400(EDT)", true  },
        { "1975-04-05 03:20:00", "1975-04-04 23:20:00-0400(EDT)", true  },
        { "1976-04-20 20:26:40", "1976-04-20 15:26:40-0500(EST)", false },
        { "1977-05-07 13:33:20", "1977-05-07 09:33:20-0400(EDT)", true  },
        { "1978-05-24 06:40:00", "1978-05-24 02:40:00-0400(EDT)", true  },
        { "1979-06-09 23:46:40", "1979-06-09 19:46:40-0400(EDT)", true  },
        { "1980-06-25 16:53:20", "1980-06-25 12:53:20-0400(EDT)", true  },
        { "1981-07-12 10:00:00", "1981-07-12 06:00:00-0400(EDT)", true  },
        { "1982-07-29 03:06:40", "1982-07-28 23:06:40-0400(EDT)", true  },
        { "1983-08-14 20:13:20", "1983-08-14 16:13:20-0400(EDT)", true  },
        { "1984-08-30 13:20:00", "1984-08-30 09:20:00-0400(EDT)", true  },
        { "1985-09-16 06:26:40", "1985-09-16 02:26:40-0400(EDT)", true  },
        { "1986-10-02 23:33:20", "1986-10-02 19:33:20-0400(EDT)", true  },
        { "1987-10-19 16:40:00", "1987-10-19 12:40:00-0400(EDT)", true  },
        { "1988-11-04 09:46:40", "1988-11-04 04:46:40-0500(EST)", false },
        { "1989-11-21 02:53:20", "1989-11-20 21:53:20-0500(EST)", false },
        { "1990-12-07 20:00:00", "1990-12-07 15:00:00-0500(EST)", false },
        { "1991-12-24 13:06:40", "1991-12-24 08:06:40-0500(EST)", false },
        { "1993-01-09 06:13:20", "1993-01-09 01:13:20-0500(EST)", false },
        { "1994-01-25 23:20:00", "1994-01-25 18:20:00-0500(EST)", false },
        { "1995-02-11 16:26:40", "1995-02-11 11:26:40-0500(EST)", false },
        { "1996-02-28 09:33:20", "1996-02-28 04:33:20-0500(EST)", false },
        { "1997-03-16 02:40:00", "1997-03-15 21:40:00-0500(EST)", false },
        { "1998-04-01 19:46:40", "1998-04-01 14:46:40-0500(EST)", false },
        { "1999-04-18 12:53:20", "1999-04-18 08:53:20-0400(EDT)", true  },
        { "2000-05-04 06:00:00", "2000-05-04 02:00:00-0400(EDT)", true  },
        { "2001-05-20 23:06:40", "2001-05-20 19:06:40-0400(EDT)", true  },
        { "2002-06-06 16:13:20", "2002-06-06 12:13:20-0400(EDT)", true  },
        { "2003-06-23 09:20:00", "2003-06-23 05:20:00-0400(EDT)", true  },
        { "2004-07-09 02:26:40", "2004-07-08 22:26:40-0400(EDT)", true  },
        { "2005-07-25 19:33:20", "2005-07-25 15:33:20-0400(EDT)", true  },
        { "2006-08-11 12:40:00", "2006-08-11 08:40:00-0400(EDT)", true  },
        { "2007-08-28 05:46:40", "2007-08-28 01:46:40-0400(EDT)", true  },
        { "2008-09-12 22:53:20", "2008-09-12 18:53:20-0400(EDT)", true  },
        { "2009-09-29 16:00:00", "2009-09-29 12:00:00-0400(EDT)", true  },
        { "2010-10-16 09:06:40", "2010-10-16 05:06:40-0400(EDT)", true  },
        { "2011-11-02 02:13:20", "2011-11-01 22:13:20-0400(EDT)", true  },
        { "2012-11-17 19:20:00", "2012-11-17 14:20:00-0500(EST)", false },
        { "2013-12-04 12:26:40", "2013-12-04 07:26:40-0500(EST)", false },
        { "2014-12-21 05:33:20", "2014-12-21 00:33:20-0500(EST)", false },
        { "2016-01-06 22:40:00", "2016-01-06 17:40:00-0500(EST)", false },
        { "2017-01-22 15:46:40", "2017-01-22 10:46:40-0500(EST)", false },
        { "2018-02-08 08:53:20", "2018-02-08 03:53:20-0500(EST)", false },
        { "2019-02-25 02:00:00", "2019-02-24 21:00:00-0500(EST)", false },
        { "2020-03-12 19:06:40", "2020-03-12 15:06:40-0400(EDT)", true  },
        { "2021-03-29 12:13:20", "2021-03-29 08:13:20-0400(EDT)", true  }
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

/**
 *  @brief 测试伦敦所在时区，Europe/London
 */
void TestLondon()
{
    Util::PrintTitle("TestLondon");
    TimeZone tz("/usr/share/zoneinfo/Europe/London");
    TestCase cases[] =
    {
        { "1970-01-12 13:46:40", "1970-01-12 14:46:40+0100(BST)", false },
        { "1971-01-29 06:53:20", "1971-01-29 07:53:20+0100(BST)", false },
        { "1972-02-15 00:00:00", "1972-02-15 00:00:00+0000(GMT)", false },
        { "1973-03-02 17:06:40", "1973-03-02 17:06:40+0000(GMT)", false },
        { "1974-03-19 10:13:20", "1974-03-19 11:13:20+0100(BST)", true  },
        { "1975-04-05 03:20:00", "1975-04-05 04:20:00+0100(BST)", true  },
        { "1976-04-20 20:26:40", "1976-04-20 21:26:40+0100(BST)", true  },
        { "1977-05-07 13:33:20", "1977-05-07 14:33:20+0100(BST)", true  },
        { "1978-05-24 06:40:00", "1978-05-24 07:40:00+0100(BST)", true  },
        { "1979-06-09 23:46:40", "1979-06-10 00:46:40+0100(BST)", true  },
        { "1980-06-25 16:53:20", "1980-06-25 17:53:20+0100(BST)", true  },
        { "1981-07-12 10:00:00", "1981-07-12 11:00:00+0100(BST)", true  },
        { "1982-07-29 03:06:40", "1982-07-29 04:06:40+0100(BST)", true  },
        { "1983-08-14 20:13:20", "1983-08-14 21:13:20+0100(BST)", true  },
        { "1984-08-30 13:20:00", "1984-08-30 14:20:00+0100(BST)", true  },
        { "1985-09-16 06:26:40", "1985-09-16 07:26:40+0100(BST)", true  },
        { "1986-10-02 23:33:20", "1986-10-03 00:33:20+0100(BST)", true  },
        { "1987-10-19 16:40:00", "1987-10-19 17:40:00+0100(BST)", true  },
        { "1988-11-04 09:46:40", "1988-11-04 09:46:40+0000(GMT)", false },
        { "1989-11-21 02:53:20", "1989-11-21 02:53:20+0000(GMT)", false },
        { "1990-12-07 20:00:00", "1990-12-07 20:00:00+0000(GMT)", false },
        { "1991-12-24 13:06:40", "1991-12-24 13:06:40+0000(GMT)", false },
        { "1993-01-09 06:13:20", "1993-01-09 06:13:20+0000(GMT)", false },
        { "1994-01-25 23:20:00", "1994-01-25 23:20:00+0000(GMT)", false },
        { "1995-02-11 16:26:40", "1995-02-11 16:26:40+0000(GMT)", false },
        { "1996-02-28 09:33:20", "1996-02-28 09:33:20+0000(GMT)", false },
        { "1997-03-16 02:40:00", "1997-03-16 02:40:00+0000(GMT)", false },
        { "1998-04-01 19:46:40", "1998-04-01 20:46:40+0100(BST)", true  },
        { "1999-04-18 12:53:20", "1999-04-18 13:53:20+0100(BST)", true  },
        { "2000-05-04 06:00:00", "2000-05-04 07:00:00+0100(BST)", true  },
        { "2001-05-20 23:06:40", "2001-05-21 00:06:40+0100(BST)", true  },
        { "2002-06-06 16:13:20", "2002-06-06 17:13:20+0100(BST)", true  },
        { "2003-06-23 09:20:00", "2003-06-23 10:20:00+0100(BST)", true  },
        { "2004-07-09 02:26:40", "2004-07-09 03:26:40+0100(BST)", true  },
        { "2005-07-25 19:33:20", "2005-07-25 20:33:20+0100(BST)", true  },
        { "2006-08-11 12:40:00", "2006-08-11 13:40:00+0100(BST)", true  },
        { "2007-08-28 05:46:40", "2007-08-28 06:46:40+0100(BST)", true  },
        { "2008-09-12 22:53:20", "2008-09-12 23:53:20+0100(BST)", true  },
        { "2009-09-29 16:00:00", "2009-09-29 17:00:00+0100(BST)", true  },
        { "2010-10-16 09:06:40", "2010-10-16 10:06:40+0100(BST)", true  },
        { "2011-11-02 02:13:20", "2011-11-02 02:13:20+0000(GMT)", false },
        { "2012-11-17 19:20:00", "2012-11-17 19:20:00+0000(GMT)", false },
        { "2013-12-04 12:26:40", "2013-12-04 12:26:40+0000(GMT)", false },
        { "2014-12-21 05:33:20", "2014-12-21 05:33:20+0000(GMT)", false },
        { "2016-01-06 22:40:00", "2016-01-06 22:40:00+0000(GMT)", false },
        { "2017-01-22 15:46:40", "2017-01-22 15:46:40+0000(GMT)", false },
        { "2018-02-08 08:53:20", "2018-02-08 08:53:20+0000(GMT)", false },
        { "2019-02-25 02:00:00", "2019-02-25 02:00:00+0000(GMT)", false },
        { "2020-03-12 19:06:40", "2020-03-12 19:06:40+0000(GMT)", false },
        { "2021-03-29 12:13:20", "2021-03-29 13:13:20+0100(BST)", true  }
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

/**
 *  @brief 测试香港所再时区，Asia/Hong_Kong
 */
void TestHongKong()
{
    Util::PrintTitle("TestHongKong");
    TimeZone tz("/usr/share/zoneinfo/Asia/Hong_Kong");

    TestCase cases[] =
    {
        { "1970-01-12 13:46:40", "1970-01-12 21:46:40+0800(HKT)", false },
        { "1971-01-29 06:53:20", "1971-01-29 14:53:20+0800(HKT)", false },
        { "1972-02-15 00:00:00", "1972-02-15 08:00:00+0800(HKT)", false },
        { "1973-03-02 17:06:40", "1973-03-03 01:06:40+0800(HKT)", false },
        { "1974-03-19 10:13:20", "1974-03-19 19:13:20+0900(HKST)", true  },
        { "1975-04-05 03:20:00", "1975-04-05 11:20:00+0800(HKT)", false },
        { "1976-04-20 20:26:40", "1976-04-21 05:26:40+0900(HKST)", true  },
        { "1977-05-07 13:33:20", "1977-05-07 21:33:20+0800(HKT)", false },
        { "1978-05-24 06:40:00", "1978-05-24 14:40:00+0800(HKT)", false },
        { "1979-06-09 23:46:40", "1979-06-10 08:46:40+0900(HKST)", true  },
        { "1980-06-25 16:53:20", "1980-06-26 00:53:20+0800(HKT)", false },
        { "1981-07-12 10:00:00", "1981-07-12 18:00:00+0800(HKT)", false },
        { "1982-07-29 03:06:40", "1982-07-29 11:06:40+0800(HKT)", false },
        { "1983-08-14 20:13:20", "1983-08-15 04:13:20+0800(HKT)", false },
        { "1984-08-30 13:20:00", "1984-08-30 21:20:00+0800(HKT)", false },
        { "1985-09-16 06:26:40", "1985-09-16 14:26:40+0800(HKT)", false },
        { "1986-10-02 23:33:20", "1986-10-03 07:33:20+0800(HKT)", false },
        { "1987-10-19 16:40:00", "1987-10-20 00:40:00+0800(HKT)", false },
        { "1988-11-04 09:46:40", "1988-11-04 17:46:40+0800(HKT)", false },
        { "1989-11-21 02:53:20", "1989-11-21 10:53:20+0800(HKT)", false },
        { "1990-12-07 20:00:00", "1990-12-08 04:00:00+0800(HKT)", false },
        { "1991-12-24 13:06:40", "1991-12-24 21:06:40+0800(HKT)", false },
        { "1993-01-09 06:13:20", "1993-01-09 14:13:20+0800(HKT)", false },
        { "1994-01-25 23:20:00", "1994-01-26 07:20:00+0800(HKT)", false },
        { "1995-02-11 16:26:40", "1995-02-12 00:26:40+0800(HKT)", false },
        { "1996-02-28 09:33:20", "1996-02-28 17:33:20+0800(HKT)", false },
        { "1997-03-16 02:40:00", "1997-03-16 10:40:00+0800(HKT)", false },
        { "1998-04-01 19:46:40", "1998-04-02 03:46:40+0800(HKT)", false },
        { "1999-04-18 12:53:20", "1999-04-18 20:53:20+0800(HKT)", false },
        { "2000-05-04 06:00:00", "2000-05-04 14:00:00+0800(HKT)", false },
        { "2001-05-20 23:06:40", "2001-05-21 07:06:40+0800(HKT)", false },
        { "2002-06-06 16:13:20", "2002-06-07 00:13:20+0800(HKT)", false },
        { "2003-06-23 09:20:00", "2003-06-23 17:20:00+0800(HKT)", false },
        { "2004-07-09 02:26:40", "2004-07-09 10:26:40+0800(HKT)", false },
        { "2005-07-25 19:33:20", "2005-07-26 03:33:20+0800(HKT)", false },
        { "2006-08-11 12:40:00", "2006-08-11 20:40:00+0800(HKT)", false },
        { "2007-08-28 05:46:40", "2007-08-28 13:46:40+0800(HKT)", false },
        { "2008-09-12 22:53:20", "2008-09-13 06:53:20+0800(HKT)", false },
        { "2009-09-29 16:00:00", "2009-09-30 00:00:00+0800(HKT)", false },
        { "2010-10-16 09:06:40", "2010-10-16 17:06:40+0800(HKT)", false },
        { "2011-11-02 02:13:20", "2011-11-02 10:13:20+0800(HKT)", false },
        { "2012-11-17 19:20:00", "2012-11-18 03:20:00+0800(HKT)", false },
        { "2013-12-04 12:26:40", "2013-12-04 20:26:40+0800(HKT)", false },
        { "2014-12-21 05:33:20", "2014-12-21 13:33:20+0800(HKT)", false },
        { "2016-01-06 22:40:00", "2016-01-07 06:40:00+0800(HKT)", false },
        { "2017-01-22 15:46:40", "2017-01-22 23:46:40+0800(HKT)", false },
        { "2018-02-08 08:53:20", "2018-02-08 16:53:20+0800(HKT)", false },
        { "2019-02-25 02:00:00", "2019-02-25 10:00:00+0800(HKT)", false },
        { "2020-03-12 19:06:40", "2020-03-13 03:06:40+0800(HKT)", false },
        { "2021-03-29 12:13:20", "2021-03-29 20:13:20+0800(HKT)", false },
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

/**
 *  @brief 测试悉尼所在时区，Australia/Sydney
 */
void TestSydney()
{
    Util::PrintTitle("TestSydney");
    TimeZone tz("/usr/share/zoneinfo/Australia/Sydney");
    TestCase cases[] =
    {
        { "1970-01-12 13:46:40", "1970-01-12 23:46:40+1000(AEST)", false },
        { "1971-01-29 06:53:20", "1971-01-29 16:53:20+1000(AEST)", false },
        { "1972-02-15 00:00:00", "1972-02-15 11:00:00+1100(AEDT)", true  },
        { "1973-03-02 17:06:40", "1973-03-03 04:06:40+1100(AEDT)", true  },
        { "1974-03-19 10:13:20", "1974-03-19 20:13:20+1000(AEST)", false },
        { "1975-04-05 03:20:00", "1975-04-05 13:20:00+1000(AEST)", false },
        { "1976-04-20 20:26:40", "1976-04-21 06:26:40+1000(AEST)", false },
        { "1977-05-07 13:33:20", "1977-05-07 23:33:20+1000(AEST)", false },
        { "1978-05-24 06:40:00", "1978-05-24 16:40:00+1000(AEST)", false },
        { "1979-06-09 23:46:40", "1979-06-10 09:46:40+1000(AEST)", false },
        { "1980-06-25 16:53:20", "1980-06-26 02:53:20+1000(AEST)", false },
        { "1981-07-12 10:00:00", "1981-07-12 20:00:00+1000(AEST)", false },
        { "1982-07-29 03:06:40", "1982-07-29 13:06:40+1000(AEST)", false },
        { "1983-08-14 20:13:20", "1983-08-15 06:13:20+1000(AEST)", false },
        { "1984-08-30 13:20:00", "1984-08-30 23:20:00+1000(AEST)", false },
        { "1985-09-16 06:26:40", "1985-09-16 16:26:40+1000(AEST)", false },
        { "1986-10-02 23:33:20", "1986-10-03 09:33:20+1000(AEST)", false },
        { "1987-10-19 16:40:00", "1987-10-20 02:40:00+1000(AEST)", false },
        { "1988-11-04 09:46:40", "1988-11-04 20:46:40+1100(AEDT)", true  },
        { "1989-11-21 02:53:20", "1989-11-21 13:53:20+1100(AEDT)", true  },
        { "1990-12-07 20:00:00", "1990-12-08 07:00:00+1100(AEDT)", true  },
        { "1991-12-24 13:06:40", "1991-12-25 00:06:40+1100(AEDT)", true  },
        { "1993-01-09 06:13:20", "1993-01-09 17:13:20+1100(AEDT)", true  },
        { "1994-01-25 23:20:00", "1994-01-26 10:20:00+1100(AEDT)", true  },
        { "1995-02-11 16:26:40", "1995-02-12 03:26:40+1100(AEDT)", true  },
        { "1996-02-28 09:33:20", "1996-02-28 20:33:20+1100(AEDT)", true  },
        { "1997-03-16 02:40:00", "1997-03-16 13:40:00+1100(AEDT)", true  },
        { "1998-04-01 19:46:40", "1998-04-02 05:46:40+1000(AEST)", false },
        { "1999-04-18 12:53:20", "1999-04-18 22:53:20+1000(AEST)", false },
        { "2000-05-04 06:00:00", "2000-05-04 16:00:00+1000(AEST)", false },
        { "2001-05-20 23:06:40", "2001-05-21 09:06:40+1000(AEST)", false },
        { "2002-06-06 16:13:20", "2002-06-07 02:13:20+1000(AEST)", false },
        { "2003-06-23 09:20:00", "2003-06-23 19:20:00+1000(AEST)", false },
        { "2004-07-09 02:26:40", "2004-07-09 12:26:40+1000(AEST)", false },
        { "2005-07-25 19:33:20", "2005-07-26 05:33:20+1000(AEST)", false },
        { "2006-08-11 12:40:00", "2006-08-11 22:40:00+1000(AEST)", false },
        { "2007-08-28 05:46:40", "2007-08-28 15:46:40+1000(AEST)", false },
        { "2008-09-12 22:53:20", "2008-09-13 08:53:20+1000(AEST)", false },
        { "2009-09-29 16:00:00", "2009-09-30 02:00:00+1000(AEST)", false },
        { "2010-10-16 09:06:40", "2010-10-16 20:06:40+1100(AEDT)", true  },
        { "2011-11-02 02:13:20", "2011-11-02 13:13:20+1100(AEDT)", true  },
        { "2012-11-17 19:20:00", "2012-11-18 06:20:00+1100(AEDT)", true  },
        { "2013-12-04 12:26:40", "2013-12-04 23:26:40+1100(AEDT)", true  },
        { "2014-12-21 05:33:20", "2014-12-21 16:33:20+1100(AEDT)", true  },
        { "2016-01-06 22:40:00", "2016-01-07 09:40:00+1100(AEDT)", true  },
        { "2017-01-22 15:46:40", "2017-01-23 02:46:40+1100(AEDT)", true  },
        { "2018-02-08 08:53:20", "2018-02-08 19:53:20+1100(AEDT)", true  },
        { "2019-02-25 02:00:00", "2019-02-25 13:00:00+1100(AEDT)", true  },
        { "2020-03-12 19:06:40", "2020-03-13 06:06:40+1100(AEDT)", true  },
        { "2021-03-29 12:13:20", "2021-03-29 23:13:20+1100(AEDT)", true  }
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

/**
 * @brief 测试固定的时区
 * @details 无历史的时区信息，适用于从未使用过夏令时的时区，比如国内
 */
void TestFixedTimezone()
{
    Util::PrintTitle("TestFixedTimezone");
    TimeZone tz(8 * 3600, "CST");
    TestCase cases[] =
    {
        { "1970-01-12 13:46:40", "1970-01-12 21:46:40+0800(CST)", false },
        { "1971-01-29 06:53:20", "1971-01-29 14:53:20+0800(CST)", false },
        { "1972-02-15 00:00:00", "1972-02-15 08:00:00+0800(CST)", false },
        { "1973-03-02 17:06:40", "1973-03-03 01:06:40+0800(CST)", false },
        { "1974-03-19 10:13:20", "1974-03-19 18:13:20+0800(CST)", false },
        { "1975-04-05 03:20:00", "1975-04-05 11:20:00+0800(CST)", false },
        { "1976-04-20 20:26:40", "1976-04-21 04:26:40+0800(CST)", false },
        { "1977-05-07 13:33:20", "1977-05-07 21:33:20+0800(CST)", false },
        { "1978-05-24 06:40:00", "1978-05-24 14:40:00+0800(CST)", false },
        { "1979-06-09 23:46:40", "1979-06-10 07:46:40+0800(CST)", false },
        { "1980-06-25 16:53:20", "1980-06-26 00:53:20+0800(CST)", false },
        { "1981-07-12 10:00:00", "1981-07-12 18:00:00+0800(CST)", false },
        { "1982-07-29 03:06:40", "1982-07-29 11:06:40+0800(CST)", false },
        { "1983-08-14 20:13:20", "1983-08-15 04:13:20+0800(CST)", false },
        { "1984-08-30 13:20:00", "1984-08-30 21:20:00+0800(CST)", false },
        { "1985-09-16 06:26:40", "1985-09-16 14:26:40+0800(CST)", false },
        { "1986-10-02 23:33:20", "1986-10-03 07:33:20+0800(CST)", false },
        { "1987-10-19 16:40:00", "1987-10-20 00:40:00+0800(CST)", false },
        { "1988-11-04 09:46:40", "1988-11-04 17:46:40+0800(CST)", false },
        { "1989-11-21 02:53:20", "1989-11-21 10:53:20+0800(CST)", false },
        { "1990-12-07 20:00:00", "1990-12-08 04:00:00+0800(CST)", false },
        { "1991-12-24 13:06:40", "1991-12-24 21:06:40+0800(CST)", false },
        { "1993-01-09 06:13:20", "1993-01-09 14:13:20+0800(CST)", false },
        { "1994-01-25 23:20:00", "1994-01-26 07:20:00+0800(CST)", false },
        { "1995-02-11 16:26:40", "1995-02-12 00:26:40+0800(CST)", false },
        { "1996-02-28 09:33:20", "1996-02-28 17:33:20+0800(CST)", false },
        { "1997-03-16 02:40:00", "1997-03-16 10:40:00+0800(CST)", false },
        { "1998-04-01 19:46:40", "1998-04-02 03:46:40+0800(CST)", false },
        { "1999-04-18 12:53:20", "1999-04-18 20:53:20+0800(CST)", false },
        { "2000-05-04 06:00:00", "2000-05-04 14:00:00+0800(CST)", false },
        { "2001-05-20 23:06:40", "2001-05-21 07:06:40+0800(CST)", false },
        { "2002-06-06 16:13:20", "2002-06-07 00:13:20+0800(CST)", false },
        { "2003-06-23 09:20:00", "2003-06-23 17:20:00+0800(CST)", false },
        { "2004-07-09 02:26:40", "2004-07-09 10:26:40+0800(CST)", false },
        { "2005-07-25 19:33:20", "2005-07-26 03:33:20+0800(CST)", false },
        { "2006-08-11 12:40:00", "2006-08-11 20:40:00+0800(CST)", false },
        { "2007-08-28 05:46:40", "2007-08-28 13:46:40+0800(CST)", false },
        { "2008-09-12 22:53:20", "2008-09-13 06:53:20+0800(CST)", false },
        { "2009-09-29 16:00:00", "2009-09-30 00:00:00+0800(CST)", false },
        { "2010-10-16 09:06:40", "2010-10-16 17:06:40+0800(CST)", false },
        { "2011-11-02 02:13:20", "2011-11-02 10:13:20+0800(CST)", false },
        { "2012-11-17 19:20:00", "2012-11-18 03:20:00+0800(CST)", false },
        { "2013-12-04 12:26:40", "2013-12-04 20:26:40+0800(CST)", false },
        { "2014-12-21 05:33:20", "2014-12-21 13:33:20+0800(CST)", false },
        { "2016-01-06 22:40:00", "2016-01-07 06:40:00+0800(CST)", false },
        { "2017-01-22 15:46:40", "2017-01-22 23:46:40+0800(CST)", false },
        { "2018-02-08 08:53:20", "2018-02-08 16:53:20+0800(CST)", false },
        { "2019-02-25 02:00:00", "2019-02-25 10:00:00+0800(CST)", false },
        { "2020-03-12 19:06:40", "2020-03-13 03:06:40+0800(CST)", false },
        { "2021-03-29 12:13:20", "2021-03-29 20:13:20+0800(CST)", false }
    };

    for (const auto& c : cases)
    {
        SingleCaseTest(tz, c);
    }
}

int main()
{
    GenerateTestCase();
    // TestConvert();
    // TestUTC();
    // TestLocal();
    // TestNewYork();
    // TestLondon();
    // TestHongKong();
    // TestSydney();
    // TestFixedTimezone();

    return 0;
}