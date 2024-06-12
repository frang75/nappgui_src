/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb_gtk.inl
 *
 */

/* Operating System webview */

#include "osgui_gtk.ixx"

__EXTERN_C

GtkWidget *_osweb_focus_widget(OSWeb *view);

void _osweb_set_capture(OSWeb *view, OSControl *control);

void _osweb_release_capture(OSWeb *view);

__END_C
