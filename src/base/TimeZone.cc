#include <stdio.h>
#include <assert.h>
#include <endian.h>
#include <stdint.h>

#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "Date.h"
#include "TimeZone.h"
#include "noncopyable.h"

namespace mrpc
{
namespace details
{

// struct tm
// {
//     int tm_sec;		    // Seconds.	    [0-60] (1 闰秒)
//     int tm_min;		    // Minutes.	    [0-59]
//     int tm_hour;	        // Hours.	    [0-23]
//     int tm_mday;		    // Day.		    [1-31]
//     int tm_mon;			// Month.	    [0-11]
//     int tm_year;		    // Year - 1900. 
//     /**
//      * 星期，取值区间为 [0,6]，其中 0 代表星期天，1 代表星期一，以此类推 
//      */
//     int tm_wday;		    // Day of week.  [0-6]
//     /**
//      * 从每年的 1 月 1 日开始的天数，其中0代表 1 月 1 日，1 代表 1 月 2 日，以此类推
//      */
//     int tm_yday;		    // Days in year. [0-365]

//     /**
//      * 夏令时标识符
//      * 实行夏令时的时候，tm_isdst 为 1
//      * 不实行夏令时的进候，tm_isdst为 0
//      * 不了解情况时，tm_isdst()为负
//      */
//     int tm_isdst;	    // Daylight Saving Time(DST). [-1/0/1] 

//     long int tm_gmtoff;  // Seconds east of UTC.  
//     const char *tm_zone; // Timezone abbreviation. 
// };

struct Transition
{
    time_t gmtTime;
    time_t localTime;
    int localTimeIdx;

    Transition(time_t g, time_t l, int localIdx)
        : gmtTime(g), localTime(l), localTimeIdx(localIdx) { }
};

struct Comp
{
    /**
     * GMT: Greenwich Mean Time 格林尼治标准时间
     * GMT 是以英国格林尼治天文台观测结果得出的时间，过去被当成世界标准的时间
     * 
     * UT: Universal Time 世界时，根据原子钟计算出来的时间
     * 
     * UTC: Coordinated Universal Time 协调世界时
     * 因为地球自转越来越慢，每年都会比前一年多出零点几秒，每隔几年协调世界时
     * 组织都会给世界时 +1 秒让基于原子钟的世界时和基于天文学（人类感知）的格
     * 林尼治标准时间相差不至于太大，并将得到的时间称为UTC，这是现在使用的世界标准时间。
     * 
     * PS: 协调世界时不与任何地区位置相关，也不代表此刻某地的时间，所以如果要说明某地时间，
     * 需要加上时区。也就是说 GMT 并不等于 UTC，而是等于 UTC + 0，因为格林尼治刚好在 0 时区
     */

    bool compareGMT;  // 选择比较 GMT 时间或者 local 时间

    Comp(bool gmt) : compareGMT(gmt) { }

    bool operator()(const Transition& lhs, const Transition& rhs) const
    {
        if(compareGMT)
            return lhs.gmtTime < rhs.gmtTime;
        else
            return lhs.localTime < rhs.localTime;
    }  
    
    bool equal(const Transition& lhs, const Transition& rhs) const
    {
        if(compareGMT)
            return lhs.gmtTime == rhs.gmtTime;
        else
            return lhs.localTime == rhs.localTime;   
    }
};

struct LocalTime
{
    time_t gmtOffset;
    bool isDst;
    int arrbIdx;
    
    LocalTime(time_t offset, bool dst, int arrb)
        : gmtOffset(offset), isDst(dst), arrbIdx(arrb) { }
};

inline void FillHMS(unsigned seconds, struct tm* utc)
{
    utc->tm_sec = seconds % 60;
    unsigned minutes = seconds / 60;
    utc->tm_min = minutes % 60;
    utc->tm_hour = minutes / 60;
}

}  // namespace details

const int g_secondsPerDay = 24 * 60 * 60;

struct TimeZone::Data
{
    std::vector<details::Transition> transitions;
    std::vector<details::LocalTime> localtimes;
    std::vector<std::string> names;
    /// 时区名称缩写（e.g., CST GMT)
    std::string abbreviation;
};

namespace details
{

class File : noncopyable
{
public:
    File(const char* file)
        : m_fp(::fopen(file, "rb")) { }

    ~File()
    {
        if(m_fp)
        {
            ::fclose(m_fp);
        }
    }

    std::string readBytes(int n)
    {
        char buf[n];
        ssize_t read = ::fread(buf, 1, n, m_fp);
        if(read != n)
            throw std::logic_error("no enough data");

        return std::string(buf, n);
    }

