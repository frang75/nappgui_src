/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.inl
 *
 */

/* Operating System native custom view */

#include "osgui_gtk.ixx"

__EXTERN_C

void _osview_detach_and_destroy(OSView **view, OSPanel *panel);

void _osview_set_capture(OSView *view, OSControl *control);

void _osview_release_capture(OSView *view);

void _osview_set_focus(OSView *view);

void _osview_unset_focus(OSView *view);

GtkWidget *_osview_focus(OSView *view);

__END_C

