/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: layout.c
 *
 */

/* Layouts */

#include "layout.h"
#include "layout.inl"
#include "cell.h"
#include "cell.inl"
#include "button.inl"
#include "component.inl"
#include "gbind.inl"
#include "gui.inl"
#include "label.inl"
#include "edit.h"
#include "edit.inl"
#include "panel.inl"
#include "popup.inl"
#include "listbox.inl"
#include "slider.inl"
#include "view.inl"
#include "window.inl"

#include "arrpt.h"
#include "arrst.h"
#include "bmath.h"
#include "bmem.h"
#include "cassert.h"
#include "dbindh.h"
#include "event.h"
#include "ptr.h"
#include "objh.h"
#include "s2d.h"
#include "strings.h"
#include "v2d.h"

typedef struct i_line_dim_t i_LineDim;
typedef struct i_cell_dim_t i_CellDim;
typedef union i_cell_content_t i_CellContent;

struct i_line_dim_t
{
    real32_t forced_size;
    real32_t margin;
    real32_t size;
    real32_t resize_percent;
};

struct i_cell_dim_t
{
    real32_t forced_size;
    real32_t padding_before;
    real32_t padding_after;
    real32_t size;
    align_t align;
};

typedef enum _ctype_t
{
    i_ekEMPTY,
    i_ekCOMPONENT,
    i_ekLAYOUT
} ctype_t;

union i_cell_content_t
{
    void *empty;
    GuiComponent *component;
    Layout *layout;
};

struct _cell_t
{
    ctype_t type;
    bool_t visible;
    bool_t enabled;
    bool_t displayed;
    bool_t tabstop;
    i_CellDim dim[2];
    i_CellContent content;
    Layout *parent;
    const DBind *dbind;
};

struct _layout_t
{
    Object object;
    Cell *parent;
    Panel *panel;
    bool_t is_row_major_tab;
    ArrSt(Cell) *cells;
    ArrSt(i_LineDim) *lines_dim[2];
    ArrPt(Cell) *cells_dim[2];
    uint32_t dim_num_elems[2];
    real32_t dim_margin[2];
    color_t bgcolor;
    color_t skcolor;
    const StBind *stbind;
    void *objbind;
    Listener *OnObjChange;
};

DeclSt(i_LineDim);

/*---------------------------------------------------------------------------*/

