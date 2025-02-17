/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
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

_gui_api Menu *menu_create(void);

_gui_api void menu_destroy(Menu **menu);

_gui_api void menu_add_item(Menu *menu, MenuItem *item);

_gui_api void menu_ins_item(Menu *menu, const uint32_t pos, MenuItem *item);

_gui_api void menu_del_item(Menu *menu, const uint32_t pos);

_gui_api void menu_launch(Menu *menu, const V2Df position);

_gui_api void menu_off_items(Menu *menu);

_gui_api uint32_t menu_count(const Menu *menu);

_gui_api MenuItem *menu_get_item(Menu *menu, const uint32_t index);

_gui_api const MenuItem *menu_get_citem(const Menu *menu, const uint32_t index);

_gui_api bool_t menu_is_menubar(const Menu *menu);

_gui_api void *menu_imp(const Menu *menu);

__END_C
