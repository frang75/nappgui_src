/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.inl
 *
 */

/* Windows controls commons */

#include "osgui_win.ixx"

__EXTERN_C

void _oscontrol_init(OSControl *control, const DWORD dwExStyle, const DWORD dwStyle, const LPCWSTR lpClassName, int nWidth, int nHeight, WNDPROC wndProc, HWND parent_window);

void _oscontrol_init_hidden(OSControl *control, const DWORD dwExStyle, const DWORD dwStyle, const LPCWSTR lpClassName, int nWidth, int nHeight, WNDPROC wndProc, HWND parent_window);

void _oscontrol_destroy(OSControl *control);

WCHAR *_oscontrol_convers_text(const char_t *text_utf8, uint32_t *size);

void _oscontrol_destroy_text(WCHAR **text, const uint32_t size);

char_t *_oscontrol_get_text(const OSControl *control, uint32_t *tsize);

void _oscontrol_set_text(OSControl *control, const char_t *text);

void _oscontrol_set_tooltip(OSControl *control, const char_t *text);

void _oscontrol_set_tooltip_hwnd(OSControl *control, HWND hwnd, const char_t *text);

void _oscontrol_set_font(OSControl *control, const Font *font);

void _oscontrol_update_font(OSControl *control, Font **current_font, const Font *font);

DWORD _oscontrol_halign(const align_t align);

DWORD _oscontrol_ellipsis(const ellipsis_t ellipsis);

void _oscontrol_text_bounds(const OSControl *control, const char_t *text, const Font *font, const real32_t refwidth, real32_t *width, real32_t *height);

void _oscontrol_set_visible(OSControl *control, const bool_t visible);

void _oscontrol_set_enabled(OSControl *control, const bool_t enabled);

COLORREF _oscontrol_colorref(const color_t color);

color_t _oscontrol_from_colorref(const COLORREF color);

void _oscontrol_update_brush(const color_t color, HBRUSH *brush, COLORREF *colorref);

void _oscontrol_destroy_brush(HBRUSH *brush);

void _oscontrol_get_local_frame(HWND hwnd, RECT *rect);

void _oscontrol_get_origin(const OSControl *control, real32_t *x, real32_t *y);

void _oscontrol_get_origin_in_screen(HWND hwnd, real32_t *x, real32_t *y);

void _oscontrol_get_size(const OSControl *control, real32_t *width, real32_t *height);

void _oscontrol_set_position(OSControl *control, const int x, const int y);

void _oscontrol_set_size(HWND hwnd, const real32_t width, const real32_t height);

void _oscontrol_set_frame(OSControl *control, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

void _oscontrol_attach_to_parent(OSControl *control, OSControl *parent_control);

void _oscontrol_detach_from_parent(OSControl *control, OSControl *parent_control);

uint32_t _oscontrol_get_children(HWND hwnd, HWND *children, const uint32_t children_size);

uint32_t _oscontrol_num_children(HWND hwnd);

void _oscontrol_draw_focus(HWND hwnd, const INT left_offset, const INT right_offset, const INT top_offset, const INT bottom_offset);
//HWND _oscontrol_hit_point(const OSControl *control, const POINT *point);

__END_C

