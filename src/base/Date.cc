#include <time.h>
#include <stdio.h>

#include "Date.h"

namespace mrpc
{
namespace details
{

char require_32_bit_integer_at_least[sizeof(int) >= sizeof(int32_t) ? 1 : -1]; 

/// algorithm and explanation see:
/// http://www.faqs.org/faqs/calendars/faq/part2/

int GetJulianDayNumber(int year, int month, int day)
{
    (void) require_32_bit_integer_at_least; // no warning please
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    return day + (153*m + 2) / 5 + y*365 + y/4 - y/100 + y/400 - 32045;
}

struct Date::YearMonthDay GetYearMonthDay(int julianDayNumber)
{
    int a = julianDayNumber + 32044;
    int b = (4 * a + 3) / 146097;
    int c = a - ((b * 146097) / 4);
    int d = (4 * c + 3) / 1461;
    int e = c - ((1461 * d) / 4);
    int m = (5 * e + 2) / 153;
    Date::YearMonthDay ymd;
    ymd.day = e - ((153 * m + 2) / 5) + 1;
    ymd.month = m + 3 - 12 * (m / 10);
    ymd.year = b * 100 + d - 4800 + (m / 10);
    return ymd;
}

}  // namespace details

const int Date::s_julianDayOf1970_01_01 = details::GetJulianDayNumber(1970, 1, 1);

Date::Date(int y, int m, int d)
    : m_julianDayNumber(details::GetJulianDayNumber(y, m, d)) { }

Date::Date(const struct tm& t)
    : m_julianDayNumber(
        details::GetJulianDayNumber(
            t.tm_year + 1900,
            t.tm_mon + 1,
            t.tm_mday
        )
    ) { }

std::string Date::toIsoString() const
{
    char buf[32];
    YearMonthDay ymd(yearMonthDay());
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d", ymd.year, ymd.month, ymd.day);
    return buf;
}

Date::YearMonthDay Date::yearMonthDay() const 
{
    return details::GetYearMonthDay(m_julianDayNumber);
}

}  // namespace mrpc
