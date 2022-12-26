/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
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

_core_api Date date_system(void);

_core_api Date date_add_seconds(const Date *date, int32_t seconds);

_core_api Date date_add_minutes(const Date *date, int32_t minutes);

_core_api Date date_add_hours(const Date *date, int32_t hours);

_core_api Date date_add_days(const Date *date, int32_t days);

_core_api int16_t date_year(void);

_core_api int date_cmp(const Date *date1, const Date *date2);

_core_api bool_t date_between(const Date *date, const Date *from, const Date *to);

_core_api bool_t date_is_null(const Date *date);

_core_api String *date_DD_MM_YYYY_HH_MM_SS(const Date *date);

_core_api String *date_YYYY_MM_DD_HH_MM_SS(const Date *date);

_core_api const char_t *date_month_en(const month_t month);

_core_api const char_t *date_month_es(const month_t month);

_core_api extern Date kDATE_NULL;

__END_C
