/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osprogress.inl
 *
 */

/* Operating System native progress indicator */

#include "osgui_osx.ixx"

BOOL _osprogress_is(NSView *view);

void _osprogress_detach_and_destroy(OSProgress **progress, OSPanel *panel);

