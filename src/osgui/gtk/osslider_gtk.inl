/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osslider_gtk.inl
 *
 */

/* Operating System native slider */

#include "osgui_gtk.ixx"

__EXTERN_C

GtkWidget *_osslider_focus_widget(OSSlider *slider);

bool_t _osslider_has_focus(const OSSlider *slider);

__END_C
