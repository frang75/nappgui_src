/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nrclib.h
 *
 */

/* Resource compiler logic */

#include "nrc.ixx"

__EXTERN_C

void nrclib_serial_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors, bool_t *regenerated);

void nrclib_pack_dir(const char_t *src_dir, const char_t *dest_file, ArrPt(String) **warnings, ArrPt(String) **errors, bool_t *regenerated);

__END_C
