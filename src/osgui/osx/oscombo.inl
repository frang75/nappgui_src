/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.inl
 *
 */

/* Operating System native combo box */

#include "osgui_osx.ixx"

BOOL _oscombo_is(NSView *view);

void _oscombo_detach_and_destroy(OSCombo **combo, OSPanel *panel);
