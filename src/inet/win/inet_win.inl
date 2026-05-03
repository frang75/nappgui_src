/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: inet_win.inl
 *
 */

/* Inet Windows */

#include "inet_win.ixx"

__EXTERN_C

const WCHAR *wstring_init(const char_t *text, WString *str);

void wstring_remove(WString *str);

__END_C
