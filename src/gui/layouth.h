/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: layouth.h
 *
 */

/* Undocumented (hidden) API about Layouts */

#include "gui.hxx"

__EXTERN_C

_gui_api void cell_force_size(Cell *cell, const real32_t width, const real32_t height);

_gui_api real32_t cell_get_hsize(const Cell *cell);

_gui_api real32_t cell_get_vsize(const Cell *cell);

_gui_api align_t cell_get_halign(const Cell *cell);

_gui_api align_t cell_get_valign(const Cell *cell);

_gui_api real32_t layout_get_hsize(const Layout *layout, const uint32_t col);

_gui_api real32_t layout_get_vsize(const Layout *layout, const uint32_t row);

_gui_api real32_t layout_get_hmargin(const Layout *layout, const uint32_t col);

_gui_api real32_t layout_get_vmargin(const Layout *layout, const uint32_t row);

_gui_api real32_t layout_get_margin_left(const Layout *layout);

_gui_api real32_t layout_get_margin_right(const Layout *layout);

_gui_api real32_t layout_get_margin_top(const Layout *layout);

_gui_api real32_t layout_get_margin_bottom(const Layout *layout);

_gui_api void layout_remove_cell(Layout *layout, const uint32_t col, const uint32_t row);

_gui_api void layout_name(Layout *layout, const char_t *name);

__END_C
