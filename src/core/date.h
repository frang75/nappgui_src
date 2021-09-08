/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: date.h
 * https://nappgui.com/en/core/date.html
 *
 */

/* Dates */

#include "core.hxx"

__EXTERN_C

Date date_system(void);

Date date_add_seconds(const Date *date, int32_t seconds);

Date date_add_minutes(const Date *date, int32_t minutes);

Date date_add_hours(const Date *date, int32_t hours);

Date date_add_days(const Date *date, int32_t days);

int16_t date_year(void);

int date_cmp(const Date *date1, const Date *date2);

bool_t date_between(const Date *date, const Date *from, const Date *to);

bool_t date_is_null(const Date *date);

String *date_DD_MM_YYYY_HH_MM_SS(const Date *date);

String *date_YYYY_MM_DD_HH_MM_SS(const Date *date);

const char_t *date_month_en(const month_t month);

const char_t *date_month_es(const month_t month);

extern Date kDATE_NULL;

__END_C
