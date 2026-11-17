/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: nfa.inl
 *
 */

/* Non-deterministic finite automata */

#include "core.ixx"

__EXTERN_C

NFA *_nfa_regex(const char_t *regex, const bool_t verbose, rerror_t *error, uint32_t *errpos);

void _nfa_destroy(NFA **nfa);

bool_t _nfa_match(const NFA *nfa, const char_t *str);

bool_t _nfa_search(const NFA *nfa, const char_t *str, uint32_t *pos, uint32_t *len);

__END_C
