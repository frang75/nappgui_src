/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.inl
 *
 */

/* Drawing custom GUI controls */

#include "draw2d.ixx"
#include "win/draw2d_gdi.ixx"
#include "osgui_win.ixx"

__EXTERN_C

void osdrawctrl_header(HWND hwnd, HDC hdc, HFONT font, const RECT *rect, int state, const WCHAR *text, const align_t align, const Image *image);

__END_C