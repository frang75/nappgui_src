/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: regex.c
 *
 */

/* Regular expresions */

#include "regex.h"
#include "nfa.inl"

/*---------------------------------------------------------------------------*/

RegEx *regex_create(const char_t *pattern, rerror_t *error, uint32_t *errpos)
{
    return cast(_nfa_regex(pattern, FALSE, error, errpos), RegEx);
}

/*---------------------------------------------------------------------------*/

void regex_destroy(RegEx **regex)
{
    _nfa_destroy(dcast(regex, NFA));
}

/*---------------------------------------------------------------------------*/

bool_t regex_match(const RegEx *regex, const char_t *str)
{
    return _nfa_match(cast_const(regex, NFA), str);
}

/*---------------------------------------------------------------------------*/

bool_t regex_search(const RegEx *regex, const char_t *str, uint32_t *pos, uint32_t *len)
{
    return _nfa_search(cast_const(regex, NFA), str, pos, len);
}
