/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton_win.inl
 *
 */

/* Operating System native button */

#include "osgui_win.ixx"

__EXTERN_C

void _osbutton_command(OSButton *button, WPARAM wParam, const bool_t restore_focus);

void _osbutton_toggle(OSButton *button);

void _osbutton_set_can_focus(OSButton *button, const bool_t can_focus);

__END_C
