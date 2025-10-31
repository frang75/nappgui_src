/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw_win.inl
 *
 */

/* Drawing commands */

#include "draw2d_gdi.ixx"

__EXTERN_C

extern int kLOG_PIXY;

extern LONG kTWIPS_PER_PIXEL;

const WCHAR *wstring_init(const char_t *text, WString *str);

void wstring_remove(WString *str);

__END_C
