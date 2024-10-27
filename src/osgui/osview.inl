/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.inl
 *
 */

/* Operating System native custom view */

#include "osgui.ixx"

__EXTERN_C

void _osview_detach_and_destroy(OSView **view, OSPanel *panel);

bool_t _osview_resign_focus(const OSView *view);

bool_t _osview_accept_focus(const OSView *view);

void _osview_focus(OSView *view, const bool_t focus);

bool_t _osview_capture_tab(const OSView *view);

__END_C
