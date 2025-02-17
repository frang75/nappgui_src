/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
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

_gui_api MenuItem *menuitem_create(void);

_gui_api MenuItem *menuitem_separator(void);

_gui_api void menuitem_OnClick(MenuItem *item, Listener *listener);

_gui_api void menuitem_enabled(MenuItem *item, const bool_t enabled);

_gui_api void menuitem_visible(MenuItem *item, const bool_t visible);

_gui_api void menuitem_state(MenuItem *item, const gui_state_t state);

_gui_api void menuitem_text(MenuItem *item, const char_t *text);

_gui_api void menuitem_image(MenuItem *item, const Image *image);

_gui_api void menuitem_key(MenuItem *item, const vkey_t key, const uint32_t modifiers);

_gui_api void menuitem_submenu(MenuItem *item, Menu **submenu);

_gui_api const char_t *menuitem_get_text(const MenuItem *item);

_gui_api const Image *menuitem_get_image(const MenuItem *item);

_gui_api bool_t menuitem_get_separator(const MenuItem *item);

_gui_api bool_t menuitem_get_enabled(const MenuItem *item);

_gui_api bool_t menuitem_get_visible(const MenuItem *item);

_gui_api gui_state_t menuitem_get_state(const MenuItem *item);

__END_C
