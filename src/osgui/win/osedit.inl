/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.inl
 *
 */

/* Operating System edit box */

#include "osgui_win.ixx"

__EXTERN_C

void _osedit_detach_and_destroy(OSEdit **edit, OSPanel *panel);

void _osedit_command(OSEdit *edit, WPARAM wParam);

COLORREF _osedit_color(const OSEdit *edit);

HBRUSH _osedit_background_color(const OSEdit *edit, COLORREF *color);

void _osedit_kill_focus(const OSEdit *edit);

__END_C

