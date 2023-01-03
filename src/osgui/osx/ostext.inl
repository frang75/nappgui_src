/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.inl
 *
 */

/* Operating System native text view */

#include "osgui_osx.ixx"

BOOL _ostext_is(NSView *view);

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel);

