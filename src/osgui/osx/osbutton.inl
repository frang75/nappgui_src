/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.inl
 *
 */

/* Operating System native button */

#include "osgui_osx.ixx"

BOOL _osbutton_is(NSView *view);

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel);

BOOL _osbutton_OnIntro(NSResponder *resp);

