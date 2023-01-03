/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osupdown.inl
 *
 */

/* Operating System native updown */

#include "osgui_osx.ixx"

BOOL _osupdown_is(NSView *view);

void _osupdown_detach_and_destroy(OSUpDown **updown, OSPanel *panel);

