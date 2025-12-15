/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: layout.c
 *
 */

/* Layouts */

#include "layout.h"
#include "layouth.h"
#include "layout.inl"
#include "cell.h"
#include "cell.inl"
#include "button.inl"
#include "component.inl"
#include "gbind.inl"
#include "guicontrol.h"
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
#include <geom2d/s2d.h>
#include <geom2d/v2d.h>
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/dbindh.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/objh.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#define LAYOUT_DEBUGGING
#undef LAYOUT_DEBUGGING

typedef struct i_line_dim_t i_LineDim;
typedef struct i_cell_dim_t i_CellDim;
typedef union i_cell_content_t i_CellContent;

/* Represents a grid partition (Column or Row) */
struct i_line_dim_t
{
    /* Fixed attributes. Change required execute compose algorithm */
    /* Computed attributes. Final results of compose algorithm  */
    bool_t displayed;      /* Fixed: If FALSE, no space allocation */
    real32_t forced_size;  /* Fixed: Specific size by user */
    real32_t resize_perc;  /* Fixed: Weight for resizing */
    real32_t margin_after; /* Fixed: Space after particion */
    real32_t natural_size; /* Computed: Natural size of partition */
    real32_t final_size;   /* Computed: Final size after expansion */
    bool_t blocked;        /* Computed: Size can't be changed during resizing part */
    bool_t must_resize;    /* Computed: Marked as must-resize during resizing part */
};

