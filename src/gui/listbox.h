/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: listbox.h
 * https://nappgui.com/en/gui/listbox.html
 *
 */

/* Listbox */

#include "gui.hxx"

__EXTERN_C

ListBox *listbox_create(void);

void listbox_OnSelect(ListBox *listbox, Listener *listener);

void listbox_size(ListBox *listbox, S2Df size);

void listbox_checkbox(ListBox *listbox, const bool_t show);

void listbox_multisel(ListBox *listbox, const bool_t multisel);

void listbox_add_elem(ListBox *listbox, const char_t *text, const Image *image);

void listbox_set_elem(ListBox *listbox, const uint32_t index, const char_t *text, const Image *image);

void listbox_clear(ListBox *listbox);

void listbox_color(ListBox *listbox, const uint32_t index, const color_t color);

void listbox_select(ListBox *listbox, const uint32_t index, const bool_t select);

void listbox_check(ListBox *listbox, const uint32_t index, const bool_t check);

uint32_t listbox_count(const ListBox *listbox);

const char_t *listbox_text(const ListBox *listbox, const uint32_t index);

bool_t listbox_selected(const ListBox *listbox, uint32_t index);

bool_t listbox_checked(const ListBox *listbox, uint32_t index);

__END_C

