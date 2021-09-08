/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospanel.inl
 *
 */

/* Operating System native view */

#include "osgui_gtk.ixx"

__EXTERN_C

void _ospanel_destroy(OSPanel **panel);

void _ospanel_attach_control(OSPanel *panel, OSControl *control);

void _ospanel_detach_control(OSPanel *panel, OSControl *control);

__END_C

