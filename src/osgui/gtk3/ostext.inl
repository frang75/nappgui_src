/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.inl
 *
 */

/* Operating System textview */

#include "osgui_gtk.ixx"

__EXTERN_C

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel);

void _ostext_set_capture(OSText *view, OSControl *control);

void _ostext_release_capture(OSText *view);

__END_C

