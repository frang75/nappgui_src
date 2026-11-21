/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: btime.c
 *
 */

/* Basic time services */

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include "../btime.h"
#include <sewer/blib.h>
#include <sewer/cassert.h>

#if !defined(__LINUX__)
#error This file is only for Linux
#endif

#include <langinfo.h>
#include <locale.h>

/*---------------------------------------------------------------------------*/

static void i_langinfo(const nl_item item, char_t *name, const uint32_t size)
{
    locale_t loc = newlocale(LC_TIME_MASK, "", (locale_t)0);
    const char *str = NULL;
    cassert_no_null(name);

    if (loc != (locale_t)0)
        str = nl_langinfo_l(item, loc);
    else
        str = nl_langinfo(item);

    blib_strcpy(name, size, cast_const(str, char_t));

    if (loc != (locale_t)0)
        freelocale(loc);
}

/*---------------------------------------------------------------------------*/

week_day_t btime_first_wday(void)
{
#if defined(__GLIBC__)
    locale_t loc = newlocale(LC_TIME_MASK, "", (locale_t)0);
    week_day_t wday = ekSUNDAY;

    if (loc != (locale_t)0)
    {
        union
        {
            unsigned int word;
            char *string;
        } info;
        uint32_t first = 0;
        uint32_t origin = 0;
        uint32_t week_1stday = 0;
        info.string = nl_langinfo_l(_NL_TIME_FIRST_WEEKDAY, loc);
        first = (uint32_t)info.string[0];
        info.string = nl_langinfo_l(_NL_TIME_WEEK_1STDAY, loc);
        origin = (uint32_t)info.word;

        if (origin == 19971201)
            week_1stday = 1;

        if (first >= 1 && first <= 7)
            wday = (week_day_t)((week_1stday + first - 1) % 7);

        freelocale(loc);
    }

    return wday;
#else
    return ekSUNDAY;
#endif
}

/*---------------------------------------------------------------------------*/

void btime_month_name(const month_t month, char_t *name, const uint32_t size)
{
    nl_item item = 0;
    cassert(month >= ekJANUARY && month <= ekDECEMBER);
#if defined(ALTMON_1)
    item = (nl_item)(ALTMON_1 + (int)month - 1);
#else
    item = (nl_item)(MON_1 + (int)month - 1);
#endif
    i_langinfo(item, name, size);
}

/*---------------------------------------------------------------------------*/

void btime_wday_name(const week_day_t wday, char_t *name, const uint32_t size)
{
    nl_item item = (nl_item)(ABDAY_1 + (int)wday);
    cassert((uint32_t)wday <= 6);
    i_langinfo(item, name, size);
}
