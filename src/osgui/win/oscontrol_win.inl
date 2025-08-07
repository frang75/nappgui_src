/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol_win.inl
 *
 */

/* Windows controls commons */

#include "osgui_win.ixx"

__EXTERN_C

void _oscontrol_init(OSControl *control, const DWORD dwExStyle, const DWORD dwStyle, const LPCWSTR lpClassName, int nWidth, int nHeight, WNDPROC wndProc, HWND parent_window);

void _oscontrol_init_hidden(OSControl *control, const DWORD dwExStyle, const DWORD dwStyle, const LPCWSTR lpClassName, int nWidth, int nHeight, WNDPROC wndProc, HWND parent_window);

void _oscontrol_destroy(OSControl *control);

char_t *_oscontrol_get_text(const OSControl *control, uint32_t *tsize, uint32_t *nchars);

void _oscontrol_set_text(OSControl *control, const char_t *text);

void _oscontrol_set_tooltip(OSControl *control, const char_t *text);

void _oscontrol_set_font(OSControl *control, const Font *font);

void _oscontrol_update_font(OSControl *control, Font **current_font, const Font *font);

void _oscontrol_set_visible(OSControl *control, const bool_t visible);

void _oscontrol_set_enabled(OSControl *control, const bool_t enabled);

void _oscontrol_get_origin(const OSControl *control, real32_t *x, real32_t *y);

void _oscontrol_get_size(const OSControl *control, real32_t *width, real32_t *height);

void _oscontrol_set_frame(OSControl *control, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

void _oscontrol_attach_to_parent(OSControl *control, OSControl *parent_control);

void _oscontrol_detach_from_parent(OSControl *control, OSControl *parent_control);

uint32_t _oscontrol_num_children(const OSControl *control);

void _oscontrol_draw_focus(HWND hwnd, const INT left_offset, const INT right_offset, const INT top_offset, const INT bottom_offset);

DWORD _oscontrol_ss_halign(const align_t align);

DWORD _oscontrol_es_halign(const align_t align);

DWORD _oscontrol_ellipsis(const ellipsis_t ellipsis);

COLORREF _oscontrol_colorref(const color_t color);

color_t _oscontrol_from_colorref(const COLORREF color);

HBRUSH _oscontrol_ctl_color_edit(HDC hdc, COLORREF color, COLORREF bgcolor, HBRUSH brush, HBRUSH defbrush);

void _oscontrol_update_brush(const color_t color, HBRUSH *brush, COLORREF *colorref);

void _oscontrol_destroy_brush(HBRUSH *brush);

void _oscontrol_clipboard(HWND hwnd, const clipboard_t clipboard);

__END_C
