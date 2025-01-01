/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.inl
 *
 */

/* Operating System native text view */

#include "osgui.ixx"

__EXTERN_C

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel);

void _ostext_focus(OSText *view, const bool_t focus);

bool_t _ostext_capture_return(const OSText *view);

__END_C
