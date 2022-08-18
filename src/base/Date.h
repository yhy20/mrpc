#ifndef __MRPC_BASE_DATE_H__
#define __MRPC_BASE_DATE_H__

#include "Types.h"
#include "copyable.h"

struct tm;

namespace mrpc
{

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

    Date() : m_julianDayNumber(0) { }

    Date(int year, int month, int day);

    explicit Date(int julianDayNumber) 
        : m_julianDayNumber(julianDayNumber) { }

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