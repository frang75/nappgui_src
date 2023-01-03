/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospopup.inl
 *
 */

/* Operating System native popup button */

#include "osgui_osx.ixx"

BOOL _ospopup_is(NSView *view);

void _ospopup_detach_and_destroy(OSPopUp **popup, OSPanel *panel);