    int32_t readInt32()
    {
        int32_t x = 0;
        ssize_t read = ::fread(&x, 1, sizeof(int32_t), m_fp);
        if(read != sizeof(int32_t))
            throw std::logic_error("bad int32_t data");
        /**
         * #include <endian.h>
         * uint16_t htobe16(uint16_t host_16bits);
         * uint16_t htole16(uint16_t host_16bits);
         * uint16_t be16toh(uint16_t big_endian_16bits);
         * uint16_t le16toh(uint16_t little_endian_16bits);
         * 
         * uint32_t htobe32(uint32_t host_32bits);
         * uint32_t htole32(uint32_t host_32bits);
         * uint32_t be32toh(uint32_t big_endian_32bits);
         * uint32_t le32toh(uint32_t little_endian_32bits);
         *
         * uint64_t htobe64(uint64_t host_64bits);
         * uint64_t htole64(uint64_t host_64bits);
         * uint64_t be64toh(uint64_t big_endian_64bits);
         * uint64_t le64toh(uint64_t little_endian_64bits);
         * 
         * 这些函数将整数值的字节编码从当前CPU（"主机"）使用的字节顺序转换为little-endian和big-endian字节顺序。
         * 每个函数名称中的数字nn表示该函数处理的整数的大小，可以是16位，32位或64位。  
         * 
         * 名称格式为" htobe nn"的函数将从主机字节顺序转换为大端顺序。
         * 名称格式为" htole nn"的函数将从主机字节顺序转换为小端顺序。
         * 名称形式为" be nn toh"的函数将从big-endian顺序转换为主机字节顺序。
         * 名称形式为" le nn toh"的函数会从little-endian顺序转换为主机字节顺序。
         */
        return be32toh(x);
    }

    uint8_t readUInt8()
    {
        uint8_t x = 0;
        ssize_t read = ::fread(&x, 1, sizeof(uint8_t), m_fp);
        if (read != sizeof(uint8_t))
            throw std::logic_error("bad uint8_t data");

        return x;
    }

    bool valid() const { return m_fp; }

private:
    FILE* m_fp;
}; 

/**
 * @brief 读取时区文件
 * @param[in] zoneFile 时区文件
 * @param[in] data 存储时区信息结构
 */
bool ReadTimeZoneFile(const char* zoneFile, struct TimeZone::Data* data)
{
    /**
     * use command 'info tzfile' to see the timezone information
     * Timezone information files begin with the magic characters "TZif" to
     * identify them as timezone information files, followed by a character
     * identifying the version of the file's format(as of 2005, either an
     * ASCII NUL ('\0') or a '2') followed by fifteen bytes containing zeros
     * reserved for future use, followed by six four-byte values of type long
     * written in a "standard" byte order (the high-order byte of the value is
     * written first).  These values are, in order:
     * 
     * 
     * 
     */
    File f(zoneFile);
    if(f.valid())
    {
        try
        {
            std::string head = f.readBytes(4);
            if(head != "TZif")
                throw std::logic_error("bad head");
            
            std::string version = f.readBytes(1);
            f.readBytes(15);

            int32_t isgmtcnt = f.readInt32();
            int32_t isstdcnt = f.readInt32();
            int32_t leapcnt = f.readInt32();
            int32_t timecnt = f.readInt32();
            int32_t typecnt = f.readInt32();
            int32_t charcnt = f.readInt32();

            std::vector<int32_t> trans;
            std::vector<int> localTimes;
            trans.reserve(timecnt);
            for(int i = 0; i < timecnt; ++i)
            {
                trans.push_back(f.readInt32());
            }

            for(int i = 0; i < timecnt; ++i)
            {
                uint8_t local = f.readUInt8();
                localTimes.push_back(local);
            }

            for(int i = 0; i < typecnt; ++i)
            {
                int32_t gmtoff = f.readInt32();
                uint8_t isdst = f.readUInt8();
                uint8_t abbrind = f.readUInt8();

                data->localtimes.push_back(LocalTime(gmtoff, isdst, abbrind));
            }
        
            for(int i = 0; i < timecnt; ++i)
            {
                int localIdx = localTimes[i];
                time_t localtime = trans[i] + data->localtimes[localIdx].gmtOffset;
                data->transitions.push_back(Transition(trans[i], localtime, localIdx));
            }

            data->abbreviation = f.readBytes(charcnt);
        }
        catch(std::logic_error& e)
        {
            fprintf(stderr, "%s\n", e.what());
        }
    }
    return true;
}

const LocalTime* FindLocalTime(const TimeZone::Data& data, Transition sentry, Comp comp)
{
    const LocalTime* local = nullptr;

    if(data.transitions.empty() || comp(sentry, data.transitions.front()))
    {
        local = &data.localtimes.front();
    }
    else
    {
        std::vector<Transition>::const_iterator itr = lower_bound(data.transitions.begin(),
                                                                  data.transitions.end(),
                                                                  sentry,
                                                                  comp);
        if(itr != data.transitions.end())
        {
            if(!comp.equal(sentry, *itr))
            {
                assert(itr != data.transitions.begin());
                --itr;
            }
            local = &data.localtimes[itr->localTimeIdx];
        }
        else
        {
            local = &data.localtimes[data.transitions.back().localTimeIdx];
        }
    }

    return local;
}

}  // namespace details

TimeZone::TimeZone(const char* zoneFile)
    : m_data(new TimeZone::Data)
{
    if(!details::ReadTimeZoneFile(zoneFile, m_data.get()))
    {
        m_data.reset();
    }
}

TimeZone::TimeZone(int eastOfUTC, const char* name)
    : m_data(new TimeZone::Data)
{
    m_data->localtimes.push_back(details::LocalTime(eastOfUTC, false, 0));
    m_data->abbreviation = name;
}

struct tm TimeZone::toLocalTime(time_t secondsSinceEpoch) const 
{
    struct tm localTm;
    memZero(&localTm, sizeof(localTm));
    assert(m_data != nullptr);
    const Data& data(*m_data);

