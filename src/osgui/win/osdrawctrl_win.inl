/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl_win.inl
 *
 */

/* Drawing custom GUI controls */

#include "osgui_win.ixx"

__EXTERN_C

void _osdrawctrl_gdi_measuse(HDC hdc, const char_t *text, INT *width, INT *height);

void _osdrawctrl_gdi_text(HDC hdc, HTHEME theme, const char_t *text, const int32_t x, const int32_t y, const align_t align, const ellipsis_t trim, const int32_t text_width, const color_t text_color, const ctrl_state_t state);

__END_C
