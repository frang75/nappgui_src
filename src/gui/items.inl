/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: items.inl
 *
 */

/* Shared implementation for PopUp and Combo lists */

#include "gui.ixx"

__EXTERN_C

Items *_items_from_popup(const GuiCtx *context, void *ositem);

Items *_items_from_combo(const GuiCtx *context, void *ositem);

void _items_destroy(Items **items);

const char_t *_items_OnSelect(Items *items, Event *e, Cell *cell, void *sender, const char_t *sender_type);

void _items_set_OnSelect(Items *items, Listener *listener);

void _items_add_elem(Items *items, const char_t *text, const Image *image);

void _items_set_elem(Items *items, const uint32_t index, const char_t *text, const Image *image);

void _items_ins_elem(Items *items, const uint32_t index, const char_t *text, const Image *image);

void _items_del_elem(Items *items, const uint32_t index);

void _items_clear(Items *items);

uint32_t _items_count(const Items *items);

void _items_list_height(Items *items, const uint32_t num_elems);

void _items_selected(Items *items, const uint32_t index);

uint32_t _items_get_selected(const Items *items);

const char_t *_items_get_text(const Items *items, const uint32_t index);

void _items_locale(Items *items);

void _items_uint32(Items *items, const uint32_t value);

__END_C
