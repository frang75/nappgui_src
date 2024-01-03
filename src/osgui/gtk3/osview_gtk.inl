/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview_gtk.inl
 *
 */

/* Operating System native custom view */

#include "osgui_gtk.ixx"

__EXTERN_C

void _osview_set_capture(OSView *view, OSControl *control);

void _osview_release_capture(OSView *view);

void _osview_set_focus(OSView *view);

void _osview_unset_focus(OSView *view);

GtkWidget *_osview_focus(OSView *view);

GtkWidget *_osview_area(OSView *view);

void _osview_scroll_event(OSView *view, const gui_orient_t orient, const gui_scroll_t event);

__END_C
