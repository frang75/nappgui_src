/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: menuitem.h
 * https://nappgui.com/en/gui/menuitem.html
 *
 */

/* Menu Item */

#include "gui.hxx"

__EXTERN_C

MenuItem *menuitem_create(void);

MenuItem *menuitem_separator(void);

void menuitem_OnClick(MenuItem *item, Listener *listener);

void menuitem_enabled(MenuItem *item, const bool_t enabled);

void menuitem_visible(MenuItem *item, const bool_t visible);

void menuitem_text(MenuItem *item, const char_t *text);

void menuitem_image(MenuItem *item, const Image *image);

void menuitem_key(MenuItem *item, const vkey_t key, const uint32_t modifiers);

void menuitem_submenu(MenuItem *item, Menu **submenu);

void menuitem_state(MenuItem *item, const state_t state);

__END_C

