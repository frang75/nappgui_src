/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.inl
 *
 */

/* Operating System native window */

#include "osgui_gtk.ixx"

__EXTERN_C

//OSWindow *_oswindow_create_default(void);
//
//void _oswindow_destroy_default(OSWindow **window);
//
void _oswindow_set_menubar(OSWindow *window, OSMenu *menu);

void _oswindow_unset_menubar(OSWindow *window, OSMenu *menu);

void _oswindow_gtk_app(GtkApplication *app, GdkPixbuf *icon);

void _oswindow_set_app_terminate(void);

void _oswindow_unset_focus(OSWindow *window);

//
//bool_t _oswindow_proccess_message(MSG *msg, HACCEL accelerator_table);
//
//void _oswindow_OnSetFocus(OSControl *control);
//
//void _oswindow_SetNextFocus(OSControl *control, const BOOL previous);
//
//bool_t _oswindow_in_resizing(HWND child_hwnd);

__END_C

