/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osslider.inl
 *
 */

/* Operating System native slider */

#include "osgui_osx.ixx"

BOOL _osslider_is(NSView *view);

void _osslider_detach_and_destroy(OSSlider **slider, OSPanel *panel);

