/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.inl
 *
 */

/* Operating System textview */

#include "osgui_win.ixx"

__EXTERN_C

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel);

void _ostext_command(OSText *view, WPARAM wParam);

__END_C

