#ifndef __MRPC_BASE_TIMEZONE_H__
#define __MRPC_BASE_TIMEZONE_H__

#include <time.h>

#include <memory>

#include "copyable.h"

namespace mrpc
{
/**
 * @brief 时区类
 */
class TimeZone : public copyable
{
public:
    /**
     * @brief 构造函数，通过系统时区文件构造时区类
     * @param[in] zoneFile 时区文件 (e.g.,/usr/share/zoneinfo/America/New_York)
     * @details zdump -v /etc/localtime 查看历史时区信息
     */
    explicit TimeZone(const char* zoneFile);
    /**
     * @brief 构造函数，通过时差可时区缩写构造时区类
     * @param[in] eastOfUTC 东部时差 
     * @param[in] tzname 时区缩写
     */
    TimeZone(int eastOfUTC, const char* tzname);
    TimeZone() = default;

    bool valid() const 
    {
        return static_cast<bool>(m_data);
    }

    /**
     * @brief 由 UTC 纪元时间获取 TimeZone 时区对应的 tm 结构体
     * @param secondsSinceEpoch 由 time(2) 获取的纪元时
     */
    struct tm toLocalTime(time_t secondsSinceEpoch) const;

    /**
     * @brief 将 TimeZone 时间对应的 tm 结构体转换到 UTC 纪元时间
     */
    time_t fromLocalTime(const struct tm&) const;

    /**
     * @brief 由 UTC 纪元时间获取 TimeZone时区的 tm 日历时间
     * @param[in] secondsSinceEpoch 纪元时间
     * @param[in] yday 是否填充 tm_yday 每年天数
     */
    static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);

    /**
     * @brief 由 UTC tm 结构体获取 UTC 纪元时间
     */
    static time_t fromUtcTime(const struct tm&);

    /**
     * @brief 由各个时间参数获取 UTC 纪元时间
     */
    static time_t fromUtcTime(int year, int month, int day,
                              int hour, int minute, int seconds);

    struct Data;

private:
    std::shared_ptr<Data> m_data;
};

}  // namespace mrpc

#endif  // __MRPC_BASE_TIMEZONE_H__