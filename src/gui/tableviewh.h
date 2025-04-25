/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tableviewh.h
 *
 */

/* Undocumented (hidden) API about TableView */

#include "gui.hxx"

__EXTERN_C

_gui_api const Font *tableview_get_font(const TableView *view);

_gui_api real32_t tableview_get_header_height(const TableView *view);

_gui_api uint32_t tableview_get_num_columns(const TableView *view);

_gui_api real32_t tableview_get_column_width(const TableView *view, const uint32_t column_id);

_gui_api const char_t *tableview_get_header_title(const TableView *view, const uint32_t column_id);

_gui_api align_t tableview_get_header_align(const TableView *view, const uint32_t column_id);

__END_C
