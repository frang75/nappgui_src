/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit_gtk.inl
 *
 */

/* Operating System split view */

#include "osgui_gtk.ixx"

__EXTERN_C

void _ossplit_create_tracks(void);

void _ossplit_destroy_tracks(void);

void _ossplit_OnPress(OSSplit *view, GdkEventButton *event);

void _ossplit_OnRelease(OSSplit *view, GdkEventButton *event);

void _ossplit_OnMove(GtkWidget *sender, GdkEventMotion *event);

__END_C
