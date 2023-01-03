/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.inl
 *
 */

/* Operating System split view */

#include "osgui_gtk.ixx"

__EXTERN_C

void _ossplit_detach_and_destroy(OSSplit **view, OSPanel *panel);

void _ossplit_OnPress(OSSplit *view, GdkEventButton *event);

__END_C

