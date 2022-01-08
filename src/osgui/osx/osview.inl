/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.inl
 *
 */

/* Operating System native custom view */

#include "osgui_osx.ixx"

BOOL _osview_is(NSView *view);

void _osview_OnFocus(NSView *view, const bool_t focus);

void _osview_detach_and_destroy(OSView **view, OSPanel *panel);

