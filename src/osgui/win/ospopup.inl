/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospopup.inl
 *
 */

/* Operating System native popup button */

#include "osgui_win.ixx"

__EXTERN_C

void _ospopup_detach_and_destroy(OSPopUp **popup, OSPanel *panel);

void _ospopup_command(OSPopUp *popup, WPARAM wParam);

HWND _ospopup_focus(OSPopUp *popup);

__END_C

