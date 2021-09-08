/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: btime.h
 * https://nappgui.com/en/osbs/btime.html
 *
 */

/* Basic time services */

#include "osbs.hxx"

__EXTERN_C

uint64_t btime_now(void);

void btime_date(Date *date);

uint64_t btime_to_micro(const Date *date);

void btime_to_date(const uint64_t micro, Date *date);

__END_C
