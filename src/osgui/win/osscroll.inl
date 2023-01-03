/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osscroll.inl
 *
 */

/* Windows scrolled view logic */

#include "osgui_win.ixx"

__EXTERN_C

OSScroll *osscroll_create(HWND hwnd, const bool_t horizontal, const bool_t vertical);

void osscroll_destroy(OSScroll **scroll);

void osscroll_visible_area(OSScroll *scroll, int *x, int *y, int *width, int *height, int *total_width, int *total_height);

int osscroll_x_pos(const OSScroll *scroll);

int osscroll_y_pos(const OSScroll *scroll);

int osscroll_bar_width(const OSScroll *scroll, const bool_t check_if_visible);

int osscroll_bar_height(const OSScroll *scroll, const bool_t check_if_visible);

bool_t osscroll_wheel(OSScroll *scroll, WPARAM wParam, const bool_t update_children);

void osscroll_message(OSScroll *scroll, WPARAM wParam, UINT nMsg, const bool_t update_children);

void osscroll_set(OSScroll *scroll, const int x, const int y, const bool_t update_children);

void osscroll_content_size(OSScroll *scroll, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height);

void osscroll_control_size(OSScroll *scroll, const real32_t width, const real32_t height);

__END_C