/* Represents a cell dimension (Width or Height) */
struct i_cell_dim_t
{
    /* Fixed attributes. Change required execute compose algorithm */
    /* Computed attributes. Final results of compose algorithm  */
    align_t align;           /* Fixed: Cell alignment within the partition */
    real32_t forced_size;    /* Fixed: Specific size by user */
    real32_t padding_before; /* Fixed: Internal space before content */
    real32_t padding_after;  /* Fixed: Internal space after content */
    real32_t natural_size;   /* Computed: Natural size of dimension */
    real32_t final_size;     /* Computed: Final size after expansion */
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

/* Represents a cell within a layout */
struct _cell_t
{
    ctype_t type;
    bool_t displayed;      /* If FALSE, no space allocation */
    bool_t visible;        /* Only if displayed==TRUE. Shown or hide content, but space allocation is computed */
    bool_t enabled;        /* If FALSE, content is grayed and disable events */
    bool_t tabstop;        /* If TRUE, the cell will be included in the tab-list */
    i_CellDim dim[2];      /* Cell width/height dimensions */
    i_CellContent content; /* Content */
    Layout *parent;        /* Layout to which it belongs */
    uint32_t member_id;    /* DBind struct member linked to the cell */
};

/* GUI composition grid. Can be also de content of a cell (recursive sublayouts) */
struct _layout_t
{
#if defined(LAYOUT_DEBUGGING)
    char_t name[64];
#endif
    Cell *parent;                   /* For sublayouts, cell to which it belongs */
    Panel *panel;                   /* Nearest ancestor panel */
    bool_t is_row_major_tab;        /* TRUE, cells are traversed by rows */
    ArrPt(Cell) *cells;             /* Cells */
    ArrSt(i_LineDim) *lines_dim[2]; /* Columns/Rows dimensions */
    ArrPt(Cell) *cells_dim[2];      /* Cells in row-major/column-major order */
    uint32_t dim_num_elems[2];      /* Total rows, total columns. Dimension 0 (columns) has 'dim_num_elems[0]' (rows) elements */
    real32_t dim_margin_before[2];  /* Left/top border margin */
    real32_t dim_margin_after[2];   /* Right/bottom border margin */
    color_t bgcolor;                /* Background color */
    color_t skcolor;                /* Border color */
    const DBind *stbind;            /* Data binding: Struct linked to layout */
    void *objbind;                  /* Data binding: Struct instance linked to layout */
    Listener *OnObjChange;          /* Data binding: Struct instance change event */
};

DeclSt(i_LineDim);
static real32_t i_EPSILON = 0.0001f;
static real32_t i_MINIMUM_PIXELS_SIZE = 5;

/*---------------------------------------------------------------------------*/

#define i_NUM_COLS(layout) ((layout)->dim_num_elems[1])
#define i_NUM_ROWS(layout) ((layout)->dim_num_elems[0])

/*---------------------------------------------------------------------------*/

static ___INLINE void i_init_celldim(i_CellDim *dim)
{
    cassert_no_null(dim);
    dim->align = ENUM_MAX(align_t);
    dim->forced_size = 0;
    dim->padding_before = 0;
    dim->padding_after = 0;
    dim->natural_size = 0;
    dim->final_size = 0;
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_init_cell(Cell *cell, Layout *layout)
{
    cassert_no_null(cell);
    cell->type = i_ekEMPTY;
    cell->displayed = TRUE;
    cell->visible = TRUE;
    cell->enabled = TRUE;
    cell->tabstop = TRUE;
    i_init_celldim(&cell->dim[0]);
    i_init_celldim(&cell->dim[1]);
    cell->content.empty = NULL;
    cell->parent = layout;
    cell->member_id = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

static void i_remove_cell(Cell *cell)
{
    Layout *layout = NULL;
    cassert_no_null(cell);
    layout = cell->parent;
    cassert_no_null(layout);

    switch (cell->type)
    {
    case i_ekEMPTY:
        break;

    case i_ekCOMPONENT:
    {
        GuiComponent *component = cell->content.component;
        cell->type = i_ekEMPTY;
        obj_release(&cell->content.component, GuiComponent);
        if (layout->panel != NULL)
            _panel_destroy_component(layout->panel, component);
        break;
    }

    case i_ekLAYOUT:
        cell->type = i_ekEMPTY;
        _layout_destroy(&cell->content.layout);
        break;

    default:
        cassert_default(cell->type);
    }
}

/*---------------------------------------------------------------------------*/

static void i_destroy_cell(Cell **cell)
{
    cassert_no_null(cell);
    i_remove_cell(*cell);
    heap_delete(cell, Cell);
}

/*---------------------------------------------------------------------------*/

void _layout_destroy(Layout **layout)
{
    cassert_no_null(layout);
    cassert_no_null(*layout);
    arrst_destroy(&(*layout)->lines_dim[0], NULL, i_LineDim);
    arrst_destroy(&(*layout)->lines_dim[1], NULL, i_LineDim);
    arrpt_destroy(&(*layout)->cells_dim[0], NULL, Cell);
    arrpt_destroy(&(*layout)->cells_dim[1], NULL, Cell);
    arrpt_destroy(&(*layout)->cells, i_destroy_cell, Cell);
    listener_destroy(&(*layout)->OnObjChange);
    heap_delete(layout, Layout);
}

/*---------------------------------------------------------------------------*/

static ArrSt(i_LineDim) *i_create_linedim(const uint32_t num_elems)
{
    ArrSt(i_LineDim) *dimension = arrst_create(i_LineDim);
    real32_t resize = 1 / (real32_t)num_elems;
    real32_t total = 0;

    arrst_new_n0(dimension, num_elems, i_LineDim);
    arrst_foreach(dim, dimension, i_LineDim)
        if (dim_i == dim_total - 1)
            resize = 1 - total;
        dim->displayed = TRUE;
        dim->resize_perc = resize;
        total += resize;
    arrst_end()

    cassert(bmath_absf(total - 1) < i_EPSILON);
    return dimension;
}

/*---------------------------------------------------------------------------*/

static void i_cell_rowcol_order(ArrPt(Cell) *cells, const uint32_t ncols, const uint32_t nrows, ArrPt(Cell) *cells_dim0, ArrPt(Cell) *cells_dim1)
{
    uint32_t ncells = arrpt_size(cells, Cell);
    cassert(ncells == ncols * nrows);
    cassert(arrpt_size(cells_dim0, Cell) == 0);
    cassert(arrpt_size(cells_dim1, Cell) == 0);
    arrpt_insert_n(cells_dim0, 0, ncells, Cell);
    arrpt_insert_n(cells_dim1, 0, ncells, Cell);

    /* Row major order for column-dimensions (dim[0]) */
    {
        uint32_t i, j;
        Cell **cell = arrpt_all(cells, Cell);
        Cell **dim_cell = arrpt_all(cells_dim0, Cell);
        for (i = 0; i < ncols; ++i)
        {
            for (j = 0; j < nrows; ++j, ++dim_cell)
                *dim_cell = *(cell + (j * ncols) + i);
        }
    }

    /* Column major order for row-dimensions (dim[1]) */
    {
        uint32_t i;
        Cell **cell = arrpt_all(cells, Cell);
        Cell **dim_cell = arrpt_all(cells_dim1, Cell);
        for (i = 0; i < ncells; ++i, ++dim_cell, ++cell)
            *dim_cell = *cell;
    }
}

/*---------------------------------------------------------------------------*/

Layout *layout_create(const uint32_t ncols, const uint32_t nrows)
{
    Layout *layout = heap_new0(Layout);
    uint32_t i, ncells = nrows * ncols;
    cassert(ncells > 0);
    layout->cells = arrpt_create(Cell);
    layout->lines_dim[0] = i_create_linedim(ncols);
    layout->lines_dim[1] = i_create_linedim(nrows);
    layout->cells_dim[0] = arrpt_create(Cell);
    layout->cells_dim[1] = arrpt_create(Cell);
    layout->is_row_major_tab = TRUE;
    layout->dim_num_elems[0] = nrows; /* Every column has nrows elems */
    layout->dim_num_elems[1] = ncols; /* Every row has ncols elems */

    for (i = 0; i < ncells; ++i)
    {
        Cell *cell = heap_new(Cell);
        i_init_cell(cell, layout);
        arrpt_append(layout->cells, cell, Cell);
    }

    i_cell_rowcol_order(layout->cells, ncols, nrows, layout->cells_dim[0], layout->cells_dim[1]);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Cell *i_get_cell(Layout *layout, const uint32_t col, const uint32_t row)
{
    uint32_t position = UINT32_MAX;
    cassert_no_null(layout);
    cassert(col < i_NUM_COLS(layout));
    cassert(row < i_NUM_ROWS(layout));
    position = row * i_NUM_COLS(layout) + col;
    return arrpt_get(layout->cells, position, Cell);
}

/*---------------------------------------------------------------------------*/

Cell *layout_cell(Layout *layout, const uint32_t col, const uint32_t row)
{
    return i_get_cell(layout, col, row);
}

/*---------------------------------------------------------------------------*/

GuiControl *layout_control(Layout *layout, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    if (cell->type == i_ekCOMPONENT)
        return cast(cell->content.component, GuiControl);
    else
        return NULL;
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

    if (layout->panel != NULL)
        _panel_attach_component(layout->panel, component);

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
    bool_t displayed = FALSE, visible = FALSE, enabled = FALSE, tabstop = FALSE;
    i_CellDim dim0, dim1;
    i_CellContent content;
    Window *parent_window;
    cassert_no_null(layout);
    cassert_no_null(layout->panel);
    cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    cassert(cell->type == i_ekCOMPONENT);
    cassert(cell->content.component->type == component->type);
    cassert(cell->member_id == UINT32_MAX);
    displayed = cell->displayed;
    visible = cell->visible;
    enabled = cell->enabled;
    tabstop = cell->tabstop;
    dim0 = cell->dim[0];
    dim1 = cell->dim[1];
    content.component = obj_retain(component, GuiComponent);
    i_remove_cell(cell);
    cassert(cell->type == i_ekEMPTY);
    cell->type = i_ekCOMPONENT;
    cell->displayed = displayed;
    cell->visible = visible;
    cell->enabled = enabled;
    cell->tabstop = tabstop;
    cell->dim[0] = dim0;
    cell->dim[1] = dim1;
    cell->content = content;
    _panel_attach_component(layout->panel, cell->content.component);
    _panel_invalidate_layout(layout->panel, layout);
    parent_window = _panel_get_window(layout->panel);
    _component_set_parent_window(component, parent_window);
}

/*---------------------------------------------------------------------------*/

void layout_label(Layout *layout, Label *label, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    align_t align = ekLEFT;
    cell = i_set_component(layout, cast(label, GuiComponent), col, row, align, ekCENTER);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_button(Layout *layout, Button *button, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    align_t halign = ekJUSTIFY;
    align_t valign = ekJUSTIFY;
    uint32_t flags = _button_flags(button);
    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
        halign = ekJUSTIFY;
        valign = ekCENTER;
        break;

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
        halign = ekLEFT;
        valign = ekCENTER;
        break;

    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        halign = ekCENTER;
        valign = ekCENTER;
        break;

    default:
        cassert_default(button_get_type(flags));
    }

    cell = i_set_component(layout, cast(button, GuiComponent), col, row, halign, valign);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_popup(Layout *layout, PopUp *popup, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(popup, GuiComponent), col, row, ekJUSTIFY, ekCENTER);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_edit(Layout *layout, Edit *edit, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    align_t valign = ekCENTER;
    if (_edit_is_multiline(edit) == TRUE)
        valign = ekJUSTIFY;
    cell = i_set_component(layout, cast(edit, GuiComponent), col, row, ekJUSTIFY, valign);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_combo(Layout *layout, Combo *combo, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(combo, GuiComponent), col, row, ekJUSTIFY, ekCENTER);
    cassert_no_null(cell);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_listbox(Layout *layout, ListBox *list, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(list, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_updown(Layout *layout, UpDown *updown, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(updown, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_slider(Layout *layout, Slider *slider, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    align_t halig = ekJUSTIFY;
    align_t valign = ekCENTER;
    if (_slider_is_horizontal(slider) == FALSE)
    {
        halig = ekCENTER;
        valign = ekJUSTIFY;
    }

    cell = i_set_component(layout, cast(slider, GuiComponent), col, row, halig, valign);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_progress(Layout *layout, Progress *progress, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(progress, GuiComponent), col, row, ekJUSTIFY, ekCENTER);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_view(Layout *layout, View *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_textview(Layout *layout, TextView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_webview(Layout *layout, WebView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = NULL;
    cassert_no_null(view);
    cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_imageview(Layout *layout, ImageView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = FALSE;
}

/*---------------------------------------------------------------------------*/

void layout_tableview(Layout *layout, TableView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_splitview(Layout *layout, SplitView *view, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(view, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_no_null(cell);
    cell->tabstop = TRUE;
}

/*---------------------------------------------------------------------------*/

void layout_panel(Layout *layout, Panel *panel, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_set_component(layout, cast(panel, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
    cassert_unref(cell != NULL, cell);
}

/*---------------------------------------------------------------------------*/

void layout_panel_replace(Layout *layout, Panel *panel, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    if (cell->type == i_ekEMPTY)
    {
        i_set_component(layout, cast(panel, GuiComponent), col, row, ekJUSTIFY, ekJUSTIFY);
        cassert(cell->tabstop == TRUE);

        if (layout->panel != NULL)
        {
            Window *parent_window = _panel_get_window(layout->panel);
            _component_set_parent_window(cast(panel, GuiComponent), parent_window);
            layout_update(layout);
        }
    }
    else
    {
        cassert(cell->type == i_ekCOMPONENT);
        cassert(cell->content.component->type == ekGUI_TYPE_PANEL);
        i_change_component(layout, cast(panel, GuiComponent), col, row);
        layout_update(layout);
    }
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

    if (layout->panel != NULL)
        _layout_attach_to_panel(sublayout, layout->panel);
}

/*---------------------------------------------------------------------------*/

Label *layout_get_label(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_label(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Button *layout_get_button(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_button(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

PopUp *layout_get_popup(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_popup(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Edit *layout_get_edit(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_edit(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Combo *layout_get_combo(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_combo(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

ListBox *layout_get_listbox(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_listbox(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

UpDown *layout_get_updown(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_updown(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Slider *layout_get_slider(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_slider(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Progress *layout_get_progress(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_progress(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

View *layout_get_view(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_view(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

TextView *layout_get_textview(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_textview(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

WebView *layout_get_webview(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_webview(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

ImageView *layout_get_imageview(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_imageview(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

TableView *layout_get_tableview(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_tableview(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

SplitView *layout_get_splitview(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_splitview(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Panel *layout_get_panel(Layout *layout, const uint32_t col, const uint32_t row)
{
    return guicontrol_panel(layout_control(layout, col, row));
}

/*---------------------------------------------------------------------------*/

Layout *layout_get_layout(Layout *layout, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    if (cell->type == i_ekLAYOUT)
        return cell->content.layout;
    else
        return NULL;
}

/*---------------------------------------------------------------------------*/

uint32_t layout_ncols(const Layout *layout)
{
    cassert_no_null(layout);
    return i_NUM_COLS(layout);
}

/*---------------------------------------------------------------------------*/

uint32_t layout_nrows(const Layout *layout)
{
    cassert_no_null(layout);
    return i_NUM_ROWS(layout);
}

/*---------------------------------------------------------------------------*/

void layout_insert_col(Layout *layout, const uint32_t col)
{
    uint32_t i, ncols = 0, nrows = 0;
    i_LineDim *dim = NULL;

    cassert_no_null(layout);
    ncols = i_NUM_COLS(layout);
    nrows = i_NUM_ROWS(layout);
    cassert(col <= ncols);

    /* Insert and init (empty) the new cells */
    for (i = 0; i < nrows; ++i)
    {
        uint32_t inspos = ((ncols + 1) * i) + col;
        Cell *new_cell = heap_new(Cell);
        i_init_cell(new_cell, layout);
        arrpt_insert(layout->cells, inspos, new_cell, Cell);
    }

    /* Add a new column dimensions. The margin after this new column will be 0 */
    dim = arrst_insert_n0(layout->lines_dim[0], col, 1, i_LineDim);
    dim->displayed = TRUE;

    /* Every row has ncols elems */
    ncols += 1;
    layout->dim_num_elems[1] = ncols;

    /* Regenerate the cell indices */
    arrpt_clear(layout->cells_dim[0], NULL, Cell);
    arrpt_clear(layout->cells_dim[1], NULL, Cell);
    i_cell_rowcol_order(layout->cells, ncols, nrows, layout->cells_dim[0], layout->cells_dim[1]);
}

/*---------------------------------------------------------------------------*/

void layout_insert_row(Layout *layout, const uint32_t row)
{
    uint32_t inspos = 0;
    uint32_t i, ncols = 0, nrows = 0;
    Cell **new_cells = NULL;
    i_LineDim *dim = NULL;

    cassert_no_null(layout);
    ncols = i_NUM_COLS(layout);
    nrows = i_NUM_ROWS(layout);
    cassert(row <= nrows);

    /* Cells insert position */
    inspos = row * ncols;
    /* Cells array is in row-major order. All row cells are together in memory */
    new_cells = arrpt_insert_n(layout->cells, inspos, ncols, Cell);

    /* Init the new cells (empty) */
    for (i = 0; i < ncols; ++i)
    {
        new_cells[i] = heap_new(Cell);
        i_init_cell(new_cells[i], layout);
    }

    /* Add a new row dimensions. The margin after this new column will be 0 */
    dim = arrst_insert_n0(layout->lines_dim[1], row, 1, i_LineDim);
    dim->displayed = TRUE;

    /* Every row has ncols elems */
    nrows += 1;
    layout->dim_num_elems[0] = nrows;

    /* Regenerate the cell indices */
    arrpt_clear(layout->cells_dim[0], NULL, Cell);
    arrpt_clear(layout->cells_dim[1], NULL, Cell);
    i_cell_rowcol_order(layout->cells, ncols, nrows, layout->cells_dim[0], layout->cells_dim[1]);
}

/*---------------------------------------------------------------------------*/

static void i_recompute_resize_percent(ArrSt(i_LineDim) *dim)
{
    real32_t norm = 0;
    real32_t nedims = 0;
    cassert_no_null(dim);
    cassert(arrst_size(dim, i_LineDim) > 0);
    arrst_foreach(edim, dim, i_LineDim)
        if (edim->resize_perc > i_EPSILON)
        {
            norm += edim->resize_perc;
            nedims += 1;
        }
    arrst_end()

    /* We must normalize */
    if (bmath_absf(1 - norm) >= i_EPSILON)
    {
        if (nedims > 0)
        {
            arrst_foreach(edim, dim, i_LineDim)
                if (edim->resize_perc > i_EPSILON)
                    edim->resize_perc /= norm;
            arrst_end()
        }
        /* All resize to last dim */
        else
        {
            i_LineDim *edim = arrst_last(dim, i_LineDim);
            edim->resize_perc = 1;
        }
    }
}

/*---------------------------------------------------------------------------*/

void layout_remove_col(Layout *layout, const uint32_t col)
{
    uint32_t ncols = 0;
    cassert_no_null(layout);
    ncols = i_NUM_COLS(layout);

    /* 0 cols layout is not supported */
    if (ncols > 1)
    {
        uint32_t i, nrows = i_NUM_ROWS(layout);
        cassert(col < ncols);

        /* Destroy the column cells */
        for (i = 0; i < nrows; ++i)
        {
            uint32_t delrow = nrows - i - 1;
            uint32_t delpos = (ncols * delrow) + col;
            arrpt_delete(layout->cells, delpos, i_destroy_cell, Cell);
        }

        /* Remove the column dimensions */
        arrst_delete(layout->lines_dim[0], col, NULL, i_LineDim);

        /* Remove the last-column margin */
        {
            i_LineDim *lcol = arrst_last(layout->lines_dim[0], i_LineDim);
            lcol->margin_after = 0;
        }

        /* Every row has ncols elems */
        ncols -= 1;
        layout->dim_num_elems[1] = ncols;

        /* Regenerate the cell indices */
        arrpt_clear(layout->cells_dim[0], NULL, Cell);
        arrpt_clear(layout->cells_dim[1], NULL, Cell);
        i_cell_rowcol_order(layout->cells, ncols, nrows, layout->cells_dim[0], layout->cells_dim[1]);

        /* Resize percent */
        i_recompute_resize_percent(layout->lines_dim[0]);
    }
}

/*---------------------------------------------------------------------------*/

void layout_remove_row(Layout *layout, const uint32_t row)
{
    uint32_t nrows = 0;
    cassert_no_null(layout);
    nrows = i_NUM_ROWS(layout);

    /* 0 rows layout is not supported */
    if (nrows > 1)
    {
        uint32_t i, ncols = i_NUM_COLS(layout);
        cassert(row < nrows);

        /* Destroy the row cells */
        for (i = 0; i < ncols; ++i)
        {
            uint32_t delcol = ncols - i - 1;
            uint32_t delpos = (ncols * row) + delcol;
            arrpt_delete(layout->cells, delpos, i_destroy_cell, Cell);
        }

        /* Remove the row dimensions */
        arrst_delete(layout->lines_dim[1], row, NULL, i_LineDim);

        /* Remove the last-row margin */
        {
            i_LineDim *lcol = arrst_last(layout->lines_dim[1], i_LineDim);
            lcol->margin_after = 0;
        }

        /* Every column has nrows elems */
        nrows -= 1;
        layout->dim_num_elems[0] = nrows;

        /* Regenerate the cell indices */
        arrpt_clear(layout->cells_dim[0], NULL, Cell);
        arrpt_clear(layout->cells_dim[1], NULL, Cell);
        i_cell_rowcol_order(layout->cells, ncols, nrows, layout->cells_dim[0], layout->cells_dim[1]);

        /* Resize percent */
        i_recompute_resize_percent(layout->lines_dim[1]);
    }
}

/*---------------------------------------------------------------------------*/

void layout_taborder(Layout *layout, const gui_orient_t order)
{
    cassert_no_null(layout);
    layout->is_row_major_tab = (order == ekGUI_HORIZONTAL) ? TRUE : FALSE;
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
    cassert(width >= 0);
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    cassert_no_null(dim);
    dim->forced_size = width;
}

/*---------------------------------------------------------------------------*/

void layout_vsize(Layout *layout, const uint32_t row, const real32_t height)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert(height >= 0);
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    cassert_no_null(dim);
    dim->forced_size = height;
}

/*---------------------------------------------------------------------------*/

void layout_hmargin(Layout *layout, const uint32_t col, const real32_t margin)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert(col < i_NUM_COLS(layout) - 1);
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    dim->margin_after = margin;
}

/*---------------------------------------------------------------------------*/

void layout_vmargin(Layout *layout, const uint32_t row, const real32_t margin)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert(row < i_NUM_ROWS(layout) - 1);
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    dim->margin_after = margin;
}

/*---------------------------------------------------------------------------*/

static void i_expand1(ArrSt(i_LineDim) *dim, const uint32_t index)
{
    cassert(index < arrst_size(dim, i_LineDim));
    arrst_foreach(edim, dim, i_LineDim)
        if (edim_i == index)
            edim->resize_perc = 1;
        else
            edim->resize_perc = 0;
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_expand2(ArrSt(i_LineDim) *dim, const uint32_t index1, const uint32_t index2, const real32_t exp)
{
    cassert(index1 != index2);
    cassert(index1 < arrst_size(dim, i_LineDim));
    cassert(index2 < arrst_size(dim, i_LineDim));
    cassert(exp <= 1);
    arrst_foreach(edim, dim, i_LineDim)
        if (edim_i == index1)
            edim->resize_perc = exp;
        else if (edim_i == index2)
            edim->resize_perc = 1 - exp;
        else
            edim->resize_perc = 0;
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_expand3(ArrSt(i_LineDim) *dim, const uint32_t index1, const uint32_t index2, const uint32_t index3, const real32_t exp1, const real32_t exp2)
{
    cassert(index1 != index2);
    cassert(index1 != index3);
    cassert(index2 != index3);
    cassert(index1 < arrst_size(dim, i_LineDim));
    cassert(index2 < arrst_size(dim, i_LineDim));
    cassert(index3 < arrst_size(dim, i_LineDim));
    cassert(exp1 + exp2 <= 1);
    arrst_foreach(edim, dim, i_LineDim)
        if (edim_i == index1)
            edim->resize_perc = exp1;
        else if (edim_i == index2)
            edim->resize_perc = exp2;
        else if (edim_i == index3)
            edim->resize_perc = 1 - exp1 - exp2;
        else
            edim->resize_perc = 0;
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_expandn(ArrSt(i_LineDim) *dim, const uint32_t n, const uint32_t *index, const real32_t *exp)
{
    cassert(n > 0);
    cassert_no_null(index);
    cassert_no_null(exp);

#if defined(__ASSERTS__)
    {
        real32_t total = 0;
        uint32_t i = 0;
        for (i = 0; i < n; ++i)
        {
            cassert(index[i] < arrst_size(dim, i_LineDim));
            cassert(exp[i] >= 0);
            total += exp[i];
        }

        cassert(bmath_absf(1 - total) < i_EPSILON);
    }
#endif

    arrst_foreach(edim, dim, i_LineDim)
        edim->resize_perc = 0;
    arrst_end()

    {
        i_LineDim *edim = arrst_all(dim, i_LineDim);
        real32_t total = 0;
        uint32_t i = 0;

        for (i = 0; i < n; ++i)
        {
            if (i == n - 1)
            {
                edim[index[i]].resize_perc = 1 - total;
            }
            else
            {
                edim[index[i]].resize_perc = exp[i];
                total += exp[i];
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void layout_hexpand(Layout *layout, const uint32_t col)
{
    cassert_no_null(layout);
    i_expand1(layout->lines_dim[0], col);
}

/*---------------------------------------------------------------------------*/

void layout_hexpand2(Layout *layout, const uint32_t col1, const uint32_t col2, const real32_t exp)
{
    cassert_no_null(layout);
    i_expand2(layout->lines_dim[0], col1, col2, exp);
}

/*---------------------------------------------------------------------------*/

void layout_hexpand3(Layout *layout, const uint32_t col1, const uint32_t col2, const uint32_t col3, const real32_t exp1, const real32_t exp2)
{
    cassert_no_null(layout);
    i_expand3(layout->lines_dim[0], col1, col2, col3, exp1, exp2);
}

/*---------------------------------------------------------------------------*/

void layout_hexpandn(Layout *layout, const uint32_t n, const uint32_t *index, const real32_t *exp)
{
    cassert_no_null(layout);
    i_expandn(layout->lines_dim[0], n, index, exp);
}

/*---------------------------------------------------------------------------*/

void layout_vexpand(Layout *layout, const uint32_t row)
{
    cassert_no_null(layout);
    i_expand1(layout->lines_dim[1], row);
}

/*---------------------------------------------------------------------------*/

void layout_vexpand2(Layout *layout, const uint32_t row1, const uint32_t row2, const real32_t exp)
{
    cassert_no_null(layout);
    i_expand2(layout->lines_dim[1], row1, row2, exp);
}

/*---------------------------------------------------------------------------*/

void layout_vexpand3(Layout *layout, const uint32_t row1, const uint32_t row2, const uint32_t row3, const real32_t exp1, const real32_t exp2)
{
    cassert_no_null(layout);
    i_expand3(layout->lines_dim[1], row1, row2, row3, exp1, exp2);
}

/*---------------------------------------------------------------------------*/

void layout_vexpandn(Layout *layout, const uint32_t n, const uint32_t *index, const real32_t *exp)
{
    cassert_no_null(layout);
    i_expandn(layout->lines_dim[1], n, index, exp);
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
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    dim->displayed = visible;
}

/*---------------------------------------------------------------------------*/

void layout_show_row(Layout *layout, const uint32_t row, const bool_t visible)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    dim->displayed = visible;
}

/*---------------------------------------------------------------------------*/

void layout_margin(Layout *layout, const real32_t mall)
{
    cassert_no_null(layout);
    layout->dim_margin_before[0] = mall;
    layout->dim_margin_before[1] = mall;
    layout->dim_margin_after[0] = mall;
    layout->dim_margin_after[1] = mall;
}

/*---------------------------------------------------------------------------*/

void layout_margin2(Layout *layout, const real32_t mtb, const real32_t mlr)
{
    cassert_no_null(layout);
    layout->dim_margin_before[0] = mlr;
    layout->dim_margin_before[1] = mtb;
    layout->dim_margin_after[0] = mlr;
    layout->dim_margin_after[1] = mtb;
}

/*---------------------------------------------------------------------------*/

void layout_margin4(Layout *layout, const real32_t mt, const real32_t mr, const real32_t mb, const real32_t ml)
{
    cassert_no_null(layout);
    layout->dim_margin_before[0] = ml;
    layout->dim_margin_before[1] = mt;
    layout->dim_margin_after[0] = mr;
    layout->dim_margin_after[1] = mb;
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
        Window *window = _panel_get_window(layout->panel);
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
    layout->stbind = dbind_from_typename(type, NULL);
    layout->OnObjChange = listener;
    cassert(dbind_type(layout->stbind) == ekDTYPE_STRUCT);
    cassert_unref(dbind_size(layout->stbind) == size, size);
}

/*---------------------------------------------------------------------------*/

static void i_layout_dbind(Layout *layout, const DBind *stbind, void *obj)
{
    arrpt_foreach(cell, layout->cells, Cell)
        if (cell->member_id != UINT32_MAX)
        {
            switch (cell->type)
            {
            case i_ekCOMPONENT:
                if (cell->content.component->type == ekGUI_TYPE_PANEL)
                {
                    Panel *panel = cast(cell->content.component, Panel);
                    ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
                    arrpt_foreach(panel_layout, panel_layouts, Layout)
                        _gbind_update_layout(panel_layout, stbind, cell->member_id, obj);
                    arrpt_end()
                }
                else
                {
                    _gbind_update_control(cell, stbind, cell->member_id, obj);
                }
                break;

            case i_ekLAYOUT:
                _gbind_update_layout(cell->content.layout, stbind, cell->member_id, obj);
                break;

            case i_ekEMPTY:
                break;

            default:
                cassert_default(cell->type);
            }
        }
        else if (cell->type == i_ekLAYOUT)
        {
            i_layout_dbind(cell->content.layout, stbind, obj);
        }
        else if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_PANEL)
        {
            Panel *panel = cast(cell->content.component, Panel);
            ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
            arrpt_foreach(panel_layout, panel_layouts, Layout)
                if (panel_layout->stbind == NULL)
                    i_layout_dbind(panel_layout, stbind, obj);
            arrpt_end()
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void layout_dbind_obj_imp(Layout *layout, void *obj, const char_t *type)
{
    cassert_no_null(layout);
    cassert_unref(str_equ_c(dbind_typename(layout->stbind), type) == TRUE, type);
    layout->objbind = obj;
    i_layout_dbind(layout, layout->stbind, layout->objbind);
}

/*---------------------------------------------------------------------------*/

void *layout_dbind_get_obj_imp(Layout *layout, const char_t *type)
{
    cassert_no_null(layout);
    cassert_unref(str_equ_c(dbind_typename(layout->stbind), type) == TRUE, type);
    return layout->objbind;
}

/*---------------------------------------------------------------------------*/

void layout_dbind_update_imp(Layout *layout, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize)
{
    const DBind *stbind = dbind_from_typename(type, NULL);
    uint32_t member_id = dbind_st_member_id(stbind, mname);
    cassert_unref(dbind_size(stbind) == size, size);
    cassert_unref(dbind_st_offset(stbind, member_id) == moffset, moffset);
#if defined(__ASSERTS__)
    {
        bool_t is_ptr = FALSE;
        const DBind *bind = dbind_from_typename(mtype, &is_ptr);
        const DBind *mbind = dbind_st_member(stbind, member_id);
        cassert(bind == mbind);
        cassert_unref((is_ptr == TRUE && msize == sizeof(void *)) || dbind_size(mbind) == msize, msize);
        unref(is_ptr);
    }
#else
    unref(mtype);
    unref(msize);
#endif
    _layout_dbind_update(layout, member_id);
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_hsize(const Layout *layout, const uint32_t col)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    cassert_no_null(dim);
    return dim->final_size;
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_vsize(const Layout *layout, const uint32_t row)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    cassert_no_null(dim);
    return dim->final_size;
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_hmargin(const Layout *layout, const uint32_t col)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert(col < i_NUM_COLS(layout) - 1);
    dim = arrst_get(layout->lines_dim[0], col, i_LineDim);
    return dim->margin_after;
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_vmargin(const Layout *layout, const uint32_t row)
{
    i_LineDim *dim = NULL;
    cassert_no_null(layout);
    cassert(row < i_NUM_ROWS(layout) - 1);
    dim = arrst_get(layout->lines_dim[1], row, i_LineDim);
    return dim->margin_after;
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_margin_left(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->dim_margin_before[0];
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_margin_right(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->dim_margin_after[0];
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_margin_top(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->dim_margin_before[1];
}

/*---------------------------------------------------------------------------*/

real32_t layout_get_margin_bottom(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->dim_margin_after[1];
}

/*---------------------------------------------------------------------------*/

void layout_remove_cell(Layout *layout, const uint32_t col, const uint32_t row)
{
    Cell *cell = i_get_cell(layout, col, row);
    cassert_no_null(cell);
    i_remove_cell(cell);
    i_init_cell(cell, layout);
}

/*---------------------------------------------------------------------------*/

void layout_name(Layout *layout, const char_t *name)
{
#if defined(LAYOUT_DEBUGGING)
    cassert_no_null(layout);
    str_copy_c(layout->name, sizeof(layout->name), name);
#else
    unref(layout);
    unref(name);
    cassert(FALSE);
#endif
}

/*---------------------------------------------------------------------------*/

void _layout_attach_to_panel(Layout *layout, Panel *panel)
{
    cassert_no_null(layout);
    cassert(layout->panel == NULL || layout->panel == panel);
    layout->panel = panel;
    arrpt_foreach(cell, layout->cells, Cell)
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
        default:
            cassert_default(cell->type);
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

Panel *_layout_panel(const Layout *layout)
{
    cassert_no_null(layout);
    return layout->panel;
}

/*---------------------------------------------------------------------------*/

Layout *_layout_search_component(const Layout *layout, const GuiComponent *component, Cell **in_cell, const bool_t in_subpanels)
{
    Layout *find_layout = NULL;
    cassert_no_null(layout);
    cassert_no_null(layout->panel);

    arrpt_foreach(cell, layout->cells, Cell)
        /* In Layout destroy process, can be found NULL-Cells */
        if (cell != NULL)
        {
            if (cell->type == i_ekCOMPONENT)
            {
                cassert_no_null(cell->content.component);
                if (cell->content.component == component)
                {
                    find_layout = (Layout *)layout;
                    ptr_assign(in_cell, cell);
                    break;
                }

                if (in_subpanels == TRUE)
                {
                    if (cell->content.component->type == ekGUI_TYPE_PANEL)
                    {
                        Panel *panel = cast(cell->content.component, Panel);
                        find_layout = _panel_active_layout(panel);
                        if (find_layout != NULL)
                            break;
                    }
                }
            }
            else if (cell->type == i_ekLAYOUT)
            {
                find_layout = _layout_search_component(cell->content.layout, component, in_cell, in_subpanels);
                if (find_layout != NULL)
                    break;
            }
        }
    arrpt_end()

    return find_layout;
}

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
        arrpt_foreach(cell, layout->cells, Cell)
            if (cell->type == i_ekLAYOUT)
            {
                bool_t exists = _layout_search_layout(cell->content.layout, sublayout);
                if (exists == TRUE)
                    return exists;
            }
        arrpt_end()

        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

void _layout_components(const Layout *layout, ArrPt(GuiComponent) *components)
{
    cassert_no_null(layout);
    arrpt_foreach(cell, layout->cells, Cell)
        switch (cell->type)
        {
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
        default:
            cassert_default(cell->type);
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

static void i_cell_natural(Cell *cell, const uint32_t di)
{
    cassert_no_null(cell);
    switch (cell->type)
    {
    case i_ekCOMPONENT:
        _component_natural(cell->content.component, di, &cell->dim[0].natural_size, &cell->dim[1].natural_size);
        break;
    case i_ekLAYOUT:
        _layout_natural(cell->content.layout, di, &cell->dim[0].natural_size, &cell->dim[1].natural_size);
        break;
    case i_ekEMPTY:
        cell->dim[di].natural_size = 0;
        break;
    default:
        cassert_default(cell->type);
    }
}

/*---------------------------------------------------------------------------*/

static real32_t i_natural_with_padding(const Cell *cell, const uint32_t di)
{
    cassert_no_null(cell);
    return cell->dim[di].natural_size + cell->dim[di].padding_after + cell->dim[di].padding_before;
}

/*---------------------------------------------------------------------------*/

static void i_cell_expand(Cell *cell, const uint32_t di, const real32_t required_size)
{
    real32_t required_net_size = required_size;
    cassert_no_null(cell);

    /* Cell expansion doesn't include padding */
    required_net_size -= cell->dim[di].padding_after;
    required_net_size -= cell->dim[di].padding_before;

    if (required_net_size < 0)
        required_net_size = 0;

    switch (cell->type)
    {
    case i_ekCOMPONENT:
        _component_expand(cell->content.component, di, cell->dim[di].natural_size, required_net_size, &cell->dim[di].final_size);
        break;
    case i_ekLAYOUT:
        _layout_expand(cell->content.layout, di, cell->dim[di].natural_size, required_net_size, &cell->dim[di].final_size);
        break;
    case i_ekEMPTY:
        cell->dim[di].final_size = required_net_size;
        break;
    default:
        cassert_default(cell->type);
    }
}

/*---------------------------------------------------------------------------*/
/*
 * Natural sizing has two steps:
 * - Compute the natural sizing of cells
 * - Compute internal cell expansion, that will occur when:
 *   - The line has a forced size
 *   - The cell has a forced size
 *   - The cell has JUSTIFY alignment
 */
static void i_line_natural(i_LineDim *dim, const uint32_t di, Cell **cell, const uint32_t n_cells)
{
    uint32_t i = 0;
    bool_t recompute = TRUE;
    real32_t final_natural_size = 0;

    cassert_no_null(dim);
    cassert_no_null(cell);
    cassert(di <= 1);

    dim->natural_size = 0;
    dim->final_size = 0;

    /* Compose the line (column or row) and compute its size (width or height) */
    for (i = 0; i < n_cells; ++i)
    {
        real32_t csize = 0;
        if (cell[i]->displayed == TRUE)
            i_cell_natural(cell[i], di);
        else
            cell[i]->dim[di].natural_size = 0;

        csize = i_natural_with_padding(cell[i], di);
        if (csize > dim->natural_size)
            dim->natural_size = csize;
    }

    final_natural_size = dim->natural_size;

    if (dim->forced_size > 0)
        final_natural_size = dim->forced_size;

    /* Need to recompute the natural size, until the constrainst are met  */
    while (recompute == TRUE)
    {
        for (i = 0; i < n_cells; ++i)
        {
            real32_t expand_required_size = -1;
            if (cell[i]->displayed == TRUE)
            {
                real32_t csize = i_natural_with_padding(cell[i], di);

                /* We have user-defined size in column/row */
                if (expand_required_size < 0 && dim->forced_size < csize)
                    expand_required_size = dim->forced_size;

                /* We have user-defined size in cell width/height */
                if (expand_required_size < 0 && cell[i]->dim[di].forced_size > 0)
                    expand_required_size = cell[i]->dim[di].forced_size;

                /* We cell dimension must be expanded */
                if (expand_required_size < 0 && cell[i]->dim[di].align == ekJUSTIFY)
                    expand_required_size = final_natural_size;
            }

            if (expand_required_size > 0)
            {
                i_cell_expand(cell[i], di, expand_required_size);
                /* Its not possible for this cell met the constraint. Move the constraint and abort */
                if (cell[i]->dim[di].final_size > final_natural_size)
                {
                    final_natural_size = cell[i]->dim[di].final_size;
                    break;
                }
            }
            else
            {
                cell[i]->dim[di].final_size = cell[i]->dim[di].natural_size;
            }
        }

        recompute = i < n_cells;
    }

    /* The natural size have been successfully computed */
    dim->natural_size = final_natural_size;
    dim->final_size = final_natural_size;
    for (i = 0; i < n_cells; ++i)
        cell[i]->dim[di].natural_size = cell[i]->dim[di].final_size;
}

/*---------------------------------------------------------------------------*/

static void i_cache_natural_size(Layout *layout, const uint32_t di)
{
    Cell **cells = NULL;
    uint32_t n = 0;
    cassert_no_null(layout);
    cassert(di <= 1);

    cells = arrpt_all(layout->cells_dim[di], Cell);
    n = layout->dim_num_elems[di];

    arrst_foreach(dim, layout->lines_dim[di], i_LineDim)
        cassert((dim_i == dim_total - 1 && dim->margin_after == 0) || dim_i < dim_total - 1);
        if (dim->displayed == TRUE)
        {
            uint32_t i = 0;
            dim->natural_size = dim->final_size;
            for (i = 0; i < n; ++i)
            {
                if (cells[i]->displayed == TRUE)
                {
                    cells[i]->dim[di].natural_size = cells[i]->dim[di].final_size;
                    if (cells[i]->type == i_ekLAYOUT)
                        i_cache_natural_size(cells[i]->content.layout, di);
                }
            }
        }
        cells += layout->dim_num_elems[di];
    arrst_end()
}

/*---------------------------------------------------------------------------*/

void _layout_natural(Layout *layout, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    real32_t size = 0;
    Cell **cells = NULL;
    cassert_no_null(layout);
    cassert(di <= 1);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    cells = arrpt_all(layout->cells_dim[di], Cell);
    size += layout->dim_margin_before[di];

    arrst_foreach(dim, layout->lines_dim[di], i_LineDim)
        cassert((dim_i == dim_total - 1 && dim->margin_after == 0) || dim_i < dim_total - 1);
        if (dim->displayed == TRUE)
        {
            i_line_natural(dim, di, cells, layout->dim_num_elems[di]);
            size += dim->final_size;
            size += dim->margin_after;
        }
        else
        {
            dim->natural_size = 0;
            dim->final_size = 0;
        }
        cells += layout->dim_num_elems[di];
    arrst_end()

    size += layout->dim_margin_after[di];

    i_cache_natural_size(layout, di);

    if (di == 0)
        *dim0 = size;
    else
        *dim1 = size;
}

/*---------------------------------------------------------------------------*/

static void i_line_expand(const uint32_t di, Cell **line_cells, const uint32_t ncells, const real32_t required_size, real32_t *final_size)
{
    uint32_t i = 0;
    cassert_no_null(final_size);
    *final_size = required_size;
    for (i = 0; i < ncells; ++i)
    {
        if (line_cells[i]->displayed == TRUE)
        {
            if (line_cells[i]->dim[di].align == ekJUSTIFY)
                i_cell_expand(line_cells[i], di, required_size);

            if (line_cells[i]->dim[di].final_size > *final_size)
                *final_size = line_cells[i]->dim[di].final_size;
        }
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_dimension_resize(ArrSt(i_LineDim) *dim, const uint32_t di, Cell **cells, const uint32_t dim_num_elems, const real32_t current_size, const real32_t required_size, real32_t *final_size)
{
    real32_t diff = bmath_roundf(required_size - current_size);
    real32_t norm = 0;
    uint32_t last_edim = UINT32_MAX;
    bool_t can_resize = FALSE;
    cassert_no_null(final_size);

    /* There are space for grow/shrink */
    if (bmath_absf(diff) > i_EPSILON)
        can_resize = TRUE;

    /* Estimate which partions can be resized */
    if (can_resize == TRUE)
    {
        arrst_foreach(edim, dim, i_LineDim)
            cassert(edim->resize_perc >= 0);
            if (edim->blocked == FALSE && edim->displayed == TRUE && edim->resize_perc > i_EPSILON)
            {
                edim->must_resize = TRUE;
                norm += edim->resize_perc;
                last_edim = edim_i;
            }
            else
            {
                edim->must_resize = FALSE;
            }
        arrst_end()
        can_resize = bmath_absf(norm) > i_EPSILON;
    }

    /* At least, one partition can be resized */
    if (can_resize == TRUE)
    {
        real32_t diff_apply = 0;

        *final_size = current_size;

        arrst_foreach(edim, dim, i_LineDim)
            cassert(edim->resize_perc >= 0);
            if (edim->must_resize == TRUE)
            {
                real32_t increment = 0;
                real32_t erequired_size, efinal_size = 0;

                /* Perfect adjust. Avoid a new resize for a few pixels because the round of increment */
                if (edim_i == last_edim)
                    increment = diff - diff_apply;
                else
                    increment = bmath_roundf(diff * (edim->resize_perc / norm));

                if (diff_apply + increment > bmath_absf(diff))
                    increment = diff - diff_apply;

                /* Any partition can be narrower than */
                if (bmath_absf(edim->final_size + increment) > i_MINIMUM_PIXELS_SIZE)
                {
                    erequired_size = edim->final_size + increment;
                }
                /* The cell has reached a minimum size. Blocked it */
                else
                {
                    erequired_size = i_MINIMUM_PIXELS_SIZE;
                    edim->blocked = TRUE;
                }

                /* We try to expand all cells in the partition */
                i_line_expand(di, cells + (edim_i * dim_num_elems), dim_num_elems, erequired_size, &efinal_size);

                diff_apply += efinal_size - edim->final_size;
                *final_size += efinal_size - edim->final_size;
                edim->final_size = efinal_size;
            }
        arrst_end()
    }
    else
    {
        /* No resize can be done, restrictions cannot be met */
        *final_size = current_size;
    }

    /* If TRUE, another resize iteration can be carried out */
    return can_resize;
}

/*---------------------------------------------------------------------------*/

void _layout_expand(Layout *layout, const uint32_t di, const real32_t natural_size, const real32_t required_size, real32_t *final_size)
{
    bool_t recompute = TRUE;
    real32_t current_size = natural_size;
    Cell **cells = arrpt_all(layout->cells_dim[di], Cell);
    cassert_no_null(layout);
    cassert_no_null(final_size);
    cells = arrpt_all(layout->cells_dim[di], Cell);

    /* Unblock all partitions */
    arrst_foreach(dim, layout->lines_dim[di], i_LineDim)
        dim->blocked = FALSE;
        dim->final_size = dim->natural_size;
    arrst_end()

    while (recompute == TRUE)
    {
        real32_t iter_final_size = 0;
        recompute = i_dimension_resize(layout->lines_dim[di], di, cells, layout->dim_num_elems[di], current_size, required_size, &iter_final_size);

        if (recompute == TRUE)
        {
            /* The current iteration has met the constraints */
            if (bmath_absf(current_size - iter_final_size) < i_EPSILON)
                recompute = FALSE;
        }

        if (recompute == TRUE)
            current_size = iter_final_size;
        else
            *final_size = iter_final_size;
    }

    /*
     * Finally, we have the final size of each partition. However, there may be cells with ekJUSTIFY
     * that are narrower than the final dimension, as they would have allowed for a smaller resizing.
     * Here, we force these cells to expand to the final partition size.
     */
    arrst_foreach(dim, layout->lines_dim[di], i_LineDim)
        if (dim->displayed == TRUE)
        {
            uint32_t i, dim_num_elems = layout->dim_num_elems[di];
            Cell **line_cells = cells + (dim_i * dim_num_elems);
            for (i = 0; i < dim_num_elems; ++i)
            {
                if (line_cells[i]->dim[di].align == ekJUSTIFY)
                    i_cell_expand(line_cells[i], di, dim->final_size);
            }
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

/*
 * _layout_compose() is a constraint satisfaction algorithm that recursively calculates
 * the positions and sizes (frames) of the widgets that make up a window. It consists
 * of two phases: _layout_natural() and _layout_expand(). The first calculates the initial
 * (natural) size of the window, and the second expands or contracts it if a size has been
 * forced from outside, for example, when the window is resized.
 * It acts first on the X coordinate (width) and then on the Y coordinate (height).
 * This is because the height of a widget can be conditioned by its width, for example, in
 * multi-line labels.
 *
 * Important: _layout_compose() is very fast because it only manipulates numbers that
 * represent sizes and does not actually size any widget (it does not call native APIs).
 * This will be done using _layout_locate() once _layout_compose() has completed.
 */

void _layout_compose(Layout *layout, const S2Df *required_size, S2Df *final_size)
{
    real32_t natural_width = 0, natural_height = 0;
    cassert_no_null(layout);
    cassert_no_null(final_size);
    _layout_natural(layout, 0, &natural_width, &natural_height);
    _layout_expand(layout, 0, natural_width, required_size != NULL ? required_size->width : natural_width, &final_size->width);
    cassert(natural_height == 0);
    _layout_natural(layout, 1, &natural_width, &natural_height);
    _layout_expand(layout, 1, natural_height, required_size != NULL ? required_size->height : natural_height, &final_size->height);
}

/*---------------------------------------------------------------------------*/

static real32_t i_dimension_size(const ArrSt(i_LineDim) *dim, const real32_t margin_before, const real32_t margin_after)
{
    real32_t size = margin_before;
    arrst_foreach_const(edim, dim, i_LineDim)
        cassert((edim_i == edim_total - 1 && edim->margin_after == 0) || edim_i < edim_total - 1);
        size += edim->final_size;
        size += edim->margin_after;
    arrst_end()
    size += margin_after;
    return size;
}

/*---------------------------------------------------------------------------*/

static void i_layout_locate(Layout *layout, const V2Df *origin, FPtr_gctx_set_area func_area, void *ospanel)
{
    uint32_t i = 0, j = 0, ncols = 0, nrows = 0;
    const i_LineDim *cols = NULL, *rows = NULL;
    const Cell **cells = NULL;
    V2Df lorigin = *origin;
    real32_t xorigin = lorigin.x;

    cassert_no_null(layout);
    if (layout->bgcolor != 0 || layout->skcolor != 0)
    {
        real32_t width = i_dimension_size(layout->lines_dim[0], layout->dim_margin_before[0], layout->dim_margin_after[0]);
        real32_t height = i_dimension_size(layout->lines_dim[1], layout->dim_margin_before[1], layout->dim_margin_after[1]);
        func_area(ospanel, layout, layout->bgcolor, layout->skcolor, lorigin.x, lorigin.y, width, height);
    }

    ncols = i_NUM_COLS(layout);
    nrows = i_NUM_ROWS(layout);
    cols = arrst_all(layout->lines_dim[0], i_LineDim);
    rows = arrst_all(layout->lines_dim[1], i_LineDim);
    cells = arrpt_all_const(layout->cells, Cell);

    lorigin.y += layout->dim_margin_before[1];

    for (i = 0; i < nrows; ++i)
    {
        lorigin.x = xorigin;
        lorigin.x += layout->dim_margin_before[0];

        for (j = 0; j < ncols; ++j)
        {
            uint32_t p = i * ncols + j;
            const Cell *cell = cells[p];

            if (cell->displayed == TRUE)
            {
                V2Df cell_origin;
                S2Df cell_size;

                cell_origin.x = lorigin.x + cell->dim[0].padding_after;
                cell_origin.y = lorigin.y + cell->dim[1].padding_after;
                cell_size.width = cell->dim[0].final_size;
                cell_size.height = cell->dim[1].final_size;

                switch (cell->dim[0].align)
                {
                case ekLEFT:
                case ekJUSTIFY:
                    break;

                case ekRIGHT:
                {
                    real32_t cell_diff = cols[j].final_size - cell->dim[0].final_size - cell->dim[0].padding_before - cell->dim[0].padding_after;
                    cell_origin.x += cell_diff;
                    break;
                }

                case ekCENTER:
                {
                    real32_t cell_diff = cols[j].final_size - cell->dim[0].final_size - cell->dim[0].padding_before - cell->dim[0].padding_after;
                    cell_origin.x += bmath_floorf(.5f * cell_diff);
                    break;
                }

                default:
                    break;
                }

                switch (cell->dim[1].align)
                {
                case ekTOP:
                case ekJUSTIFY:
                    break;

                case ekBOTTOM:
                {
                    real32_t cell_diff = rows[i].final_size - cell->dim[1].final_size - cell->dim[1].padding_before - cell->dim[1].padding_after;
                    cell_origin.y += cell_diff;
                    break;
                }

                case ekCENTER:
                {
                    real32_t cell_diff = rows[i].final_size - cell->dim[1].final_size - cell->dim[1].padding_before - cell->dim[1].padding_after;
                    cell_origin.y += bmath_floorf(.5f * cell_diff);
                    break;
                }

                default:
                    break;
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
                default:
                    cassert_default(cell->type);
                }
            }

            lorigin.x += cols[j].final_size;
            lorigin.x += cols[j].margin_after;
        }

        lorigin.y += rows[i].final_size;
        lorigin.y += rows[i].margin_after;
    }
}

/*---------------------------------------------------------------------------*/

static void i_layout_visible(Layout *layout, const bool_t parent_visible)
{
    Cell **cells = NULL;
    cassert_no_null(layout);

    cells = arrpt_all(layout->cells, Cell);
    arrst_foreach(row, layout->lines_dim[1], i_LineDim)
        arrst_foreach(col, layout->lines_dim[0], i_LineDim)
            Cell *cell = *cells;
            bool_t visible = row->displayed && col->displayed & cell->visible && cell->displayed && parent_visible;
            switch (cell->type)
            {
            case i_ekCOMPONENT:
                _component_visible(cell->content.component, visible);
                break;
            case i_ekLAYOUT:
                i_layout_visible(cell->content.layout, visible);
                break;
            case i_ekEMPTY:
                break;
            default:
                cassert_default(cell->type);
            }
            cells += 1;
        arrst_end()
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_layout_enabled(Layout *layout, const bool_t parent_enabled)
{
    cassert_no_null(layout);
    arrpt_foreach(cell, layout->cells, Cell)
        bool_t enabled = cell->enabled && parent_enabled;
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
        default:
            cassert_default(cell->type);
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void _layout_locate(Layout *layout)
{
    void *ospanel = NULL;
    FPtr_gctx_set_area func_area = NULL;
    cassert_no_null(layout);

    if (layout->panel != NULL)
    {
        ospanel = cast(layout->panel, GuiComponent)->ositem;
        func_area = cast(layout->panel, GuiComponent)->context->func_panel_area;
        func_area(ospanel, NULL, 0, 0, 0, 0, 0, 0);
    }

    i_layout_locate(layout, &kV2D_ZEROf, func_area, ospanel);
    i_layout_visible(layout, TRUE);
    i_layout_enabled(layout, TRUE);

    if (layout->panel != NULL)
        cast(layout->panel, GuiComponent)->context->func_panel_set_need_display(ospanel);
}

/*---------------------------------------------------------------------------*/

static void i_cell_taborder(const i_LineDim *col, const i_LineDim *row, const Cell *cell, Window *window)
{
    cassert_no_null(col);
    cassert_no_null(row);
    cassert_no_null(cell);
    if (col->displayed == TRUE && row->displayed == TRUE && cell->displayed == TRUE && cell->tabstop == TRUE)
    {
        switch (cell->type)
        {
        case i_ekLAYOUT:
            _layout_taborder(cell->content.layout, window);
            break;
        case i_ekCOMPONENT:
            _component_taborder(cell->content.component, window);
            break;
        case i_ekEMPTY:
            break;
        default:
            cassert_default(cell->type);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _layout_taborder(const Layout *layout, Window *window)
{
    const Cell **cells = NULL;
    const i_LineDim *cols = NULL;
    const i_LineDim *rows = NULL;
    uint32_t i = 0, j = 0, ncols = 0, nrows = 0;
    cassert_no_null(layout);
    cells = arrpt_all_const(layout->cells, Cell);
    cols = arrst_all_const(layout->lines_dim[0], i_LineDim);
    rows = arrst_all_const(layout->lines_dim[1], i_LineDim);
    ncols = i_NUM_COLS(layout);
    nrows = i_NUM_ROWS(layout);
    if (layout->is_row_major_tab == TRUE)
    {
        for (i = 0; i < nrows; ++i)
        {
            for (j = 0; j < ncols; ++j)
                i_cell_taborder(cols + j, rows + i, cells[i * ncols + j], window);
        }
    }
    else
    {
        for (i = 0; i < ncols; ++i)
        {
            for (j = 0; j < nrows; ++j)
                i_cell_taborder(cols + i, rows + j, cells[j * ncols + i], window);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_dbind_update(Layout *layout, const DBind *stbind, const uint32_t member_id, const void *obj)
{
    arrpt_foreach(cell, layout->cells, Cell)
        bool_t update = FALSE;

        /* Update a single object-field (dbind != NULL) */
        if (member_id != UINT32_MAX && cell->member_id == member_id)
            update = TRUE;

        /* Update the entire object (member_id == UINT32_MAX) */
        else if (member_id == UINT32_MAX && cell->member_id != UINT32_MAX)
            update = TRUE;

        if (update)
        {
            switch (cell->type)
            {
            case i_ekCOMPONENT:
                _gbind_update_control(cell, stbind, cell->member_id, obj);
                break;
            case i_ekLAYOUT:
                _gbind_update_layout(cell->content.layout, stbind, cell->member_id, obj);
                break;
            case i_ekEMPTY:
                break;
            default:
                cassert_default(cell->type);
            }
        }
        else if (cell->type == i_ekLAYOUT)
        {
            const DBind *lstbind = cell->content.layout->stbind;
            if (lstbind != NULL)
            {
                const void *lobjbind = cell->content.layout->objbind;
                i_dbind_update(cell->content.layout, lstbind, member_id, lobjbind);
            }
            else
            {
                i_dbind_update(cell->content.layout, stbind, member_id, obj);
            }
        }
        else if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_PANEL)
        {
            Panel *panel = cast(cell->content.component, Panel);
            ArrPt(Layout) *panel_layouts = _panel_layouts(panel);
            arrpt_foreach(panel_layout, panel_layouts, Layout)
                const DBind *lstbind = panel_layout->stbind;
                if (lstbind != NULL)
                {
                    const void *lobjbind = panel_layout->objbind;
                    i_dbind_update(panel_layout, lstbind, member_id, lobjbind);
                }
                else
                {
                    i_dbind_update(panel_layout, stbind, member_id, obj);
                }
            arrpt_end()
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void _layout_dbind_update(Layout *layout, const uint32_t member_id)
{
    cassert_no_null(layout);
    cassert_no_null(layout->stbind);
    cassert_no_null(layout->objbind);
    i_dbind_update(layout, layout->stbind, member_id, layout->objbind);
}

/*---------------------------------------------------------------------------*/

ArrPt(Cell) *_layout_cells(Layout *layout)
{
    cassert_no_null(layout);
    return layout->cells;
}

/*---------------------------------------------------------------------------*/

void _layout_dbind_notif_obj(Layout *layout, void **obj, const char_t **obj_type, uint16_t *obj_size, Listener **listener)
{
    cassert_no_null(layout);
    cassert_no_null(obj);
    cassert_no_null(obj_type);
    cassert_no_null(obj_size);
    cassert_no_null(listener);
    *obj = layout->objbind;
    *obj_type = dbind_typename(layout->stbind);
    *obj_size = dbind_size(layout->stbind);
    *listener = layout->OnObjChange;
}

/*---------------------------------------------------------------------------*/

bool_t cell_empty(Cell *cell)
{
    cassert_no_null(cell);
    return (bool_t)(cell->type == i_ekEMPTY);
}

/*---------------------------------------------------------------------------*/

GuiControl *cell_control(Cell *cell)
{
    cassert_no_null(cell);
    if (cell->type == i_ekCOMPONENT)
        return cast(cell->content.component, GuiControl);
    else
        return NULL;
}

/*---------------------------------------------------------------------------*/

Label *cell_label(Cell *cell)
{
    return guicontrol_label(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Button *cell_button(Cell *cell)
{
    return guicontrol_button(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

PopUp *cell_popup(Cell *cell)
{
    return guicontrol_popup(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Edit *cell_edit(Cell *cell)
{
    return guicontrol_edit(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Combo *cell_combo(Cell *cell)
{
    return guicontrol_combo(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

ListBox *cell_listbox(Cell *cell)
{
    return guicontrol_listbox(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

UpDown *cell_updown(Cell *cell)
{
    return guicontrol_updown(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Slider *cell_slider(Cell *cell)
{
    return guicontrol_slider(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Progress *cell_progress(Cell *cell)
{
    return guicontrol_progress(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

View *cell_view(Cell *cell)
{
    return guicontrol_view(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

TextView *cell_textview(Cell *cell)
{
    return guicontrol_textview(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

WebView *cell_webview(Cell *cell)
{
    return guicontrol_webview(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

ImageView *cell_imageview(Cell *cell)
{
    return guicontrol_imageview(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

TableView *cell_tableview(Cell *cell)
{
    return guicontrol_tableview(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

SplitView *cell_splitview(Cell *cell)
{
    return guicontrol_splitview(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Panel *cell_panel(Cell *cell)
{
    return guicontrol_panel(cell_control(cell));
}

/*---------------------------------------------------------------------------*/

Layout *cell_layout(Cell *cell)
{
    cassert_no_null(cell);
    if (cell->type == i_ekLAYOUT)
        return cell->content.layout;
    else
        return NULL;
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
        default:
            cassert_default(cell->type);
        }
    }
}

/*---------------------------------------------------------------------------*/

void cell_visible(Cell *cell, const bool_t visible)
{
    cassert_no_null(cell);
    if (cell->visible != visible)
    {
        cell->visible = visible;
        switch (cell->type)
        {
        case i_ekCOMPONENT:
            _component_visible(cell->content.component, visible);
            break;
        case i_ekLAYOUT:
            i_layout_visible(cell->content.layout, visible);
            break;
        case i_ekEMPTY:
            break;
        default:
            cassert_default(cell->type);
        }
    }
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

void cell_force_size(Cell *cell, const real32_t width, const real32_t height)
{
    cassert_no_null(cell);
    cell->dim[0].forced_size = width;
    cell->dim[1].forced_size = height;
}

/*---------------------------------------------------------------------------*/

real32_t cell_get_hsize(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->dim[0].final_size;
}

/*---------------------------------------------------------------------------*/

real32_t cell_get_vsize(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->dim[1].final_size;
}

/*---------------------------------------------------------------------------*/

align_t cell_get_halign(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->dim[0].align;
}

/*---------------------------------------------------------------------------*/

align_t cell_get_valign(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->dim[1].align;
}

/*---------------------------------------------------------------------------*/

static void i_set_dbind(Cell *cell, const DBind *stbind, const uint32_t member_id)
{
    const DBind *bind = dbind_st_member(stbind, member_id);
    dtype_t dtype = dbind_type(bind);
    cassert(cell->member_id == UINT32_MAX);
    cassert(member_id != UINT32_MAX);
    cell->member_id = member_id;
    if (cell->type == i_ekCOMPONENT)
    {
        switch (cell->content.component->type)
        {
        case ekGUI_TYPE_POPUP:
        {
            PopUp *popup = cast(cell->content.component, PopUp);
            if (dtype == ekDTYPE_ENUM && _popup_size(popup) == 0)
            {
                uint32_t i, n = dbind_enum_count(bind);
                for (i = 0; i < n; ++i)
                {
                    const char_t *alias = dbind_enum_alias(bind, i);
                    _popup_add_enum_item(popup, alias);
                }

                _popup_list_height(popup, n < 15 ? n : 15);
            }

            break;
        }

        case ekGUI_TYPE_CUSTOMVIEW:
        {
            View *view = cast(cell->content.component, View);
            if (str_equ_c(_view_subtype(view), "ListBox") == TRUE)
            {
                ListBox *listbox = cast(cell->content.component, ListBox);
                if (dtype == ekDTYPE_ENUM && _listbox_count(listbox) == 0)
                {
                    uint32_t i, n = dbind_enum_count(bind);
                    for (i = 0; i < n; ++i)
                    {
                        const char_t *alias = dbind_enum_alias(bind, i);
                        _listbox_add_enum_item(listbox, alias);
                    }
                }
            }

            break;
        }

        case ekGUI_TYPE_EDITBOX:
            if (dbind_is_number_type(dtype) == TRUE)
            {
                Edit *edit = cast(cell->content.component, Edit);
                edit_autoselect(edit, TRUE);
            }
            break;

        case ekGUI_TYPE_BUTTON:
        case ekGUI_TYPE_COMBOBOX:
        case ekGUI_TYPE_SLIDER:
        case ekGUI_TYPE_UPDOWN:
        case ekGUI_TYPE_PROGRESS:
            break;

        case ekGUI_TYPE_TEXTVIEW:
        case ekGUI_TYPE_WEBVIEW:
        case ekGUI_TYPE_TREEVIEW:
        case ekGUI_TYPE_BOXVIEW:
        case ekGUI_TYPE_SPLITVIEW:
        case ekGUI_TYPE_PANEL:
        case ekGUI_TYPE_LINE:
        case ekGUI_TYPE_HEADER:
        case ekGUI_TYPE_WINDOW:
        case ekGUI_TYPE_TOOLBAR:
        default:
            cassert_default(cell->content.component->type);
        }
    }
    else if (cell->type == i_ekLAYOUT && dbind_is_basic_type(dtype) == TRUE)
    {
        arrpt_foreach(lcell, cell->content.layout->cells, Cell)
            i_set_dbind(lcell, stbind, member_id);
        arrpt_end()
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
    const DBind *stbind = dbind_from_typename(type, NULL);
    uint32_t member_id = dbind_st_member_id(stbind, mname);
    cassert_no_null(cell);
    cassert(member_id != UINT32_MAX);
    cassert_unref(dbind_size(stbind) == size, size);
    cassert_unref(dbind_st_offset(stbind, member_id) == moffset, moffset);

#if defined(__ASSERTS__)
    {
        bool_t is_ptr = FALSE;
        const DBind *bind = dbind_from_typename(mtype, &is_ptr);
        const DBind *mbind = dbind_st_member(stbind, member_id);
        cassert(bind == mbind);
        cassert_unref((is_ptr == TRUE && msize == sizeof(void *)) || dbind_size(mbind) == msize, msize);
        unref(is_ptr);
    }
#else
    unref(mtype);
    unref(msize);
#endif

    i_set_dbind(cell, stbind, member_id);
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
    arrpt_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_BUTTON && _button_is_radio(cast_const(cell->content.component, Button)))
            _button_radio_state(cast(cell->content.component, Button), (cell == on_cell) ? ekGUI_ON : ekGUI_OFF);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void _cell_set_radio_index(Cell *on_cell, const uint32_t index)
{
    uint32_t i = 0;
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrpt_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_BUTTON && _button_is_radio(cast_const(cell->content.component, Button)))
        {
            _button_radio_state(cast(cell->content.component, Button), (i == index) ? ekGUI_ON : ekGUI_OFF);
            i += 1;
        }
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

uint32_t _cell_radio_index(Cell *on_cell)
{
    uint32_t i = 0;
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrpt_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_BUTTON && _button_is_radio(cast_const(cell->content.component, Button)))
        {
            if (cell == on_cell)
                return i;
            else
                i += 1;
        }
    arrpt_end()
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

Button *_cell_radio_listener(Cell *on_cell)
{
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrpt_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_BUTTON && _button_is_radio(cast_const(cell->content.component, Button)))
        {
            if (_button_radio_listener(cast_const(cell->content.component, Button)) != NULL)
                return cast(cell->content.component, Button);
        }
    arrpt_end()
    return NULL;
}

/*---------------------------------------------------------------------------*/

Cell *_cell_radio_dbind_cell(Cell *on_cell)
{
    cassert_no_null(on_cell);
    cassert_no_null(on_cell->parent);
    arrpt_foreach(cell, on_cell->parent->cells, Cell)
        if (cell->type == i_ekCOMPONENT && cell->content.component->type == ekGUI_TYPE_BUTTON && _button_is_radio(cast_const(cell->content.component, Button)))
        {
            if (cell->member_id != UINT32_MAX)
                return cell;
        }
    arrpt_end()
    return NULL;
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
        layout = _panel_active_layout(layout->panel);
        while (layout->stbind == NULL && layout->parent != NULL)
            layout = layout->parent->parent;
    }

    if (layout->stbind != NULL)
    {
        Layout *layout_notify = layout;
        *obj = layout->objbind;

        /* Find the closest parent layout with notification data */
        while (layout_notify->OnObjChange == NULL && layout_notify->parent != NULL)
            layout_notify = layout_notify->parent->parent;

        if (layout_notify->OnObjChange != NULL)
            *layout_notif = layout_notify;
    }

    return layout;
}

/*---------------------------------------------------------------------------*/

bool_t _cell_filter_str(Cell *cell, const char_t *str, char_t *dest, const uint32_t size)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        cassert_no_null(layout);
        return dbind_st_str_filter(layout->stbind, cell->member_id, str, dest, size);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

void _cell_update_bool(Cell *cell, const bool_t value)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (obj != NULL)
            _gbind_update_bool(layout, layout->stbind, cell->member_id, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_update_u32(Cell *cell, const uint32_t value)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (obj != NULL)
            _gbind_update_u32(layout, layout->stbind, cell->member_id, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_update_norm32(Cell *cell, const real32_t value)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (layout->objbind != NULL)
            _gbind_update_norm32(layout, layout->stbind, cell->member_id, obj, layout_notif, value);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_update_str(Cell *cell, const char_t *str)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (layout->objbind != NULL)
            _gbind_update_str(layout, layout->stbind, cell->member_id, obj, layout_notif, str);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_update_image(Cell *cell, const Image *image)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (layout->objbind != NULL)
            _gbind_update_image(layout, layout->stbind, cell->member_id, obj, layout_notif, image);
    }
}

/*---------------------------------------------------------------------------*/

void _cell_update_incr(Cell *cell, const bool_t pos)
{
    cassert_no_null(cell);
    if (cell->member_id != UINT32_MAX)
    {
        void *obj = NULL;
        Layout *layout_notif = NULL;
        Layout *layout = i_cell_obj(cell, &obj, &layout_notif);
        if (layout->objbind != NULL)
            _gbind_update_incr(layout, layout->stbind, cell->member_id, obj, layout_notif, pos);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _cell_enabled(const Cell *cell)
{
    cassert_no_null(cell);
    return cell->enabled;
}