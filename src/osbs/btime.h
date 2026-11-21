/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: btime.h
 * https://www.nappgui.com/en/osbs/btime.html
 *
 */

/* Basic time services */

#include "osbs.hxx"

__EXTERN_C

_osbs_api uint64_t btime_now(void);

_osbs_api void btime_date(Date *date);

_osbs_api uint64_t btime_to_micro(const Date *date);

_osbs_api void btime_to_date(const uint64_t micro, Date *date);

_osbs_api week_day_t btime_first_wday(void);

_osbs_api void btime_month_name(const month_t month, char_t *name, const uint32_t size);

_osbs_api void btime_wday_name(const week_day_t wday, char_t *name, const uint32_t size);

__END_C
