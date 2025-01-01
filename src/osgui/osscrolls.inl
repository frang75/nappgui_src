/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osscrolls.inl
 *
 */

/* Scrollbars management in control */

#include "osgui.ixx"

__EXTERN_C

OSScrolls *_osscrolls_create(OSControl *control, const bool_t horizontal, const bool_t vertical);

void _osscrolls_destroy(OSScrolls **scroll);

void _osscrolls_OnScroll(OSScrolls *scroll, Listener *listener);

void _osscrolls_visible_area(OSScrolls *scroll, uint32_t *x, uint32_t *y, uint32_t *width, uint32_t *height, uint32_t *total_width, uint32_t *total_height);

uint32_t _osscrolls_x_pos(const OSScrolls *scroll);

uint32_t _osscrolls_y_pos(const OSScrolls *scroll);

uint32_t _osscrolls_bar_width(const OSScrolls *scroll, const bool_t check_if_visible);

uint32_t _osscrolls_bar_height(const OSScrolls *scroll, const bool_t check_if_visible);

bool_t _osscrolls_event(OSScrolls *scroll, const gui_orient_t orient, const gui_scroll_t event, const bool_t update_children);

void _osscrolls_set(OSScrolls *scroll, const uint32_t x, const uint32_t y, const bool_t update_children);

void _osscrolls_content_size(OSScrolls *scroll, const uint32_t width, const uint32_t height, const uint32_t line_width, const uint32_t line_height);

void _osscrolls_control_size(OSScrolls *scroll, const uint32_t width, const uint32_t height);

void _osscrolls_visible(OSScrolls *scroll, const bool_t horizontal, const bool_t vertical);

__END_C
