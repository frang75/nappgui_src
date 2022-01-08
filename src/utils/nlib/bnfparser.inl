/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bnfparser.inl
 *
 */

/* Parser generator from BNF */

#include "nlib.ixx"

__EXTERN_C

bool_t bnfparser_create(Stream *stm, const char_t *dest_file, String **error);

__END_C

