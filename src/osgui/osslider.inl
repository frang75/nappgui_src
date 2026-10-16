/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osslider.inl
 *
 */

/* Operating System native slider */

#include "osgui.ixx"

__EXTERN_C

void _osslider_detach_and_destroy(OSSlider **slider, OSPanel *panel);

void _osslider_focus(OSSlider *slider, const bool_t focus);

__END_C
