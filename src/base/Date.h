#ifndef __MRPC_BASE_DATE_H__
#define __MRPC_BASE_DATE_H__

#include "Types.h"
#include "copyable.h"

struct tm;

namespace mrpc
{
/**
 * @brief 日历类
 * @details Date 以 julianDayNumber 为基准进行计算，是对 tm 的一种替代
 * 
 */
class Date : public copyable
{
public:
    struct YearMonthDay
    {
        int year;   // [1900..2500]
        int month;  // [1..12]
        int day;    // [1..31]
    };

    static const int s_daysPerweek = 7;
    static const int s_julianDayOf1970_01_01;

    /**
     * @brief 构造一个非法的 Date
     */
    Date() : m_julianDayNumber(0) { }

    /**
     * @brief 以 yyyy-mm-dd 格式构造一个 Date
     * 
     * @param[in] year 年 
     * @param[in] month 月
     * @param[in] day 日
     */
    Date(int year, int month, int day);

    /**
     * @brief 以 Julian Day Number 构造一个 Date
     * @param[in] julianDayNumber Julian Day Number
     */
    explicit Date(int julianDayNumber) 
        : m_julianDayNumber(julianDayNumber) { }

    /**
     * @brief 通过 struct tm 构造一个 Date 
     */
    explicit Date(const struct tm&);

    void swap(Date& rhs)
    {
        std::swap(m_julianDayNumber, rhs.m_julianDayNumber);
    }

    bool valid() const { return m_julianDayNumber > 0; }

    /**
     * @brief Converts to yyyy-mm-dd format.
     */
    std::string toIsoString() const;

    /**
     * @brief 由 Date 返回 struct YearMonthDay
     */
    struct YearMonthDay yearMonthDay() const;
    
    int year() const
    {
        return yearMonthDay().year;
    }

    int month() const
    {
        return yearMonthDay().month;
    }

    int day() const
    {
        return yearMonthDay().day;
    }

    /**
     * @brief [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday]
     */
    int weekDay() const 
    {
        return (m_julianDayNumber + 1) % s_daysPerweek;
    }

    int julianDayNumber() const { return m_julianDayNumber; }

private:
    int m_julianDayNumber;

};

inline bool operator<(Date lhs, Date rhs)
{
    return lhs.julianDayNumber() < rhs.julianDayNumber();
}

inline bool operator==(Date lhs, Date rhs)
{
    return lhs.julianDayNumber() == rhs.julianDayNumber();
}

}  // namespace mrpc


#endif  // __MRPC_BASE_DATE_H__