    details::Transition sentry(secondsSinceEpoch, 0, 0);
    const details::LocalTime* local = FindLocalTime(data, sentry, details::Comp(true));

    if(local)
    {
        time_t localSeconds = secondsSinceEpoch + local->gmtOffset;
        ::gmtime_r(&localSeconds, &localTm);
        localTm.tm_isdst = local->isDst;
        localTm.tm_gmtoff = local->gmtOffset;
        localTm.tm_zone = &data.abbreviation[local->arrbIdx];
    }

    return localTm;
}

time_t TimeZone::fromLocalTime(const struct tm& localTm) const 
{
    assert(m_data != nullptr);
    const Data& data(*m_data);

    struct tm tmp = localTm;
    time_t seconds = ::timegm(&tmp);
    details::Transition sentry(0, seconds, 0);
    const details::LocalTime* local = FindLocalTime(data, sentry, details::Comp(false));
    if (localTm.tm_isdst)
    {
        struct tm tryTm = toLocalTime(seconds - local->gmtOffset);
        if (!tryTm.tm_isdst
            && tryTm.tm_hour == localTm.tm_hour
            && tryTm.tm_min == localTm.tm_min)
        {
            // FIXME: HACK
            seconds -= 3600;
        }
    }
    return seconds - local->gmtOffset;
}

struct tm TimeZone::toUtcTime(time_t secondsSinceEpoch, bool yday)
{
    struct tm utc;
    memZero(&utc, sizeof(utc));
    utc.tm_zone = "GMT";
    int seconds = static_cast<int>(secondsSinceEpoch % g_secondsPerDay);
    int days = static_cast<int>(secondsSinceEpoch / g_secondsPerDay);
    if(seconds < 0)
    {
        seconds += g_secondsPerDay;
        --days;
    }
    details::FillHMS(seconds, &utc);
    Date date(days + Date::s_julianDayOf1970_01_01);
    Date::YearMonthDay ymd = date.yearMonthDay();
    utc.tm_year = ymd.year - 1900;
    utc.tm_mon = ymd.month - 1;
    utc.tm_mday = ymd.day;
    utc.tm_wday = date.weekDay();
    if(yday)
    {
        Date startOfYear(ymd.year, 1, 1);
        utc.tm_yday = date.julianDayNumber() - startOfYear.julianDayNumber();
    }
    return utc;
}

time_t TimeZone::fromUtcTime(const struct tm& utc)
{
    return fromUtcTime(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
                       utc.tm_hour, utc.tm_min, utc.tm_sec);
}

time_t TimeZone::fromUtcTime(int year, int month, int day,
                             int hour, int minute, int seconds)
{
    Date date(year, month, day);
    int secondsInDay = hour * 3600 + minute * 60 + seconds;
    time_t days = date.julianDayNumber() - Date::s_julianDayOf1970_01_01;
    return days * g_secondsPerDay + secondsInDay;
}

}  // namespace mrpc
