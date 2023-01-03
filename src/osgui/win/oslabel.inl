/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oslabel.inl
 *
 */

/* Operating System label */

#include "osgui_win.ixx"

__EXTERN_C

void _oslabel_detach_and_destroy(OSLabel **label, OSPanel *panel);

COLORREF _oslabel_color(const OSLabel *label);

HBRUSH _oslabel_background_color(const OSLabel *label, COLORREF *color);

__END_C

