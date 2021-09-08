/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: combo.h
 * https://nappgui.com/en/gui/combo.html
 *
 */

/* Combo box */

#include "gui.hxx"

__EXTERN_C

Combo *combo_create(void);

void combo_OnFilter(Combo *combo, Listener *listener);

void combo_OnChange(Combo *combo, Listener *listener);

void combo_text(Combo *combo, const char_t *text);

void combo_align(Combo *combo, const align_t align);

void combo_tooltip(Combo *combo, const char_t *text);

void combo_color(Combo *combo, const color_t color);

void combo_color_focus(Combo *combo, const color_t color);

void combo_bgcolor(Combo *combo, const color_t color);

void combo_bgcolor_focus(Combo *combo, const color_t color);

void combo_phtext(Combo *combo, const char_t *text);

void combo_phcolor(Combo *combo, const color_t color);

void combo_phstyle(Combo *combo, const uint32_t fstyle);

const char_t *combo_get_text(const Combo *combo, const uint32_t index);

uint32_t combo_count(const Combo *combo);

void combo_add_elem(Combo *combo, const char_t *text, const Image *image);

void combo_set_elem(Combo *combo, const uint32_t index, const char_t *text, const Image *image);

void combo_ins_elem(Combo *combo, const uint32_t index, const char_t *text, const Image *image);

void combo_del_elem(Combo *combo, const uint32_t index);

void combo_duplicates(Combo *combo, const bool_t duplicates);

__END_C

