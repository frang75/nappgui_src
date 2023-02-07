/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.inl
 *
 */

/* Operating System native view */

#include "osgui_osx.ixx"

__EXTERN_C

BOOL _oswindow_in_destroy(NSWindow *window);

NSView *_oswindow_main_view(OSWindow *window);

void _oswindow_focus_edit(NSWindow *window, NSView *edit);

NSView *_oswindow_get_focus_edit(NSWindow *window);

__END_C

