/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: date.c
 *
 */

/* Dates */

#include "date.h"
#include "strings.h"
#include <osbs/btime.h>
#include <sewer/blib.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

Date kDATE_NULL = {0, 0, 0, 0, 0, 0, 0};
static const int32_t i_MIN_DAYS = -12687794;
static const int32_t i_MAX_DAYS = 11248737;

/*---------------------------------------------------------------------------*/

static int32_t i_days(const int32_t year, const int32_t month, const int32_t mday)
{
    int32_t y = year;
    int32_t era = 0;
    int32_t yoe = 0;
    int32_t mp = 0;
    int32_t doy = 0;
    int32_t doe = 0;

    if (month <= 2)
        y -= 1;

    if (y >= 0)
        era = y / 400;
    else
        era = (y - 399) / 400;

    yoe = y - era * 400;

    if (month > 2)
        mp = month - 3;
    else
        mp = month + 9;

    doy = (153 * mp + 2) / 5 + mday - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

/*---------------------------------------------------------------------------*/

static void i_civil(const int32_t days, int32_t *year, int32_t *month, int32_t *mday)
{
    int32_t z = days + 719468;
    int32_t era = 0;
    int32_t doe = 0;
    int32_t yoe = 0;
    int32_t doy = 0;
    int32_t mp = 0;

    cassert_no_null(year);
    cassert_no_null(month);
    cassert_no_null(mday);

    if (z >= 0)
        era = z / 146097;
    else
        era = (z - 146096) / 146097;

    doe = z - era * 146097;
    yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    mp = (5 * doy + 2) / 153;
    *year = yoe + era * 400;
    *mday = doy - (153 * mp + 2) / 5 + 1;

    if (mp < 10)
        *month = mp + 3;
    else
        *month = mp - 9;

    if (*month <= 2)
        *year += 1;
}

/*---------------------------------------------------------------------------*/

static week_day_t i_wday(const int32_t days)
{
    int32_t r = days % 7;
    if (r < 0)
        r += 7;
    return (week_day_t)((r + 4) % 7);
}

/*---------------------------------------------------------------------------*/

Date date_system(void)
{
    Date date;
    btime_date(&date);
    return date;
}

/*---------------------------------------------------------------------------*/

Date date_pack(const int16_t year, const uint8_t month, const uint8_t mday, const uint8_t hour, const uint8_t minute, const uint8_t second)
{
    Date date;
    date.year = year;
    date.month = month;
    date.mday = mday;
    date.hour = hour;
    date.minute = minute;
    date.second = second;
    date.wday = date_weekday(&date);
    return date;
}

/*---------------------------------------------------------------------------*/

static Date i_add_micros(const Date *date, int64_t micros)
{
    uint64_t dmicros = btime_to_micro(date);
    Date ndate;

    if (micros > 0)
        dmicros += (uint64_t)micros;
    else
        dmicros -= (uint64_t)(-micros);

    btime_to_date(dmicros, &ndate);
    return ndate;
}

/*---------------------------------------------------------------------------*/

Date date_add_seconds(const Date *date, int32_t seconds)
{
    return i_add_micros(date, (int64_t)seconds * 1000000);
}

/*---------------------------------------------------------------------------*/

Date date_add_minutes(const Date *date, int32_t minutes)
{
    return i_add_micros(date, (int64_t)minutes * 60 * 1000000);
}

/*---------------------------------------------------------------------------*/

Date date_add_hours(const Date *date, int32_t hours)
{
    return i_add_micros(date, (int64_t)hours * 60 * 60 * 1000000);
}

/*---------------------------------------------------------------------------*/

Date date_add_days(const Date *date, int32_t days)
{
    return i_add_micros(date, (int64_t)days * 24 * 60 * 60 * 1000000);
}

/*---------------------------------------------------------------------------*/

int16_t date_year(void)
{
    Date date;
    btime_date(&date);
    return date.year;
}

/*---------------------------------------------------------------------------*/

int date_cmp(const Date *date1, const Date *date2)
{
    cassert_no_null(date1);
    cassert_no_null(date2);
    if (date1->year < date2->year)
        return -1;
    if (date1->year > date2->year)
        return 1;
    if (date1->month < date2->month)
        return -1;
    if (date1->month > date2->month)
        return 1;
    if (date1->mday < date2->mday)
        return -1;
    if (date1->mday > date2->mday)
        return 1;
    if (date1->hour < date2->hour)
        return -1;
    if (date1->hour > date2->hour)
        return 1;
    if (date1->minute < date2->minute)
        return -1;
    if (date1->minute > date2->minute)
        return 1;
    if (date1->second < date2->second)
        return -1;
    if (date1->second > date2->second)
        return 1;
    return 0;
}

/*---------------------------------------------------------------------------*/

int64_t date_ellapsed_seconds(const Date *from, const Date *to)
{
    uint64_t from_micros = btime_to_micro(from);
    uint64_t to_micros = btime_to_micro(to);

    if (to_micros > from_micros)
        return (int64_t)((to_micros - from_micros) / 1000000);
    else
        return -(int64_t)((from_micros - to_micros) / 1000000);
}

/*---------------------------------------------------------------------------*/

bool_t date_between(const Date *date, const Date *from, const Date *to)
{
    int c = date_cmp(to, from);
    int f = date_cmp(date, from);
    int t = date_cmp(date, to);

    /* 'from' is more recent date --> swap */
    if (c < 0)
    {
        int s = f;
        f = t;
        t = s;
    }

    if (f >= 0 && t <= 0)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t date_is_null(const Date *date)
{
    cassert_no_null(date);
    if (date->year == 0 && date->month == 0 && date->wday == 0 && date->mday == 0 && date->hour == 0 && date->minute == 0 && date->second == 0)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t date_is_valid(const Date *date)
{
    cassert_no_null(date);
    if (date->month == 0 || date->month > 12)
        return FALSE;
    if (date->mday == 0 || date->mday > date_month_days(date->year, (month_t)date->month))
        return FALSE;
    if (date->hour >= 24)
        return FALSE;
    if (date->minute >= 60)
        return FALSE;
    if (date->second >= 60)
        return FALSE;

    return TRUE;
}

/*---------------------------------------------------------------------------*/

week_day_t date_weekday(const Date *date)
{
    int32_t days = 0;
    cassert_no_null(date);
    cassert(date->mday >= 1 && date->mday <= date_month_days(date->year, (month_t)date->month));
    days = i_days((int32_t)date->year, (int32_t)date->month, (int32_t)date->mday);
    return i_wday(days);
}

/*---------------------------------------------------------------------------*/

bool_t date_is_leap(const int16_t year)
{
    if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

uint8_t date_month_days(const int16_t year, const month_t month)
{
    switch (month)
    {
    case ekJANUARY:
    case ekMARCH:
    case ekMAY:
    case ekJULY:
    case ekAUGUST:
    case ekOCTOBER:
    case ekDECEMBER:
        return 31;
    case ekAPRIL:
    case ekJUNE:
    case ekSEPTEMBER:
    case ekNOVEMBER:
        return 30;
    case ekFEBRUARY:
        if (date_is_leap(year) == TRUE)
            return 29;
        else
            return 28;
    default:
        cassert_default(month);
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

int32_t date_to_days(const Date *date)
{
    cassert_no_null(date);
    cassert(date->month >= 1 && date->month <= 12);
    cassert(date->mday >= 1 && date->mday <= date_month_days(date->year, (month_t)date->month));
    return i_days((int32_t)date->year, (int32_t)date->month, (int32_t)date->mday);
}

/*---------------------------------------------------------------------------*/

Date date_from_days(const int32_t days)
{
    int32_t cdays = days;
    int32_t year = 0;
    int32_t month = 0;
    int32_t mday = 0;
    Date date;

    if (cdays < i_MIN_DAYS)
        cdays = i_MIN_DAYS;
    else if (cdays > i_MAX_DAYS)
        cdays = i_MAX_DAYS;

    i_civil(cdays, &year, &month, &mday);
    date.year = (int16_t)year;
    date.month = (uint8_t)month;
    date.wday = (uint8_t)i_wday(cdays);
    date.mday = (uint8_t)mday;
    date.hour = 0;
    date.minute = 0;
    date.second = 0;
    return date;
}

/*---------------------------------------------------------------------------*/

String *date_format(const Date *date, const char_t *format)
{
    char_t dest[256];
    cassert_no_null(date);
    if (blib_strftime(dest, sizeof(dest), format, date->year, date->month, date->mday, date->wday, date->hour, date->minute, date->second) != 0)
        return str_c(dest);
    else
        return str_printf("%04d/%02d/%02d-%02d:%02d:%02d", date->year, date->month, date->mday, date->hour, date->minute, date->second);
}