static __INLINE void i_init_celldim(
                            i_CellDim *dim,
                            const real32_t forced_size,
                            const real32_t padding_before,
                            const real32_t padding_after,
                            const real32_t size,
                            const align_t align)
{
    cassert_no_null(dim);
    dim->forced_size = forced_size;
    dim->padding_before = padding_before;
    dim->padding_after = padding_after;
    dim->size = size;
    dim->align = align;
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_init_cell(
                        Cell *cell,
                        const ctype_t type,
                        const bool_t visible,
                        const bool_t enabled,
                        const bool_t displayed,
                        const bool_t tabstop,
                        const i_CellDim *dim0,
                        const i_CellDim *dim1,
                        const i_CellContent *content,
                        Layout *layout)
{
    cassert_no_null(cell);
    cell->type = type;
    cell->visible = visible;
    cell->enabled = enabled;
    cell->displayed = displayed;
    cell->tabstop = tabstop;
    cell->dim[0] = ptr_get(dim0, i_CellDim);
    cell->dim[1] = ptr_get(dim1, i_CellDim);
    cell->content = ptr_get(content, i_CellContent);
    cell->parent = layout;
    cell->dbind = NULL;
}

/*---------------------------------------------------------------------------*/

static void i_remove_cell(Cell *cell)
{
    cassert_no_null(cell);
    switch(cell->type)
    {
        case i_ekEMPTY:
            break;

        case i_ekCOMPONENT:
            cell->type = i_ekEMPTY;
            obj_release(&cell->content.component, GuiComponent);
            break;

        case i_ekLAYOUT:
            cell->type = i_ekEMPTY;
            _layout_destroy(&cell->content.layout);
            break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void _layout_destroy(Layout **layout)
{
    cassert_no_null(layout);
    cassert_no_null(*layout);
    if ((*layout)->object.count == 0)
    {
        arrst_destroy(&(*layout)->lines_dim[0], NULL, i_LineDim);
        arrst_destroy(&(*layout)->lines_dim[1], NULL, i_LineDim);
        arrpt_destroy(&(*layout)->cells_dim[0], NULL, Cell);
        arrpt_destroy(&(*layout)->cells_dim[1], NULL, Cell);
        arrst_destroy(&(*layout)->cells, i_remove_cell, Cell);
        listener_destroy(&(*layout)->OnObjChange);
        obj_delete(layout, Layout);
    }
    else
    {
        obj_release(layout, Layout);
    }
}

/*---------------------------------------------------------------------------*/

static ArrSt(i_LineDim) *i_create_linedim(const uint32_t num_elems)
{
    ArrSt(i_LineDim) *dimension = arrst_create(i_LineDim);
    real32_t resize = 1.f / (real32_t)num_elems;
    real32_t total = 0.f;

    arrst_grow(dimension, num_elems, i_LineDim);
    arrst_foreach(dim, dimension, i_LineDim)
        if (dim_i == dim_total - 1)
            resize = 1.f - total;
        bmem_zero(dim, i_LineDim);
        dim->resize_percent = resize;
        total += resize;
    arrst_end()
    cassert(bmath_absf(total - 1.f) < 0.000001f);

    return dimension;
}

/*---------------------------------------------------------------------------*/

Layout *layout_create(const uint32_t ncols, const uint32_t nrows)
{
    register uint32_t num_cells = nrows * ncols;
    ArrSt(Cell) *cells = arrst_create(Cell);
    ArrSt(i_LineDim) *lines_dim0 = i_create_linedim(ncols);
    ArrSt(i_LineDim) *lines_dim1 = i_create_linedim(nrows);
    ArrPt(Cell) *cells_dim0 = arrpt_create(Cell);
    ArrPt(Cell) *cells_dim1 = arrpt_create(Cell);
    Layout *layout = obj_new0(Layout);

    cassert(num_cells > 0);
    arrst_grow(cells, num_cells, Cell);
    arrpt_grow(cells_dim0, num_cells, Cell);
    arrpt_grow(cells_dim1, num_cells, Cell);

    arrst_foreach(cell, cells, Cell)
        i_CellDim cdim0, cdim1;
        i_CellContent content;
        content.empty = NULL;
        i_init_celldim(&cdim0, 0.f, 0.f, 0.f, 0.f, ENUM_MAX(align_t));
        i_init_celldim(&cdim1, 0.f, 0.f, 0.f, 0.f, ENUM_MAX(align_t));
        i_init_cell(cell, i_ekEMPTY, PARAM(visible, TRUE), PARAM(enabled, TRUE), PARAM(displayed, TRUE), PARAM(tabstop, TRUE), &cdim0, &cdim1, &content, layout);
    arrst_end()

    /* Row major order for column-dimensions (dim[0]) */
    {
        register uint32_t i, j;
        register Cell *cell = arrst_all(cells, Cell);
        register Cell **dim_cell = arrpt_all(cells_dim0, Cell);
        for (i = 0; i < ncols; ++i)
        {
            for (j = 0; j < nrows; ++j, ++dim_cell)
                *dim_cell = cell + (j * ncols) + i;
        }
    }

    /* Column major order for row-dimensions (dim[1]) */
    {
        register uint32_t i;
        register Cell *cell = arrst_all(cells, Cell);
        register Cell **dim_cell = arrpt_all(cells_dim1, Cell);
        for (i = 0; i < num_cells; ++i, ++dim_cell, ++cell)
            *dim_cell = cell;
    }

    layout->is_row_major_tab = TRUE;
    layout->cells = cells;
    layout->lines_dim[0] = lines_dim0;
    layout->lines_dim[1] = lines_dim1;
    layout->cells_dim[0] = cells_dim0;
    layout->cells_dim[1] = cells_dim1;
    layout->dim_num_elems[0] = nrows;
    layout->dim_num_elems[1] = ncols;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Cell *i_get_cell(Layout *layout, const uint32_t col, const uint32_t row)
{
    register uint32_t position = UINT32_MAX;
    cassert_no_null(layout);
    cassert(col < arrst_size(layout->lines_dim[0], i_LineDim));
    cassert(row < arrst_size(layout->lines_dim[1], i_LineDim));
    position = row * arrst_size(layout->lines_dim[0], i_LineDim) + col;
    return arrst_get(layout->cells, position, Cell);
}

/*---------------------------------------------------------------------------*/

Cell *layout_cell(Layout *layout, const uint32_t col, const uint32_t row)
{
    return i_get_cell(layout, col, row);
}

/*---------------------------------------------------------------------------*/

void *layout_get_control_imp(Layout *layout, const uint32_t col, const uint32_t row, const char_t *type)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    cassert_unref(str_equ_c(_component_type(cell->content.component), type) == TRUE, type);
    return (void*)cell->content.component;
}

/*---------------------------------------------------------------------------*/

Layout* layout_get_layout(Layout* layout, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(cell->type == i_ekLAYOUT);
    return cell->content.layout;
}

/*---------------------------------------------------------------------------*/

static Cell *i_set_component(Layout *layout, GuiComponent *component, const uint32_t col, const uint32_t row, const align_t halign, const align_t valign)
{
    i_CellContent content;
    Cell *cell = NULL;
    cassert_no_null(layout);
    content.component = obj_retain(component, GuiComponent);
    cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(cell->type == i_ekEMPTY);
    /* cassert(component->parent == NULL); */
    component->parent = cell;
    cell->type = i_ekCOMPONENT;
    if (cell->dim[0].align == ENUM_MAX(align_t))
        cell->dim[0].align = halign;

    if (cell->dim[1].align == ENUM_MAX(align_t))
        cell->dim[1].align = valign;

    cell->content = content;
    return cell;
}

/*---------------------------------------------------------------------------*/

static void i_change_component(Layout *layout, GuiComponent *component, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    bool_t visible = FALSE, enabled = FALSE, displayed = FALSE, tabstop = FALSE;
    i_CellDim dim0, dim1;
    i_CellContent content;
    GuiComponent *current_component;
    Window *parent_window;
    cassert_no_null(layout);
    cassert_no_null(layout->panel);
    content.component = obj_retain(component, GuiComponent);
    cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    cassert(cell->content.component->type == component->type);
    cassert(cell->dbind == NULL);
    visible = cell->visible;
    enabled = cell->enabled;
    displayed = cell->displayed;
    tabstop = cell->tabstop;
    dim0 = cell->dim[0];
    dim1 = cell->dim[1];
    current_component = cell->content.component;
    i_remove_cell(cell);
    cassert(cell->type == i_ekEMPTY);
    i_init_cell(cell, i_ekCOMPONENT, visible, enabled, displayed, tabstop, &dim0, &dim1, &content, layout);
    _panel_attach_component(layout->panel, cell->content.component);
    _panel_invalidate_layout(layout->panel, layout);
    parent_window = _panel_get_window(layout->panel);
    _component_set_parent_window(component, parent_window);

    if (layout->panel != NULL)
        _panel_destroy_component(layout->panel, current_component);
}

/*---------------------------------------------------------------------------*/

void layout_label(Layout *layout, Label *label, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    register align_t align = ekLEFT;
    /* if (_label_is_multiline(label) == TRUE)
       align = ekJUSTIFY; */
    cell = i_set_component(layout, (GuiComponent*)label, col, row, align, ekCENTER);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_button(Layout *layout, Button *button, const uint32_t col, const uint32_t row)
{
    register align_t align = ekJUSTIFY;
    uint32_t flags = _button_flags(button);
    if (button_get_type(flags) != ekBUTTON_PUSH/* && button_type(flags) != ekBUTTON_HEADER*/)
        align = ekLEFT;
    i_set_component(layout, (GuiComponent*)button, col, row, align, ekCENTER);
}

/*---------------------------------------------------------------------------*/

void layout_popup(Layout *layout, PopUp *popup, const uint32_t col, const uint32_t row)
{
    cassert_no_null(popup);
    i_set_component(layout, (GuiComponent*)popup, col, row, ekJUSTIFY, ekCENTER);
}

/*---------------------------------------------------------------------------*/

void layout_edit(Layout *layout, Edit *edit, const uint32_t col, const uint32_t row)
{
    align_t valign = ekCENTER;
    cassert_no_null(edit);
    if (_edit_is_multiline(edit) == TRUE)
        valign = ekJUSTIFY;
    i_set_component(layout, (GuiComponent*)edit, col, row, ekJUSTIFY, valign);
}

/*---------------------------------------------------------------------------*/

void layout_combo(Layout *layout, Combo *combo, const uint32_t col, const uint32_t row)
{
    cassert_no_null(combo);
    i_set_component(layout, (GuiComponent*)combo, col, row, ekJUSTIFY, ekCENTER);
}

/*---------------------------------------------------------------------------*/

void layout_listbox(Layout *layout, ListBox *list, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(list);
    cell = i_set_component(layout, (GuiComponent*)list, col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_unref(cell->tabstop == TRUE, cell);
}

/*---------------------------------------------------------------------------*/

void layout_updown(Layout *layout, UpDown *updown, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(updown);
    cell = i_set_component(layout, (GuiComponent*)updown, col, row, ekJUSTIFY, ekJUSTIFY);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_slider(Layout *layout, Slider *slider, const uint32_t col, const uint32_t row)
{
    register align_t halig = ekJUSTIFY;
    register align_t valign = ekCENTER;
    if (_slider_is_horizontal(slider) == FALSE)
    {
        halig = ekCENTER;
        valign = ekJUSTIFY;
    }

    i_set_component(layout, (GuiComponent*)slider, col, row, halig, valign);
}

/*---------------------------------------------------------------------------*/

void layout_progress(Layout *layout, Progress *progress, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(progress);
    cell = i_set_component(layout, (GuiComponent*)progress, col, row, ekJUSTIFY, ekCENTER);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_view(Layout *layout, View *view, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(view);
    cell = i_set_component(layout, (GuiComponent*)view, col, row, ekJUSTIFY, ekJUSTIFY);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_textview(Layout *layout, TextView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(view);
    cell = i_set_component(layout, (GuiComponent*)view, col, row, ekJUSTIFY, ekJUSTIFY);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_imageview(Layout *layout, ImageView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell;
    cassert_no_null(view);
    cell = i_set_component(layout, (GuiComponent*)view, col, row, ekJUSTIFY, ekJUSTIFY);
    cassert(cell->tabstop == TRUE);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_tableview(Layout *layout, TableView *view, const uint32_t col, const uint32_t row)
{
    cassert_no_null(view);
    i_set_component(layout, (GuiComponent*)view, col, row, ekJUSTIFY, ekJUSTIFY);
}

/*---------------------------------------------------------------------------*/

void layout_splitview(Layout *layout, SplitView *view, const uint32_t col, const uint32_t row)
{
    cassert_no_null(view);
    i_set_component(layout, (GuiComponent*)view, col, row, ekJUSTIFY, ekJUSTIFY);
}

/*---------------------------------------------------------------------------*/

void layout_panel(Layout *layout, Panel *panel, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    if (cell->type == i_ekEMPTY)
        i_set_component(layout, (GuiComponent*)panel, col, row, ekJUSTIFY, ekJUSTIFY);
    else
        i_change_component(layout, (GuiComponent*)panel, col, row);
}

/*---------------------------------------------------------------------------*/

void layout_layout(Layout *layout, Layout *sublayout, const uint32_t col, const uint32_t row)
{
    i_CellContent content;
    Cell *cell = NULL;
    cassert_no_null(layout);
    cassert_no_null(sublayout);
    cassert(layout != sublayout);
    content.layout = sublayout;
    cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(sublayout->parent == NULL);
    sublayout->parent = cell;

    cassert(cell->type == i_ekEMPTY);
    cell->type = i_ekLAYOUT;

    if (cell->dim[0].align == ENUM_MAX(align_t))
        cell->dim[0].align = ekJUSTIFY;

    if (cell->dim[1].align == ENUM_MAX(align_t))
        cell->dim[1].align = ekJUSTIFY;

    cell->content = content;
}

/*---------------------------------------------------------------------------*/

void layout_taborder(Layout *layout, const gui_orient_t order)
{
    cassert_no_null(layout);
    layout->is_row_major_tab = (order == ekGUI_HORIZONTAL) ? FALSE : TRUE;
}

/*---------------------------------------------------------------------------*/

void layout_tabstop(Layout *layout, const uint32_t col, const uint32_t row, const bool_t tabstop)
{
    Cell *cell = NULL;
    cassert_no_null(layout);
    cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cell->tabstop = tabstop;
}

/*---------------------------------------------------------------------------*/

void layout_hsize(Layout *layout, const uint32_t col, const real32_t width)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert_msg(width >= 0.f, "Column 'width' must be positive.");
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    cassert_no_null(dim);
    dim->forced_size = width;
}

/*---------------------------------------------------------------------------*/

void layout_vsize(Layout *layout, const uint32_t row, const real32_t height)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert_msg(height >= 0.f, "Row 'height' must be positive.");
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    dim->forced_size = height;
}

/*---------------------------------------------------------------------------*/

void layout_hmargin(Layout *layout, const uint32_t col, const real32_t margin)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
	cassert(col < arrst_size(layout->lines_dim[0], i_LineDim) - 1);
    dim = arrst_get(layout->lines_dim[0], col + 1, i_LineDim);
    dim->margin = margin;
}

/*---------------------------------------------------------------------------*/

void layout_vmargin(Layout *layout, const uint32_t row, const real32_t margin)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
	cassert_msg(row < arrst_size(layout->lines_dim[1], i_LineDim) - 1, "'row' out of range");
    dim = arrst_get(layout->lines_dim[1], row + 1, i_LineDim);
    dim->margin = margin;
}

/*---------------------------------------------------------------------------*/

void layout_hexpand(Layout *layout, const uint32_t col)
{
    cassert_no_null(layout);
	cassert(col < arrst_size(layout->lines_dim[0], i_LineDim));
    arrst_foreach(dim, layout->lines_dim[0], i_LineDim)
        dim->resize_percent = (dim_i == col) ? 1.f : 0.f;
    arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_hexpand2(Layout* layout, const uint32_t col1, const uint32_t col2, const real32_t exp)
{
	cassert_no_null(layout);
	cassert(col1 != col2);
	cassert(col1 < arrst_size(layout->lines_dim[0], i_LineDim));
	cassert(col2 < arrst_size(layout->lines_dim[0], i_LineDim));
    cassert(exp <= 1);
	arrst_foreach(dim, layout->lines_dim[0], i_LineDim)
		if (dim_i == col1)
			dim->resize_percent = exp;
		else if (dim_i == col2)
			dim->resize_percent = 1.f - exp;
		else
			dim->resize_percent = 0.f;
	arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_hexpand3(Layout* layout, const uint32_t col1, const uint32_t col2, const uint32_t col3, const real32_t exp1, const real32_t exp2)
{
	cassert_no_null(layout);
	cassert(col1 != col2);
	cassert(col1 < arrst_size(layout->lines_dim[0], i_LineDim));
	cassert(col2 < arrst_size(layout->lines_dim[0], i_LineDim));
    cassert(exp1 + exp2 <= 1);
	arrst_foreach(dim, layout->lines_dim[0], i_LineDim)
		if (dim_i == col1)
			dim->resize_percent = exp1;
		else if (dim_i == col2)
			dim->resize_percent = exp2;
		else if (dim_i == col3)
			dim->resize_percent = 1.f - exp1 - exp2;
		else
			dim->resize_percent = 0.f;
	arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_vexpand(Layout *layout, const uint32_t row)
{
    cassert_no_null(layout);
	cassert(row < arrst_size(layout->lines_dim[1], i_LineDim));
    arrst_foreach(dim, layout->lines_dim[1], i_LineDim)
        dim->resize_percent = (dim_i == row) ? 1.f : 0.f;
    arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_vexpand2(Layout* layout, const uint32_t row1, const uint32_t row2, const real32_t exp)
{
	cassert_no_null(layout);
	cassert(row1 != row2);
	cassert(row1 < arrst_size(layout->lines_dim[1], i_LineDim));
	cassert(row2 < arrst_size(layout->lines_dim[1], i_LineDim));
    cassert(exp <= 1);
	arrst_foreach(dim, layout->lines_dim[1], i_LineDim)
		if (dim_i == row1)
			dim->resize_percent = exp;
		else if (dim_i == row2)
			dim->resize_percent = 1.f - exp;
		else
			dim->resize_percent = 0.f;
	arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_vexpand3(Layout* layout, const uint32_t row1, const uint32_t row2, const uint32_t row3, const real32_t exp1, const real32_t exp2)
{
	cassert_no_null(layout);
	cassert(row1 != row2);
	cassert(row1 < arrst_size(layout->lines_dim[1], i_LineDim));
	cassert(row2 < arrst_size(layout->lines_dim[1], i_LineDim));
    cassert(exp1 + exp2 <= 1);
	arrst_foreach(dim, layout->lines_dim[1], i_LineDim)
		if (dim_i == row1)
			dim->resize_percent = exp1;
		else if (dim_i == row2)
			dim->resize_percent = exp2;
		else if (dim_i == row3)
			dim->resize_percent = 1.f - exp1 - exp2;
		else
			dim->resize_percent = 0.f;
	arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_halign(Layout *layout, const uint32_t col, const uint32_t row, const align_t align)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cell->dim[0].align = align;
}

/*---------------------------------------------------------------------------*/

void layout_valign(Layout *layout, const uint32_t col, const uint32_t row, const align_t align)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cell->dim[1].align = align;
}

/*---------------------------------------------------------------------------*/

void layout_show_col(Layout *layout, const uint32_t col, const bool_t visible)
{
    register uint32_t i, num_rows;
    cassert_no_null(layout);
	cassert(col < arrst_size(layout->lines_dim[0], i_LineDim));
    num_rows = arrst_size(layout->lines_dim[1], i_LineDim);
    for (i = 0; i < num_rows; ++i)
    {
        Cell *cell = i_get_cell(layout, col, i);
        cassert_no_null(cell);
        cell->displayed = visible;
    }
}

/*---------------------------------------------------------------------------*/

void layout_show_row(Layout *layout, const uint32_t row, const bool_t visible)
{
    register uint32_t i, num_cols;
    cassert_no_null(layout);
	cassert(row < arrst_size(layout->lines_dim[1], i_LineDim));
    num_cols = arrst_size(layout->lines_dim[0], i_LineDim);
    for (i = 0; i < num_cols; ++i)
    {
        Cell *cell = i_get_cell(layout, i, row);
        cassert_no_null(cell);
        cell->displayed = visible;
    }
}

/*---------------------------------------------------------------------------*/

void layout_margin(Layout *layout, const real32_t mall)
{
    i_LineDim *first_column = NULL;
    i_LineDim *first_row = NULL;
    cassert_no_null(layout);
    first_column = arrst_get(layout->lines_dim[0], 0, i_LineDim);
    first_row = arrst_get(layout->lines_dim[1], 0, i_LineDim);
    first_column->margin = mall;
    first_row->margin = mall;
    layout->dim_margin[0] = mall;
    layout->dim_margin[1] = mall;
}

/*---------------------------------------------------------------------------*/

void layout_margin2(Layout *layout, const real32_t mtb, const real32_t mlr)
{
    i_LineDim *first_column = NULL;
    i_LineDim *first_row = NULL;
    cassert_no_null(layout);
    first_column = arrst_get(layout->lines_dim[0], 0, i_LineDim);
    first_row = arrst_get(layout->lines_dim[1], 0, i_LineDim);
    first_column->margin = mlr;
    first_row->margin = mtb;
    layout->dim_margin[0] = mlr;
    layout->dim_margin[1] = mtb;
}

/*---------------------------------------------------------------------------*/

void layout_margin4(Layout *layout, const real32_t mt, const real32_t mr, const real32_t mb, const real32_t ml)
{
    i_LineDim *first_column = NULL;
    i_LineDim *first_row = NULL;
    cassert_no_null(layout);
    first_column = arrst_get(layout->lines_dim[0], 0, i_LineDim);
    first_row = arrst_get(layout->lines_dim[1], 0, i_LineDim);
    first_column->margin = ml;
    first_row->margin = mt;
    layout->dim_margin[0] = mr;
    layout->dim_margin[1] = mb;
}

/*---------------------------------------------------------------------------*/

void layout_bgcolor(Layout *layout, const color_t color)
{
    cassert_no_null(layout);
    layout->bgcolor = color;
}

/*---------------------------------------------------------------------------*/

void layout_skcolor(Layout *layout, const color_t color)
{
    cassert_no_null(layout);
    layout->skcolor = color;
}

/*---------------------------------------------------------------------------*/

void layout_update(const Layout *layout)
{
    cassert_no_null(layout);
    if (layout->panel != NULL)
    {
        Window * window = _panel_get_window(layout->panel);
        if (window != NULL)
            _window_update(window);
    }
}

/*---------------------------------------------------------------------------*/

void layout_dbind_imp(Layout *layout, Listener *listener, const char_t *type, const uint16_t size)
{
    cassert_no_null(layout);
    cassert(layout->stbind == NULL);
    cassert(layout->objbind == NULL);
    cassert(layout->OnObjChange == NULL);
    layout->stbind = dbind_stbind(type);
    layout->OnObjChange = listener;
    cassert_unref(dbind_stbind_sizeof(layout->stbind) == size, size);
}

/*---------------------------------------------------------------------------*/

static void i_layout_dbind(Layout *layout, const StBind *stbind, void *obj)
{
    arrst_foreach(cell, layout->cells, Cell)
    if (cell->dbind != NULL)
    {
        switch (cell->type) {
        case i_ekCOMPONENT:
            if (cell->content.component->type == ekGUI_TYPE_PANEL)
            {
                Panel *panel = (Panel*)cell->content.component;
                ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
                arrpt_foreach(panel_layout, panel_layouts, Layout)
                    gbind_upd_layout(panel_layout, stbind, cell->dbind, obj);
                arrpt_end();
            }
            else
            {
                gbind_upd_component(cell, stbind, cell->dbind, obj);
            }
            break;

        case i_ekLAYOUT:
            gbind_upd_layout(cell->content.layout, stbind, cell->dbind, obj);
            break;

        case i_ekEMPTY:
        cassert_default();
        }
    }
    else if (cell->type == i_ekLAYOUT)
    {
        i_layout_dbind(cell->content.layout, stbind, obj);
    }
    else if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_PANEL)
    {
        Panel *panel = (Panel*)cell->content.component;
        ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
        arrpt_foreach(panel_layout, panel_layouts, Layout)
            if (panel_layout->stbind == NULL)
                i_layout_dbind(panel_layout, stbind, obj);
        arrpt_end();
    }
    arrst_end();
}

/*---------------------------------------------------------------------------*/

void layout_dbind_obj_imp(Layout *layout, void *obj, const char_t *type)
{
    cassert_no_null(layout);
    cassert_unref(str_equ_c(dbind_stbind_type(layout->stbind), type) == TRUE, type);
    layout->objbind = obj;
    i_layout_dbind(layout, layout->stbind, layout->objbind);
}

/*---------------------------------------------------------------------------*/

void layout_dbind_update_imp(Layout *layout, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize)
{
    const StBind *stbind = dbind_stbind(type);
	const DBind *dbind = dbind_stbind_find(stbind, mname);
    cassert_unref(dbind_data_type(mtype, NULL, NULL) == dbind_type(dbind), mtype);
	cassert_unref(dbind_stbind_sizeof(stbind) == size, size);
    cassert_unref(dbind_offset(dbind) == moffset, moffset);
	cassert_unref(dbind_sizeof(dbind) == msize, msize);
	_layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void _layout_attach_to_panel(Layout *layout, Panel *panel)
{
    cassert_no_null(layout);
    cassert(layout->panel == NULL || layout->panel == panel);
    layout->panel = panel;
    arrst_foreach(cell, layout->cells, Cell)
        switch (cell->type)
        {
            case i_ekCOMPONENT:
                _panel_attach_component(panel, cell->content.component);
                break;
            case i_ekLAYOUT:
                _layout_attach_to_panel(cell->content.layout, panel);
                break;
            case i_ekEMPTY:
                break;
            cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

Panel *_layout_panel(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->panel;
}

/*---------------------------------------------------------------------------*/

/*
Layout *_layout_search_component(const Layout *layout, const GuiComponent *component);
Layout *_layout_search_component(const Layout *layout, const GuiComponent *component)
{
    Layout *find_layout = NULL;
    cassert_no_null(layout);
    cassert_no_null(layout->panel);

    arrst_foreach(cell, layout->cells, Cell)
        if (cell->type == i_ekCOMPONENT)
        {
            cassert_no_null(cell->content.component);
            if (cell->content.component == component)
            {
                find_layout = (Layout*)layout;
                break;
            }
        }
        else if (cell->type == i_ekLAYOUT)
        {
            find_layout = _layout_search_component(cell->content.layout, component);
            if (find_layout != NULL)
                break;
        }
    arrst_end()

    return find_layout;
}*/

/*---------------------------------------------------------------------------*/

bool_t _layout_search_layout(const Layout *layout, Layout *sublayout)
{
    cassert_no_null(layout);
    cassert_no_null(layout->panel);
    if (layout == sublayout)
    {
        return TRUE;
    }
    else
    {
        arrst_foreach(cell, layout->cells, Cell)
            if (cell->type == i_ekLAYOUT)
            {
                bool_t exists = _layout_search_layout(cell->content.layout, sublayout);
                if (exists == TRUE)
                    return exists;
            }
        arrst_end()

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

/*
uint32_t _layout_button_index(const Layout *layout, const Button *button)
{
    uint32_t index = 0;
    cassert_no_null(layout);
    arrst_foreach(cell, layout->cells, Cell)
    if (cell->type == i_ekCOMPONENT)
    {
        cassert_no_null(cell->content.component);
        if (cell->content.component->type == ekGUI_TYPE_BUTTON)
        {
            if ((Button*)cell->content.component == button)
                return index;
            index += 1;
        }
    }
    arrst_end()
    cassert(FALSE);
    return UINT32_MAX;
}*/

/*---------------------------------------------------------------------------*/

/*
void *_layout_get_object_by_tag(const Layout *layout, const uint32_t tag, const char_t *type);
void *_layout_get_object_by_tag(const Layout *layout, const uint32_t tag, const char_t *type)
{
    void *component = NULL;

    cassert_no_null(layout);
    if (layout->tag == tag)
    {
        cassert(str_equ_c(type, "Layout") == TRUE);
        return (void*)layout;
    }

    arrst_foreach(cell, layout->cells, Cell)

        if (cell->type == i_ekCOMPONENT)
        {
            cassert_no_null(cell->content.component);
            if (cell->content.component->tag.tag_uint32 == tag)
            {
                cassert(str_equ_c(_gui_component_type(cell->content.component->type), type) == TRUE);
                component = cell->content.component;
                break;
            }
            else
            {
                Panel *panels[GUI_COMPONENT_MAX_PANELS];
                uint32_t num_panels = UINT32_MAX, i = 0;
                _component_panels(cell->content.component, &num_panels, panels);
                for (i = 0; i < num_panels; ++i)
                {
                    component = _panel_get_object_by_tag(panels[i], tag, type);
                    if (component != NULL)
                        break;
                }
            }
        }
        else if (cell->type == i_ekLAYOUT)
        {
            component = _layout_get_object_by_tag(cell->content.layout, tag, type);
            if (component != NULL)
                break;
        }

    arrst_end()

    return component;
}*/

/*---------------------------------------------------------------------------*/

/*
bool_t _layout_exists_layout(const Layout *layout, Layout *sublayout);
bool_t _layout_exists_layout(const Layout *layout, Layout *sublayout)
{
    cassert_no_null(layout);
    cassert_no_null(layout->panel);
    if (layout == sublayout)
    {
        return TRUE;
    }
    else
    {
        bool_t exists = FALSE;
        arrst_foreach(cell, layout->cells, Cell)
            if (cell->type == i_ekLAYOUT)
            {
                exists = _layout_exists_layout(cell->content.layout, sublayout);
                if (exists == TRUE)
                    break;
            }
        arrst_end()

        return exists;
    }
}*/

/*---------------------------------------------------------------------------*/

void _layout_components(const Layout *layout, ArrPt(GuiComponent) *components)
{
    cassert_no_null(layout);
    arrst_foreach(cell, layout->cells, Cell)
        switch (cell->type) {
        case i_ekCOMPONENT:
            cassert_no_null(cell->content.component);
            cassert(arrpt_find(components, cell->content.component, GuiComponent) == UINT32_MAX);
            arrpt_append(components, cell->content.component, GuiComponent);
            break;
        case i_ekLAYOUT:
            _layout_components(cell->content.layout, components);
            break;
        case i_ekEMPTY:
            break;
        cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_line_compose(i_LineDim *dim, const uint32_t di, Cell **cell, const uint32_t n_cells)
{
    register uint32_t i = 0;
    cassert_no_null(dim);
    cassert_no_null(cell);
    cassert(di <= 1);
    dim->size = 0.f;
    for (i = 0; i < n_cells; ++i)
    {
        real32_t forced_size = 0;
        if (cell[i]->displayed == TRUE)
        {
            switch (cell[i]->type)
            {
                case i_ekCOMPONENT:
                    _component_dimension(cell[i]->content.component, di, &cell[i]->dim[0].size, &cell[i]->dim[1].size);
                    break;

                case i_ekLAYOUT:
                    _layout_dimension(cell[i]->content.layout, di, &cell[i]->dim[0].size, &cell[i]->dim[1].size);
                    break;

                case i_ekEMPTY:
                    cell[i]->dim[di].size = 0;
                    break;
            }
        }
        else
        {
            cell[i]->dim[di].size = 0;
        }

        /* Forced size == Internal resizing */
        if (cell[i]->dim[di].forced_size > 0)
            forced_size = cell[i]->dim[di].forced_size;
        else if (dim->forced_size > 0)
            forced_size = dim->forced_size;

        if (forced_size > 0)
        {
            forced_size -= cell[i]->dim[di].padding_after;
            forced_size -= cell[i]->dim[di].padding_before;
            cassert_msg(forced_size > 0.f, "Cell forced size smaller than padding");

            if (cell[i]->displayed == TRUE)
            {
                if (cell[i]->dim[di].align == ekJUSTIFY)
                {
                    switch (cell[i]->type)
                    {
                        case i_ekCOMPONENT:
                            _component_expand(cell[i]->content.component, di, cell[i]->dim[di].size, forced_size, &cell[i]->dim[di].size);
                            break;

                        case i_ekLAYOUT:
                            _layout_expand(cell[i]->content.layout, di, cell[i]->dim[di].size, forced_size, &cell[i]->dim[di].size);
                            break;

                        case i_ekEMPTY:
                            cell[i]->dim[di].size = forced_size;
                            break;
                    }
                }
                else
                {
                    if (cell[i]->dim[di].size > forced_size)
                        cell[i]->dim[di].size = forced_size;
                }
            }
        }

        {
            real32_t total_cell_size = cell[i]->dim[di].size;
            total_cell_size += cell[i]->dim[di].padding_after;
            total_cell_size += cell[i]->dim[di].padding_before;

            if (total_cell_size > dim->size)
                dim->size = total_cell_size;
        }
    }
}

/*---------------------------------------------------------------------------*/

void _layout_dimension(Layout *layout, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    real32_t size = 0.f;
    Cell **cells = NULL;
    cassert_no_null(layout);
    cassert(di <= 1);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    cells = arrpt_all(layout->cells_dim[di], Cell);

    arrst_foreach(dim, layout->lines_dim[di], i_LineDim)
        size += dim->margin;
        i_line_compose(dim, di, cells, layout->dim_num_elems[di]);
        size += dim->size;
        cells += layout->dim_num_elems[di];
    arrst_end()

    size += layout->dim_margin[di];

    if (di == 0)
        *dim0 = size;
    else
        *dim1 = size;
}

/*---------------------------------------------------------------------------*/

static void i_dimension_resize(ArrSt(i_LineDim) *dim, const real32_t current_size, const real32_t required_size)
{
    real32_t diff = required_size - current_size;
    real32_t total = 0;
    uint32_t last_id = UINT32_MAX;
    real32_t norm = 0;

    cassert_no_null(dim);
    arrst_foreach(edim, dim, i_LineDim)
        norm += edim->resize_percent;
        if (edim->resize_percent > 0)
            last_id = edim_i;
    arrst_end()

    cassert_unref(bmath_absf(1 - norm) < 0.00001f, norm);

    arrst_foreach(edim, dim, i_LineDim)
        real32_t increment = 0;

        if (edim_i == last_id)
            increment = diff - total;
        else
            increment = bmath_roundf(diff * edim->resize_percent);

        edim->size += increment;
        total += increment;

        if (edim->size < 0)
            edim->size = 0;

        cassert(edim->size >= 0);
    arrst_end()

    cassert(bmath_absf(diff - total) < 0.00001f);
}

/*---------------------------------------------------------------------------*/

static bool_t i_line_expand(real32_t *size, const uint32_t di, Cell **cell, const uint32_t n_cells)
{
    register uint32_t i = 0;
    for (i = 0; i < n_cells; ++i)
    {
        if (cell[i]->displayed == FALSE)
            continue;

        switch (cell[i]->dim[di].align) {
        case ekJUSTIFY:
        {
            real32_t cell_diff = *size - cell[i]->dim[di].size - cell[i]->dim[di].padding_after - cell[i]->dim[di].padding_before;
            real32_t new_size = cell[i]->dim[di].size + cell_diff;

            switch (cell[i]->type) {
            case i_ekCOMPONENT:
                _component_expand(cell[i]->content.component, di, cell[i]->dim[di].size, new_size, &cell[i]->dim[di].size);
                break;

            case i_ekLAYOUT:
                _layout_expand(cell[i]->content.layout, di, cell[i]->dim[di].size, new_size, &cell[i]->dim[di].size);
                break;

            case i_ekEMPTY:
                break;

            cassert_default();
            }

            break;
        }

        case ekLEFT:
        case ekRIGHT:
        case ekCENTER:
            switch (cell[i]->type) {
            case i_ekCOMPONENT:
                _component_expand(cell[i]->content.component, di, cell[i]->dim[di].size, cell[i]->dim[di].size, &cell[i]->dim[di].size);
                break;

            case i_ekLAYOUT:
                _layout_expand(cell[i]->content.layout, di, cell[i]->dim[di].size, cell[i]->dim[di].size, &cell[i]->dim[di].size);
                break;

            case i_ekEMPTY:
                break;

            cassert_default();
            }

            break;

        default:
			cassert(cell[i]->dim[di].align == ENUM_MAX(align_t));
			cassert(cell[i]->type == i_ekEMPTY);
        }

        /* A col/row can not be narrowed below what is allowed in a cell. */
        if (cell[i]->dim[di].size > *size)
        {
            *size = cell[i]->dim[di].size;
            return FALSE;
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

void _layout_expand(Layout *layout, const uint32_t di, const real32_t current_size, const real32_t required_size, real32_t *final_size)
{
    Cell **cells = NULL;
    cassert_no_null(layout);
    i_dimension_resize(layout->lines_dim[di], current_size, required_size);
    cells = arrpt_all(layout->cells_dim[di], Cell);
    *final_size = 0.f;
    arrst_foreach(edim, layout->lines_dim[di], i_LineDim)
        while(i_line_expand(&edim->size, di, cells, layout->dim_num_elems[di]) == FALSE);
        *final_size += edim->size;
        *final_size += edim->margin;
        cells += layout->dim_num_elems[di];
    arrst_end()
    *final_size += layout->dim_margin[di];
}

/*---------------------------------------------------------------------------*/

void _layout_compose(Layout *layout, const S2Df *required_size, S2Df *final_size)
{
    S2Df original_size = kS2D_ZEROf;
    cassert_no_null(layout);
    cassert_no_null(final_size);
    _layout_dimension(layout, 0, &original_size.width, &original_size.height);
    _layout_expand(layout, 0, original_size.width, required_size != NULL ? required_size->width : original_size.width, &final_size->width);
    cassert(original_size.height == 0.f);
    _layout_dimension(layout, 1, &original_size.width, &original_size.height);
    _layout_expand(layout, 1, original_size.height, required_size != NULL ? required_size->height : original_size.height, &final_size->height);
}

/*---------------------------------------------------------------------------*/

static real32_t i_dimension_size(const ArrSt(i_LineDim) *dim, const real32_t margin)
{
    real32_t size = margin;
    arrst_foreach_const(edim, dim, i_LineDim)
        size += edim->margin;
        size += edim->size;
    arrst_end()
    return size;
}

/*---------------------------------------------------------------------------*/

static void i_layout_locate(Layout *layout, const V2Df *origin, FPtr_gctx_set_area func_area, void *ospanel)
{
    register uint32_t i, j, ncols, nrows;
    register const i_LineDim *cols, *rows;
    register const Cell *cells = NULL;
    V2Df lorigin = *origin;
    real32_t xorigin = lorigin.x;

    cassert_no_null(layout);
    if (layout->bgcolor != 0 || layout->skcolor != 0)
    {
        real32_t width = i_dimension_size(layout->lines_dim[0], layout->dim_margin[0]);
        real32_t height = i_dimension_size(layout->lines_dim[1], layout->dim_margin[1]);
        func_area(ospanel, layout, layout->bgcolor, layout->skcolor, lorigin.x, lorigin.y, width, height);
    }

    ncols = arrst_size(layout->lines_dim[0], i_LineDim);
    nrows = arrst_size(layout->lines_dim[1], i_LineDim);
    cols = arrst_all(layout->lines_dim[0], i_LineDim);
    rows = arrst_all(layout->lines_dim[1], i_LineDim);
    cells = arrst_all(layout->cells, Cell);
    for (i = 0; i < nrows; ++i)
    {
        lorigin.y += rows[i].margin;
        lorigin.x = xorigin;
        for (j = 0; j < ncols; ++j)
        {
            register uint32_t p = i * ncols + j;
            register const Cell *cell = cells + p;

            lorigin.x += cols[j].margin;

            if (cell->displayed == TRUE)
            {
                V2Df cell_origin;
                S2Df cell_size;

                cell_origin.x = lorigin.x + cell->dim[0].padding_after;
                cell_origin.y = lorigin.y + cell->dim[1].padding_after;
                cell_size.width = cell->dim[0].size;
                cell_size.height = cell->dim[1].size;

                switch (cell->dim[0].align) {
                case ekLEFT:
                case ekJUSTIFY:
                    break;

                case ekRIGHT:
                {
                    real32_t cell_diff = cols[j].size - cell->dim[0].size - cell->dim[0].padding_after - cell->dim[0].padding_before;
                    cell_origin.x += cell_diff;
                    break;
                }

                case ekCENTER:
                {
                    real32_t cell_diff = cols[j].size - cell->dim[0].size - cell->dim[0].padding_after - cell->dim[0].padding_before;
                    cell_origin.x += bmath_floorf(.5f * cell_diff);
                    break;
                }
                }

                switch (cell->dim[1].align) {
                case ekTOP:
                case ekJUSTIFY:
                    break;

                case ekBOTTOM:
                {
                    real32_t cell_diff = rows[i].size - cell->dim[1].size - cell->dim[1].padding_after - cell->dim[1].padding_before;
                    cell_origin.y += cell_diff;
                    break;
                }

                case ekCENTER:
                {
                    real32_t cell_diff = rows[i].size - cell->dim[1].size - cell->dim[1].padding_after - cell->dim[1].padding_before;
                    cell_origin.y += bmath_floorf(.5f * cell_diff);
                    break;
                }
                }

                switch (cell->type)
                {
                    case i_ekCOMPONENT:
                        _component_set_frame(cell->content.component, &cell_origin, &cell_size);
                        _component_locate(cell->content.component);
                        break;

                    case i_ekLAYOUT:
                        i_layout_locate(cell->content.layout, &cell_origin, func_area, ospanel);
                        break;

                    case i_ekEMPTY:
                        break;
                    cassert_default();
                }
            }

            lorigin.x += cols[j].size;
        }

        lorigin.y += rows[i].size;
    }
}

/*---------------------------------------------------------------------------*/

static void i_layout_visible(Layout *layout, const bool_t parent_visible)
{
    cassert_no_null(layout);
    arrst_foreach(cell, layout->cells, Cell)
        bool_t visible = cell->visible && cell->displayed && parent_visible;

        switch (cell->type) {
        case i_ekCOMPONENT:
            _component_visible(cell->content.component, visible);
            break;
        case i_ekLAYOUT:
            i_layout_visible(cell->content.layout, visible);
            break;
        case i_ekEMPTY:
            break;
        cassert_default();
        }

    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_layout_enabled(Layout *layout, const bool_t parent_enabled)
{
    cassert_no_null(layout);
    arrst_foreach(cell, layout->cells, Cell)
        bool_t enabled = cell->enabled && parent_enabled;

        switch (cell->type) {
        case i_ekCOMPONENT:
            _component_enabled(cell->content.component, enabled);
            break;
        case i_ekLAYOUT:
            i_layout_enabled(cell->content.layout, enabled);
            break;
        case i_ekEMPTY:
            break;
        cassert_default();
        }

    arrst_end()
}

/*---------------------------------------------------------------------------*/

void _layout_locate(Layout *layout)
{
    void *ospanel = NULL;
    FPtr_gctx_set_area func_area = NULL;
    cassert_no_null(layout);

    if (layout->panel != NULL)
    {
        ospanel = ((GuiComponent*)layout->panel)->ositem;
        func_area = ((GuiComponent*)layout->panel)->context->func_panel_area;
        func_area(ospanel, NULL, 0, 0, 0, 0, 0, 0);
    }

    i_layout_locate(layout, &kV2D_ZEROf, func_area, ospanel);
    i_layout_visible(layout, TRUE);
    i_layout_enabled(layout, TRUE);

    if (layout->panel != NULL)
        ((GuiComponent*)layout->panel)->context->func_panel_set_need_display(ospanel);
}

/*---------------------------------------------------------------------------*/

static void i_cell_taborder(const Cell *cell, Window *window)
{
    cassert_no_null(cell);
    if (cell->displayed == TRUE && cell->tabstop == TRUE)
    {
        switch (cell->type) {
        case i_ekLAYOUT:
            _layout_taborder(cell->content.layout, window);
            break;

        case i_ekCOMPONENT:
            _component_taborder(cell->content.component, window);
            break;

        case i_ekEMPTY:
            break;
        cassert_default();
        }
    }
}

/*---------------------------------------------------------------------------*/

void _layout_taborder(const Layout *layout, Window *window)
{
    register const Cell *cells;
    register uint32_t i, j, num_columns, num_rows;
    cassert_no_null(layout);
    cells = arrst_all(layout->cells, Cell);
    num_columns = arrst_size(layout->lines_dim[0], i_LineDim);
    num_rows = arrst_size(layout->lines_dim[1], i_LineDim);
    if (layout->is_row_major_tab == TRUE)
    {
        for (i = 0; i < num_rows; ++i)
        {
            for (j = 0; j < num_columns; ++j)
                i_cell_taborder(cells + (i * num_columns + j), window);
        }
    }
    else
    {
        for (i = 0; i < num_columns; ++i)
        {
            for (j = 0; j < num_rows; ++j)
                i_cell_taborder(cells + (j * num_columns + i), window);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_dbind_update(Layout *layout, const StBind *stbind, const DBind *dbind, void *obj)
{
    arrst_foreach(cell, layout->cells, Cell)
        bool_t update = FALSE;

        /* Update a single object-field (dbind != NULL) */
        if (dbind != NULL && cell->dbind == dbind)
            update = TRUE;

        /* Update the entire object (dbind == NULL) */
        else if (dbind == NULL && cell->dbind != NULL)
            update = TRUE;

        if (update)
        {
            switch (cell->type) {
            case i_ekCOMPONENT:
                gbind_upd_component(cell, stbind, cell->dbind, obj);
                break;
            case i_ekLAYOUT:
                gbind_upd_layout(cell->content.layout, stbind, cell->dbind, obj);
                break;
            case i_ekEMPTY:
            cassert_default();
            }
        }
        else if (cell->type == i_ekLAYOUT)
        {
            const StBind *lstbind = cell->content.layout->stbind;
            void *lobjbind = cell->content.layout->objbind;
            if (lstbind != NULL)
                i_dbind_update(cell->content.layout, lstbind, dbind, lobjbind);
            else
                i_dbind_update(cell->content.layout, stbind, dbind, obj);
        }
        else if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_PANEL)
        {
            Panel *panel = (Panel*)cell->content.component;
            ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
            arrpt_foreach(panel_layout, panel_layouts, Layout)
                const StBind *lstbind = panel_layout->stbind;
                void *lobjbind = panel_layout->objbind;
                if (lstbind != NULL)
                    i_dbind_update(panel_layout, lstbind, dbind, lobjbind);
                else
                    i_dbind_update(panel_layout, stbind, dbind, obj);
            arrpt_end();
        }
    arrst_end();
}

/*---------------------------------------------------------------------------*/

void _layout_dbind_update(Layout *layout, const DBind *dbind)
{
    cassert_no_null(layout);
    cassert_no_null(dbind);
    cassert(layout->objbind != NULL);
    i_dbind_update(layout, layout->stbind, dbind, layout->objbind);
}

/*---------------------------------------------------------------------------*/

ArrSt(Cell)* _layout_cells(Layout* layout)
{
    cassert_no_null(layout);
    return layout->cells;
}

/*---------------------------------------------------------------------------*/

void _layout_notif(Layout *layout, void **obj, const char_t **obj_type, Listener **listener)
{
	cassert_no_null(layout);
	cassert_no_null(obj);
	cassert_no_null(obj_type);
	cassert_no_null(listener);
	*obj = layout->objbind;
	*obj_type = dbind_stbind_type(layout->stbind);
	*listener = layout->OnObjChange;
}

/*---------------------------------------------------------------------------*/

void *cell_control_imp(Cell *cell, const char_t *type)
{
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    cassert_unref(str_equ_c(_component_type(cell->content.component), type) == TRUE, type);
    return (void*)cell->content.component;
}

/*---------------------------------------------------------------------------*/

Layout *cell_layout(Cell *cell)
{
    cassert_no_null(cell);
    cassert(cell->type == i_ekLAYOUT);
    return cell->content.layout;
 }

/*---------------------------------------------------------------------------*/

void cell_enabled(Cell *cell, const bool_t enabled)
{
    cassert_no_null(cell);
    if (cell->enabled != enabled)
    {
        cell->enabled = enabled;
        switch (cell->type)
        {
            case i_ekCOMPONENT:
                _component_enabled(cell->content.component, enabled);
                break;
            case i_ekLAYOUT:
                i_layout_enabled(cell->content.layout, enabled);
                break;
            case i_ekEMPTY:
                break;
            cassert_default();
        }
    }
}

/*---------------------------------------------------------------------------*/

void cell_visible(Cell *cell, const bool_t visible)
{
    cassert_no_null(cell);
    if (cell->visible != visible)
    {
        cassert(cell->type == i_ekCOMPONENT);
        cell->visible = visible;
        _component_visible(cell->content.component, visible);
    }
}

/*---------------------------------------------------------------------------*/

void cell_focus(Cell *cell)
{
    register GuiComponent *component;
    register Window *window;
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    cassert_no_null(cell->parent);
    component = cell->content.component;
    window = _panel_get_window(cell->parent->panel);
    component->context->func_window_set_focus(_window_ositem(window), component->ositem);
}

/*---------------------------------------------------------------------------*/

void cell_padding(Cell *cell, const real32_t pall)
{
    cassert_no_null(cell);
    cell->dim[0].padding_after = pall;
    cell->dim[0].padding_before = pall;
    cell->dim[1].padding_after = pall;
    cell->dim[1].padding_before = pall;
}

/*---------------------------------------------------------------------------*/

void cell_padding2(Cell *cell, const real32_t ptb, const real32_t plr)
{
    cassert_no_null(cell);
    cell->dim[0].padding_after = plr;
    cell->dim[0].padding_before = plr;
    cell->dim[1].padding_after = ptb;
    cell->dim[1].padding_before = ptb;
}

/*---------------------------------------------------------------------------*/

void cell_padding4(Cell *cell, const real32_t pt, const real32_t pr, const real32_t pb, const real32_t pl)
{
    cassert_no_null(cell);
    cell->dim[0].padding_after = pl;
    cell->dim[0].padding_before = pr;
    cell->dim[1].padding_after = pt;
    cell->dim[1].padding_before = pb;
}

/*---------------------------------------------------------------------------*/

static void i_set_dbind(Cell *cell, const DBind *dbind)
{
    cassert(cell->dbind == NULL);
    cell->dbind = dbind;
    if (cell->type == i_ekCOMPONENT)
    {
        dtype_t dtype = dbind_type(dbind);

        switch (cell->content.component->type) {
        case ekGUI_TYPE_POPUP:
        {
            PopUp* popup = (PopUp*)cell->content.component;
            if (dtype == ekDTYPE_ENUM && _popup_size(popup) == 0)
            {
                register uint32_t i, n = dbind_enum_count(dbind);
                for (i = 0; i < n; ++i)
                {
                    const char_t* alias = dbind_enum_alias(dbind, i);
                    _popup_add_enum_item(popup, alias);
                }

                _popup_list_height(popup, n < 15 ? n : 15);
            }

            break;
        }

        case ekGUI_TYPE_CUSTOMVIEW:
		{
            View *view = (View*)cell->content.component;
			if (str_equ_c(_view_subtype(view), "ListBox") == TRUE)
			{
				ListBox *listbox = (ListBox*)cell->content.component;
				if (dtype == ekDTYPE_ENUM && _listbox_count(listbox) == 0)
				{
					register uint32_t i, n = dbind_enum_count(dbind);
					for (i = 0; i < n; ++i)
					{
						const char_t* alias = dbind_enum_alias(dbind, i);
						_listbox_add_enum_item(listbox, alias);
					}
				}
			}

			break;
		}

        case ekGUI_TYPE_EDITBOX:
            if (dbind_is_number_type(dbind) == TRUE)
            {
                Edit* edit = (Edit*)cell->content.component;
                edit_autoselect(edit, TRUE);
            }
            break;

        case ekGUI_TYPE_LABEL:
        case ekGUI_TYPE_BUTTON:
        case ekGUI_TYPE_COMBOBOX:
        case ekGUI_TYPE_SLIDER:
        case ekGUI_TYPE_UPDOWN:
        case ekGUI_TYPE_PROGRESS:
			break;

        case ekGUI_TYPE_TEXTVIEW:
        case ekGUI_TYPE_TABLEVIEW:
        case ekGUI_TYPE_TREEVIEW:
        case ekGUI_TYPE_BOXVIEW:
        case ekGUI_TYPE_SPLITVIEW:
        case ekGUI_TYPE_PANEL:
        case ekGUI_TYPE_LINE:
        case ekGUI_TYPE_HEADER:
        case ekGUI_TYPE_WINDOW:
        case ekGUI_TYPE_TOOLBAR:
        cassert_default();
        }
    }
    else if (cell->type == i_ekLAYOUT && dbind_is_basic_type(dbind) == TRUE)
    {
        arrst_foreach(lcell, cell->content.layout->cells, Cell)
            i_set_dbind(lcell, dbind);
        arrst_end();
    }
}

/*---------------------------------------------------------------------------*/

void cell_dbind_imp(
                    Cell *cell,
                    const char_t *type,
                    const uint16_t size,
                    const char_t *mname,
                    const char_t *mtype,
                    const uint16_t moffset,
                    const uint16_t msize)
{
    const StBind *stbind = dbind_stbind(type);
    const DBind *dbind = dbind_stbind_find(stbind, mname);
    uint16_t stsize = dbind_stbind_sizeof(stbind);
    cassert_no_null(cell);
    cassert_no_null(dbind);
    cassert_unref(stsize == size, stsize);
    cassert_unref(dbind_data_type(mtype, NULL, NULL) == dbind_type(dbind), mtype);
    cassert_unref(dbind_offset(dbind) == moffset, moffset);
    cassert_unref(dbind_sizeof(dbind) == msize, msize);
    i_set_dbind(cell, dbind);
}

/*---------------------------------------------------------------------------*/

GuiComponent *_cell_component(Cell *cell)
{
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    return cell->content.component;
}

/*---------------------------------------------------------------------------*/

Layout *_cell_parent(Cell *cell)
{
    cassert_no_null(cell);
    return cell->parent;
}

/*---------------------------------------------------------------------------*/

void _cell_set_radio(Cell *on_cell)
{
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrst_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT
            && cell->content.component->type == ekGUI_TYPE_BUTTON
            && _button_is_radio((const Button*)cell->content.component) )
        {
            _button_radio_state((Button*)cell->content.component, (cell == on_cell) ? ekGUI_ON : ekGUI_OFF);
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

void _cell_set_radio_index(Cell *on_cell, const uint32_t index)
{
    uint32_t i = 0;
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrst_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT
            && cell->content.component->type == ekGUI_TYPE_BUTTON
            && _button_is_radio((const Button*)cell->content.component))
        {
            _button_radio_state((Button*)cell->content.component, (i == index) ? ekGUI_ON : ekGUI_OFF);
            i += 1;
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

uint32_t _cell_radio_index(Cell *on_cell)
{
    uint32_t i = 0;
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrst_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT
            && cell->content.component->type == ekGUI_TYPE_BUTTON
            && _button_is_radio((const Button*)cell->content.component))
        {
            if (cell == on_cell)
                return i;
            else
                i += 1;
        }
    arrst_end()
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

Button *_cell_radio_listener(Cell *on_cell)
{
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrst_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT
            && cell->content.component->type == ekGUI_TYPE_BUTTON
            && _button_is_radio((const Button*)cell->content.component))
        {
            if (_button_radio_listener((const Button*)cell->content.component) != NULL)
                return (Button*)cell->content.component;
        }
    arrst_end()
    return NULL;
}

/*---------------------------------------------------------------------------*/

Cell *_cell_radio_dbind_cell(Cell *on_cell)
{
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrst_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT
            && cell->content.component->type == ekGUI_TYPE_BUTTON
            && _button_is_radio((const Button*)cell->content.component))
        {
            if (cell->dbind != NULL)
                return cell;
        }
    arrst_end()
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool_t _cell_filter_str(Cell *cell, const char_t *str, char_t *dest, const uint32_t size)
{
    if (cell->dbind != NULL)
        return dbind_string_filter(cell->dbind, str, dest, size);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static Layout *i_cell_obj(Cell *cell, void **obj, Layout **layout_notif)
{
    Layout *layout = cell->parent;
    *obj = NULL;
    *layout_notif = NULL;

    /* Find the closest parent layout with object data */
    while (layout->stbind == NULL && layout->parent != NULL)
        layout = layout->parent->parent;

    /* No object data --> Go up to parent panel layout */
    while (layout->stbind == NULL)
    {
        GuiComponent *component = _panel_get_component(layout->panel);

        /* Main panel */
        if (component->parent == NULL)
            break;

        layout = component->parent->parent;

        while (layout->stbind == NULL && layout->parent != NULL)
            layout = layout->parent->parent;
    }

    if (layout->stbind != NULL)
    {
        Layout *layout_notify = layout;
        cassert(dbind_get_stbind(cell->dbind) == layout->stbind);
        *obj = layout->objbind;

        /* Find the closest parent layout with notification data */
        while(layout_notify->OnObjChange == NULL && layout_notify->parent != NULL)
            layout_notify = layout_notify->parent->parent;

		if (layout_notify->OnObjChange != NULL)
			*layout_notif = layout_notify;
    }

    return layout;
}

/*---------------------------------------------------------------------------*/

void _cell_upd_bool(Cell *cell, const bool_t value)
{
    cassert_no_null(cell);
    if (cell->dbind != NULL)
    {
		void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (obj != NULL)
            gbind_upd_bool(layout, cell->dbind, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_upd_uint32(Cell *cell, const uint32_t value)
{
    cassert_no_null(cell);
    if (cell->dbind != NULL)
    {
		void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (obj != NULL)
            gbind_upd_uint32(layout, cell->dbind, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_upd_norm_real32(Cell *cell, const real32_t value)
{
    cassert_no_null(cell);
    if (cell->dbind != NULL)
    {
        void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (layout->objbind != NULL)
            gbind_upd_norm_real32(layout, cell->dbind, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_upd_string(Cell *cell, const char_t *str)
{
    cassert_no_null(cell);
    if (cell->dbind != NULL)
    {
        void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (layout->objbind != NULL)
            gbind_upd_string(layout, cell->dbind, obj, layout_notif, str);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_upd_image(Cell *cell, const Image *image)
{
    if (cell && cell->dbind != NULL)
    {
        void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (layout->objbind != NULL)
            gbind_upd_image(layout, cell->dbind, layout->objbind, image);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_upd_increment(Cell *cell, const bool_t pos)
{
    cassert_no_null(cell);
    if (cell->dbind != NULL)
    {
        void *obj = NULL;
		Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);

        if (layout->objbind != NULL)
            gbind_upd_increment(layout, cell->dbind, obj, layout_notif, pos);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _cell_enabled(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->enabled;
}
