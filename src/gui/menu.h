/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: menu.h
 * https://nappgui.com/en/gui/menu.html
 *
 */

/* Menu */

#include "gui.hxx"

__EXTERN_C

Menu *menu_create(void);

void menu_destroy(Menu **menu);

void menu_item(Menu *menu, MenuItem *item);

void menu_launch(Menu *menu, const V2Df position);

void menu_hide(Menu *menu);

void menu_off_items(Menu *menu);

MenuItem *menu_get_item(Menu *menu, const uint32_t index);

uint32_t menu_size(const Menu *menu);

__END_C


