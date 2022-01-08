/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.inl
 *
 */

/* Operating System native button */

#include "osgui_win.ixx"

__EXTERN_C

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel);

void _osbutton_command(OSButton *button, WPARAM wParam);

bool_t _osbutton_is_pushbutton(const OSButton *button);

void _osbutton_set_default(OSButton *button);

void _osbutton_unset_default(OSButton *button);

__END_C

