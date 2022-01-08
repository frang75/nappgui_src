/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nrclib.h
 *
 */

/* Resource compiler logic */

#include "nlib.hxx"

__EXTERN_C

void nrclib_serial_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors);

void nrclib_pack_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors);

bool_t nrclib_bnfparser(const char_t *src_file, const char_t *dest_file, String **error);

__END_C
