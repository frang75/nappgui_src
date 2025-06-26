/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu_gtk.inl
 *
 */

/* Operating System native menu */

#include "osgui_gtk.ixx"

__EXTERN_C

GtkWidget *_osmenu_menubar(OSMenu *menu, OSWindow *window, const uint32_t max_width);

GtkWidget *_osmenu_menubar_unlink(OSMenu *menu, OSWindow *window);

uint32_t _osmenu_menubar_height(const OSMenu *menu);

void _osmenu_widget_recompute(OSMenu *menu);

void _osmenu_attach_to_item(OSMenu *menu, OSMenuItem *item);

void _osmenu_detach_from_item(OSMenu *menu, OSMenuItem *item);

void _osmenu_set_accel(OSMenu *menu, GtkAccelGroup *accel);

void _osmenu_unset_accel(OSMenu *menu, GtkAccelGroup *accel);

__END_C
