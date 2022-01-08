/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: msgparser.inl
 *
 */

/* Message files parser */

#include "nlib.ixx"

__EXTERN_C

void msgparser_process(const char_t *filepath, const char_t *file_data, const uint32_t file_size, ArrPt(String) **ids, ArrPt(String) **texts, ArrPt(String) *errors);

__END_C

