/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: regex.h
 * https://www.nappgui.com/en/core/regex.html
 *
 */

/* Regular expresions */

#include "core.hxx"

__EXTERN_C

_core_api RegEx *regex_create(const char_t *pattern, rerror_t *error, uint32_t *errpos);

_core_api void regex_destroy(RegEx **regex);

_core_api bool_t regex_match(const RegEx *regex, const char_t *str);

_core_api bool_t regex_search(const RegEx *regex, const char_t *str, uint32_t *pos, uint32_t *len);

__END_C
