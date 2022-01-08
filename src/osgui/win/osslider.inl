/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osslider.inl
 *
 */

/* Operating System native slider */

#include "osgui_win.ixx"

__EXTERN_C

void _osslider_detach_and_destroy(OSSlider **slider, OSPanel *panel);

void _osslider_message(OSSlider *slider, WPARAM wParam);

__END_C

