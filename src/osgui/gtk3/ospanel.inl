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

#include "osgui_gtk.ixx"

__EXTERN_C

void _ospanel_destroy(OSPanel **panel);

void _ospanel_attach_control(OSPanel *panel, OSControl *control);

void _ospanel_detach_control(OSPanel *panel, OSControl *control);

void _ospanel_set_capture(OSPanel *panel, OSControl *control);

void _ospanel_release_capture(OSPanel *panel);

void _ospanel_scroll_frame(const OSPanel *panel, RectI *rect);

void _ospanel_scroll(OSPanel *panel, const int x, const int y);

__END_C
