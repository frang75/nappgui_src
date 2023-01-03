/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.inl
 *
 */

/* Operating System native button */

#include "osgui_gtk.ixx"

__EXTERN_C

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel);

void _osbutton_command(OSButton *button);

GtkWidget *_osbutton_focus(OSButton *button);

__END_C

