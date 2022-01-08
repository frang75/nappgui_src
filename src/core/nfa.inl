/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nfa.inl
 *
 */

/* Non-deterministic finite automata */

#include "core.ixx"

__EXTERN_C

NFA *nfa_string(const char_t *str);

NFA *nfa_regex(const char_t *regex, const bool_t verbose);

void nfa_destroy(NFA **nfa);

void nfa_start(NFA *nfa);

bool_t nfa_next(NFA *nfa, const uint32_t codepoint);

bool_t nfa_accept(NFA *nfa);

__END_C

