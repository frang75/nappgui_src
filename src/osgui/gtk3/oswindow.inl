/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.inl
 *
 */

/* Operating System native window */

#include "osgui_gtk.ixx"

__EXTERN_C

void _oswindow_set_menubar(OSWindow *window, OSMenu *menu);

void _oswindow_unset_menubar(OSWindow *window, OSMenu *menu);

void _oswindow_gtk_app(GtkApplication *app, GdkPixbuf *icon);

void _oswindow_set_app_terminate(void);

void _oswindow_unset_focus(OSWindow *window);

__END_C

