/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.inl
 *
 */

/* Operating System edit box */

#include "osgui_osx.ixx"

BOOL _osedit_is(NSView *view);

void _osedit_detach_and_destroy(OSEdit **edit, OSPanel *panel);

