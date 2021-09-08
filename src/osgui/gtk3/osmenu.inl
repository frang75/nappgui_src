/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.inl
 *
 */

/* Operating System native menu */

#include "osgui_gtk.ixx"

__EXTERN_C

//HMENU _osmenu_hmenu(OSMenu *menu);
//
GtkWidget *_osmenu_widget(OSMenu *menu);

void _osmenu_menubar(OSMenu *menu, OSWindow *window);

void _osmenu_menubar_unlink(OSMenu *menu, OSWindow *window);
//
//void _osmenu_recompute(OSMenu *menu);
//
void _osmenu_attach_to_item(OSMenu *menu, OSMenuItem *item, GtkMenuItem *menuitem);
//
void _osmenu_detach_from_item(OSMenu *menu, OSMenuItem *item, GtkMenuItem *menuitem);

void _osmenu_set_accel(OSMenu *menu, GtkAccelGroup *accel);

void _osmenu_unset_accel(OSMenu *menu, GtkAccelGroup *accel);

//
//void _osmenu_append_item(HMENU menu, const Font *font, const uchar_t *text, const OSImage *image, OSControl *owner);

__END_C
