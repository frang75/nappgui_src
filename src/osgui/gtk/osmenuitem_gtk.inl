/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenuitem_gtk.inl
 *
 */

/* Operating System native menu item */

#include "osgui_gtk.ixx"

__EXTERN_C

GtkWidget *_osmenuitem_widget(OSMenuItem *item);

void _osmenuitem_append_to_menu(OSMenuItem *item, OSMenu *menu, GtkWidget *widget);

void _osmenuitem_append_to_menubar(OSMenuItem *item, OSMenu *menu, GtkWidget *widget, const uint32_t max_width);

void _osmenuitem_unset_parent(OSMenuItem *item, OSMenu *menu);

void _osmenuitem_set_accel(OSMenuItem *item, GtkAccelGroup *accel);

void _osmenuitem_unset_accel(OSMenuItem *item, GtkAccelGroup *accel);

__END_C
