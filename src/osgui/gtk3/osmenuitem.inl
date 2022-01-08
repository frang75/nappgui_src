/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenuitem.inl
 *
 */

/* Operating System native menu item */

#include "osgui_gtk.ixx"

__EXTERN_C

//OSMenuItem *_osmenuitem_create(const Font *font);
//

GtkWidget *_osmenuitem_widget(OSMenuItem *item);

GtkWidget *_osmenuitem_bar_widget(OSMenuItem *item);

void _osmenuitem_set_parent(OSMenuItem *item, OSMenu *menu, GtkMenuShell *menushell);

void _osmenuitem_unset_parent(OSMenuItem *item, OSMenu *menu, GtkMenuShell *menushell);

void _osmenuitem_set_accel(OSMenuItem *item, GtkAccelGroup *accel);

void _osmenuitem_unset_accel(OSMenuItem *item, GtkAccelGroup *accel);

//
//void _osmenuitem_insert_in_hmenu(OSMenuItem *item, HMENU hmenu);
//
//void _osmenuitem_click(OSMenuItem *item, UINT id, UINT type, UINT state);

__END_C
