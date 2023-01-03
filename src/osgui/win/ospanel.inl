/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospanel.inl
 *
 */

/* Operating System native panel */

#include "osgui_win.ixx"

__EXTERN_C

OSPanel *_ospanel_create_default(void);

void _ospanel_destroy_default(OSPanel **panel);

void _ospanel_destroy(OSPanel **panel);

//HWND _ospanel_hit_point(const OSPanel *panel, const POINT *point);

void _ospanel_resize_double_buffer(OSPanel *panel, LONG width, LONG height);

HDC _ospanel_paint_double_buffer(OSPanel *panel, const uint32_t resize_strategy, LONG *width, LONG *height);

void _ospanel_attach_control(OSPanel *panel, OSControl *control);

void _ospanel_detach_control(OSPanel *panel, OSControl *control);

COLORREF _ospanel_background_color(OSPanel *panel, HWND child_hwnd);

bool_t _ospanel_with_scroll(const OSPanel *panel);

void _ospanel_scroll_pos(OSPanel *panel, int *scroll_x, int *scroll_y);

void _ospanel_scroll_frame(const OSPanel *panel, RECT *rect);

void _ospanel_scroll(OSPanel *panel, const int x, const int y);

__END_C

