/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oslabel.inl
 *
 */

/* Operating System label */

#include "osgui_osx.ixx"

BOOL _oslabel_is(NSView *view);

void _oslabel_detach_and_destroy(OSLabel **label, OSPanel *panel);

