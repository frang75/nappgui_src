/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: edit.h
 * https://nappgui.com/en/gui/edit.html
 *
 */

/* Edit Box */

#include "gui.hxx"

__EXTERN_C

Edit *edit_create(void);

Edit *edit_multiline(void);

void edit_OnFilter(Edit *edit, Listener *listener);

void edit_OnChange(Edit *edit, Listener *listener);

void edit_text(Edit *edit, const char_t *text);

void edit_font(Edit *edit, const Font *font);

void edit_align(Edit *edit, const align_t align);

void edit_passmode(Edit *edit, const bool_t passmode);

void edit_editable(Edit *edit, const bool_t is_editable);

void edit_autoselect(Edit *edit, const bool_t autoselect);

void edit_tooltip(Edit *edit, const char_t *text);

void edit_color(Edit *edit, const color_t color);

void edit_color_focus(Edit *edit, const color_t color);

void edit_bgcolor(Edit *edit, const color_t color);

void edit_bgcolor_focus(Edit *edit, const color_t color);

void edit_phtext(Edit *edit, const char_t *text);

void edit_phcolor(Edit *edit, const color_t color);

void edit_phstyle(Edit *edit, const uint32_t fstyle);

const char_t *edit_get_text(const Edit *edit);

__END_C
