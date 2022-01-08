/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: regex.h
 * https://nappgui.com/en/core/regex.html
 *
 */

/* Regular expresions */

#include "core.hxx"

__EXTERN_C

RegEx *regex_create(const char_t *pattern);

void regex_destroy(RegEx **regex);

bool_t regex_match(const RegEx *regex, const char_t *str);

__END_C
