/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osscroll.inl
 *
 */

/* Operating System native scrollbar */

#include "osgui.ixx"

__EXTERN_C

OSScroll *_osscroll_horizontal(OSControl *control);

OSScroll *_osscroll_vertical(OSControl *control);

void _osscroll_destroy(OSScroll **scroll, OSControl *control);

uint32_t _osscroll_pos(const OSScroll *scroll);

uint32_t _osscroll_trackpos(const OSScroll *scroll);

uint32_t _osscroll_bar_width(const OSScroll *scroll);

uint32_t _osscroll_bar_height(const OSScroll *scroll);

void _osscroll_set_pos(OSScroll *scroll, const uint32_t pos);

void _osscroll_visible(OSScroll *scroll, const bool_t visible);

void _osscroll_config(OSScroll *scroll, const uint32_t pos, const uint32_t max, const uint32_t page);

void _osscroll_frame(OSScroll *scroll, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height);

void _osscroll_control_scroll(OSControl *control, const int32_t incr_x, const int32_t incr_y);

__END_C
