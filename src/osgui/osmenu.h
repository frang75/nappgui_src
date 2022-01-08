/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.h
 *
 */

/* Operating System native menu */

#include "osgui.hxx"

__EXTERN_C

OSMenu *osmenu_create(const enum_t flags);

void osmenu_destroy(OSMenu **menu);

void osmenu_add_item(OSMenu *menu, OSMenuItem *item);

void osmenu_delete_item(OSMenu *menu, OSMenuItem *item);

void osmenu_launch(OSMenu *menu, OSWindow *window, const real32_t x, const real32_t y);

void osmenu_hide(OSMenu *menu);

__END_C
