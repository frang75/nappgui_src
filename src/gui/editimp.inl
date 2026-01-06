/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: editimp.inl
 *
 */

/* Shared implementation for Edit and Combo */

#include "gui.ixx"

__EXTERN_C

EditImp *_editimp_from_edit(const GuiCtx *context, void *ositem);

EditImp *_editimp_from_combo(const GuiCtx *context, void *ositem);

void _editimp_destroy(EditImp **edit);

void _editimp_OnFilter(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type);

void _editimp_OnChange(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type);

void _editimp_OnFocus(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type);

void _editimp_set_OnFilter(EditImp *edit, Listener *listener);

void _editimp_set_OnChange(EditImp *edit, Listener *listener);

void _editimp_set_OnFocus(EditImp *edit, Listener *listener);

void _editimp_text(EditImp *edit, const char_t *text);

void _editimp_font(EditImp *edit, const Font *font);

void _editimp_passmode(EditImp *edit, const bool_t passmode);

void _editimp_color(EditImp *edit, const color_t color);

void _editimp_color_focus(EditImp *edit, const color_t color);

void _editimp_bgcolor(EditImp *edit, const color_t color);

void _editimp_bgcolor_focus(EditImp *edit, const color_t color);

void _editimp_phtext(EditImp *edit, const char_t *text);

void _editimp_phcolor(EditImp *edit, const color_t color);

void _editimp_phstyle(EditImp *edit, const uint32_t fstyle);

const char_t *_editimp_get_text(const EditImp *edit);

void _editimp_locale(EditImp *edit);

__END_C
