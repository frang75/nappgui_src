/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: btime.m
 *
 */

/* Basic time services */

#include "../btime.h"

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

#include <sewer/nowarn.hxx>
#include <Foundation/Foundation.h>
#include <sewer/warn.hxx>

#include <sewer/blib.h>
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

week_day_t btime_first_wday(void)
{
    NSCalendar *calendar = [NSCalendar autoupdatingCurrentCalendar];
    NSUInteger first = [calendar firstWeekday];
    cassert(first >= 1 && first <= 7);
    return (week_day_t)(first - 1);
}

/*---------------------------------------------------------------------------*/

static void i_symbol(NSArray *symbols, const NSUInteger index, char_t *name, const uint32_t size)
{
    NSString *symbol = [symbols objectAtIndex:index];
    const char *utf8 = [symbol UTF8String];
    blib_strcpy(name, size, cast_const(utf8, char_t));
}

/*---------------------------------------------------------------------------*/

void btime_month_name(const month_t month, char_t *name, const uint32_t size)
{
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    NSArray *symbols = nil;
    cassert(month >= ekJANUARY && month <= ekDECEMBER);
    cassert_no_null(name);
    [formatter setLocale:[NSLocale autoupdatingCurrentLocale]];
    symbols = [formatter standaloneMonthSymbols];
    i_symbol(symbols, (NSUInteger)month - 1, name, size);
    [formatter release];
}

/*---------------------------------------------------------------------------*/

void btime_wday_name(const week_day_t wday, char_t *name, const uint32_t size)
{
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init];
    NSArray *symbols = nil;
    cassert((uint32_t)wday <= 6);
    cassert_no_null(name);
    [formatter setLocale:[NSLocale autoupdatingCurrentLocale]];
    symbols = [formatter veryShortStandaloneWeekdaySymbols];
    i_symbol(symbols, (NSUInteger)wday, name, size);
    [formatter release];
}
