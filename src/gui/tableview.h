/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tableview.h
 * https://nappgui.com/en/gui/tableview.html
 *
 */

/* TableView */

#include "gui.hxx"

__EXTERN_C

TableView *tableview_create(const uint32_t num_cols, const uint32_t flags);

void tableview_OnNotify(TableView *view, Listener *listener);

void tableview_size(TableView *view, S2Df size);

void tableview_ctext(TableView *view, const uint32_t id, const char_t *text);

void tableview_cwidth(TableView *view, const uint32_t id, const real32_t width);

void tableview_calign(TableView *view, const uint32_t id, const align_t align);

real32_t tableview_get_cwidth(TableView *view, const uint32_t id);

void tableview_get_size(const TableView *view, S2Df *size);

void tableview_update(TableView *view);
//void tableview_set_header_font(TableView *view, const Font *font);
//
//void tableview_set_header_text(TableView *view, const uint32_t column_id, const char_t *text);
//
//void tableview_set_header_align(TableView *view, const uint32_t column_id, const align_t align);
//
//void tableview_set_column_width(TableView *view, const uint32_t column_id, const real32_t width);
//
//void tableview_set_column_font(TableView *view, const uint32_t column_id, const Font *font);
//
//void tableview_set_column_align(TableView *view, const uint32_t column_id, const align_t align);
//
//void tableview_set_column_content_size(TableView *view, const uint32_t column_id, const S2Df *size);
//
//void tableview_OnGetNumRows(TableView *view, Listener *listener);

__END_C

