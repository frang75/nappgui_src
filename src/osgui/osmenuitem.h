/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenuitem.h
 * https://nappgui.com/en/osgui/osmenuitem.html
 *
 */

/* Operating System native menu item */

#include "osgui.hxx"

__EXTERN_C

OSMenuItem *osmenuitem_create(const menu_flag_t flag);

void osmenuitem_destroy(OSMenuItem **item);

void osmenuitem_OnClick(OSMenuItem *item, Listener *listener);

void osmenuitem_enabled(OSMenuItem *item, const bool_t enabled);

void osmenuitem_visible(OSMenuItem *item, const bool_t visible);

void osmenuitem_text(OSMenuItem *item, const char_t *text);

void osmenuitem_image(OSMenuItem *item, const Image *image);

void osmenuitem_key(OSMenuItem *item, const uint32_t key, const uint32_t modifiers);

void osmenuitem_state(OSMenuItem *item, const state_t state);

void osmenuitem_submenu(OSMenuItem *item, OSMenu *menu);

void osmenuitem_unset_submenu(OSMenuItem *item, OSMenu *menu);

__END_C
