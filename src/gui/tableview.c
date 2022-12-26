/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tableview.c
 *
 */

/* TableView */

#include "tableview.h"
#include "drawctrl.inl"
#include "component.inl"
#include "view.h"
#include "view.inl"
#include "gui.inl"

#include "arrst.h"
#include "bmath.h"
#include "cassert.h"
#include "color.h"
#include "dctx.h"
#include "draw.h"
#include "event.h"
#include "font.h"
#include "gui.h"
#include "heap.h"
#include "s2d.h"
#include "strings.h"
#include "types.h"
#include "window.h"

typedef struct _column_t Column;
typedef struct _tdata_t TData;

typedef enum _ctype_t
{
    ekCTYPE_TEXT
} ctype_t;

struct _column_t
{
    ctype_t type;
    ResId head_textid;
    String *head_text;
    Font *font;
    uint32_t width;
    uint32_t yoffset;
    uint32_t indicator;
    uint32_t min_width;
    uint32_t max_width;
    align_t align;
    bool_t editable;
    bool_t resizable;
};

struct _tdata_t
{
    Font *font;
    Font *head_font;
    ArrSt(Column) *columns;
    ArrSt(uint32_t) *selected;
    uint32_t num_rows;
    uint32_t focus_row;
    uint32_t head_height;
    uint32_t head_yoffset;
    uint32_t row_height;
    uint32_t mouse_row;
    uint32_t mouse_head;
    uint32_t mouse_sep;
    uint32_t resize_mouse_x;
    uint32_t resize_col_width;
    uint32_t control_width;
    uint32_t control_height;
    uint32_t content_width;
    uint32_t content_height;
    uint32_t freeze_col_id;
    ctrl_msel_t multisel_mode;
    gui_cursor_t cursor;
    bool_t head_visible;
    bool_t head_clickable;
    bool_t head_resizable;
    bool_t mouse_down;
    bool_t focused;
    bool_t multisel;
    bool_t preserve;
    bool_t hlines;
    bool_t vlines;
    Listener *OnData;
    Listener *OnSelect;
    Listener *OnHeaderClick;
    Window *window;
};

/*---------------------------------------------------------------------------*/

DeclSt(Column);
static const uint32_t i_COLUMN_LEFT_PADDING = 5;
static const uint32_t i_COLUMN_RIGHT_PADDING = 5;
static const uint32_t i_COLUMN_MIN_DISPLAY = 15;
static const uint32_t i_BOTTOM_PADDING = 10;
static const uint32_t i_DOCUMENT_RIGHT_MARGIN = 20;
static const uint32_t i_HORIZONTAL_KEY_SCROLL = 20;
static const uint32_t i_MOUSE_HEADER_RESIZE = 3;

static const char_t* i_EMPTY_TEXT = "";

/*---------------------------------------------------------------------------*/

static TData *i_create_data(void)
{
    TData *data = heap_new0(TData);
    data->font = font_system(font_regular_size(), 0);
    data->head_font = font_copy(data->font);
    data->columns = arrst_create(Column);
    data->selected = arrst_create(uint32_t);
    data->focus_row = UINT32_MAX;
    data->mouse_row = UINT32_MAX;
    data->mouse_head = UINT32_MAX;
    data->mouse_sep = UINT32_MAX;
    data->resize_mouse_x = UINT32_MAX;
    data->resize_col_width = UINT32_MAX;
    data->head_visible = TRUE;
    data->content_width = UINT32_MAX;
    data->content_height = UINT32_MAX;
    data->freeze_col_id = UINT32_MAX;
    data->multisel_mode = ekCTRL_MSEL_NO;
    data->cursor = ekGUI_CURSOR_ARROW;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_remove_column(Column *column)
{
    str_destroy(&column->head_text);
    font_destroy(&column->font);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(TData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    font_destroy(&(*data)->font);
    font_destroy(&(*data)->head_font);
    listener_destroy(&(*data)->OnData);
    listener_destroy(&(*data)->OnSelect);
    listener_destroy(&(*data)->OnHeaderClick);
    arrst_destroy(&(*data)->columns, i_remove_column, Column);
    arrst_destroy(&(*data)->selected, NULL, uint32_t);
    heap_delete(data, TData);
}

/*---------------------------------------------------------------------------*/

static void i_cell_data(TableView *view, const TData *data, const uint32_t col_id, const uint32_t row_id, EvTbCell *cell)
{
    cassert_no_null(data);
    cassert_no_null(cell);
    cell->text = i_EMPTY_TEXT;
    cell->align = ekLEFT;

    if (data->OnData != NULL)
    {
        EvTbPos pos;
        pos.col = col_id;
        pos.row = row_id;
        listener_event(data->OnData, ekGUI_EVENT_TBL_CELL, view, &pos, cell, TableView, EvTbPos, EvTbCell);
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_cell(const EvTbCell *cell, DCtx *ctx, const Column *col, const uint32_t x, const uint32_t y, const uint32_t width, ctrl_state_t state)
{
    cassert_no_null(col);
    switch (col->type) {
    case ekCTYPE_TEXT:
        if (width > i_COLUMN_MIN_DISPLAY)
        {
            draw_font(ctx, col->font);
            draw_text_width(ctx, (real32_t)(width - i_COLUMN_LEFT_PADDING - i_COLUMN_RIGHT_PADDING));
            draw_text_halign(ctx, cell->align);
            drawctrl_text(ctx, cell->text, (int32_t)(x + i_COLUMN_LEFT_PADDING), (int32_t)(y + col->yoffset), state);
        }
        break;

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static uint32_t i_num_visible_rows(const uint32_t view_height, const uint32_t head_height, const uint32_t row_height, const uint32_t sty)
{
    if (view_height >= head_height)
    {
        uint32_t y = sty + head_height;
        uint32_t height = view_height - head_height;
        uint32_t first = row_height - (y % row_height);
        uint32_t nrows = 1;

        /* Visible pixels of first row */
        if (height <= first)
            return nrows;

        height -= first;
        nrows += height / row_height;

        if (height % row_height != 0)
            nrows += 1;

        return nrows;
    }

    /* View is too small or header too big */
    return 0;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_freezed_width(const ArrSt(Column)* columns, const uint32_t freeze_col_id)
{
    uint32_t width = 0;

    if (freeze_col_id != UINT32_MAX)
    {
        const Column* col = arrst_all_const(columns, Column);
        uint32_t i = 0;
        for (i = 0; i <= freeze_col_id; ++i)
            width += col[i].width;
    }

    return width;
}

/*---------------------------------------------------------------------------*/

static void i_visible_cols(const ArrSt(Column) *columns, const uint32_t freeze_width, const uint32_t freeze_col_id, const uint32_t stx, const uint32_t width, uint32_t *stcol, uint32_t *edcol, uint32_t *x)
{
    uint32_t lx = 0, st = UINT32_MAX, ed = UINT32_MAX;
    cassert_no_null(stcol);
    cassert_no_null(edcol);
    cassert_no_null(x);

    arrst_foreach_const(col, columns, Column)

        if (st == UINT32_MAX)
        {
             /* The column is completely to the left of the visible area */
             if (lx + col->width > stx + freeze_width)
             {
                 if (freeze_col_id == UINT32_MAX || freeze_col_id < col_i)
                 {
                     st = col_i;
                     *x = lx;
                 }
             }
        }
        else if (lx > stx + width)
        {
            /* The column is completely to the right of the visible area */
            ed = col_i;
            break;
        }

        lx += col->width;

    arrst_end();

    if (st == UINT32_MAX)
    {
        cassert(ed == UINT32_MAX);
    }
    else if (ed == UINT32_MAX)
    {
        ed = arrst_size(columns, Column);
    }

    *stcol = st;
    *edcol = ed;
}

/*---------------------------------------------------------------------------*/

static int i_uint32_cmp(const uint32_t *u1, const uint32_t *u2)
{
    return (int)(*u1 - *u2);
}

/*---------------------------------------------------------------------------*/

static bool_t i_row_is_selected(const ArrSt(uint32_t) *selected, const uint32_t row)
{
    const uint32_t *elem = arrst_bsearch_const(selected, i_uint32_cmp, &row, NULL, uint32_t, uint32_t);
    return (bool_t)(elem != NULL);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(TableView *view, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    TData *data = view_get_data((View*)view, TData);
    uint32_t freeze_width = UINT32_MAX;
    uint32_t nc = 0, nr = 0;
    cassert_no_null(data);

    freeze_width = i_freezed_width(data->columns, data->freeze_col_id);
    nc = arrst_size(data->columns, Column);
    nr = data->num_rows;
    drawctrl_clear(p->ctx, (int32_t)freeze_width, 0, data->control_width - freeze_width, data->control_height);

    if (nc > 0 && nr > 0)
    {
        uint32_t stx = (uint32_t)p->x;
        uint32_t sty = (uint32_t)p->y;
        uint32_t head_height = 0;
        const Column *cols = arrst_all(data->columns, Column);
        uint32_t strow = UINT32_MAX;
        uint32_t edrow = UINT32_MAX;
        uint32_t vrows = UINT32_MAX;
        uint32_t stcol = UINT32_MAX;
        uint32_t edcol = UINT32_MAX;
        uint32_t xmin = UINT32_MAX;
        uint32_t xmax = UINT32_MAX;
        uint32_t y = UINT32_MAX;
        int32_t focus_x = INT32_MAX;
        int32_t focus_y = INT32_MAX;
        uint32_t focus_width = UINT32_MAX;
        uint32_t focus_height = UINT32_MAX;
        ctrl_state_t focus_state = ENUM_MAX(ctrl_state_t);
        uint32_t i, j;

        if (data->head_visible == TRUE)
            head_height = data->head_height;

        strow = sty / data->row_height;
        vrows = i_num_visible_rows((uint32_t)p->height, head_height, data->row_height, sty);
        edrow = min_u32(nr, strow + vrows);
        i_visible_cols(data->columns, freeze_width, data->freeze_col_id, stx, (uint32_t)p->width, &stcol, &edcol, &xmin);

        /*
        #include "bstd.h"
        bstd_printf("X: %d Y: %d W: %d H: %d\n", (uint32_t)p->x, (uint32_t)p->y, (uint32_t)p->width, (uint32_t)p->height);
        bstd_printf("CX: %d CY: %d\n", data->control_width, data->control_height);
        bstd_printf("AX: %d AY: %d\n", data->content_width, data->content_height);
        bstd_printf("StRow: %d EdRow: %d\n", strow, edrow);
        bstd_printf("StCol: %d EdCol: %d X:%d\n", stcol, edcol, xmin);
        */

        y = head_height + (strow * data->row_height);

        if (data->OnData != NULL)
        {
            EvTbRect rect;
            rect.stcol = stcol;
            rect.edcol = edcol;
            rect.strow = strow;
            rect.edrow = edrow;
            listener_event(data->OnData, ekGUI_EVENT_TBL_BEGIN, view, &rect, NULL, TableView, EvTbRect, void);
        }

        for (i = strow; i < edrow; ++i)
        {
            ctrl_state_t state = data->focused == TRUE ? ekCTRL_STATE_NORMAL : ekCTRL_STATE_BKNORMAL;
            bool_t draw_row = FALSE;
            bool_t selected = i_row_is_selected(data->selected, i);
            uint32_t lx = xmin;
            EvTbCell cell;

            if (selected == TRUE && data->focused == TRUE)
            {
                state = ekCTRL_STATE_PRESSED;
                draw_row = TRUE;
            }
            else if (i == data->mouse_row)
            {
                state = data->focused == TRUE ? ekCTRL_STATE_HOT : ekCTRL_STATE_BKHOT;
                draw_row = TRUE;
            }
            else if (selected == TRUE)
            {
                state = ekCTRL_STATE_BKPRESSED;
                draw_row = TRUE;
            }

            /* Row background color fill */
            if (draw_row == TRUE)
                drawctrl_fill(p->ctx, (int32_t)(stx + freeze_width), (int32_t)y, data->content_width, data->row_height, state);

            /* Draw the columns */
            for (j = stcol; j < edcol; ++j)
            {
                if (cols[j].width > 0)
                {
                    i_cell_data(view, data, j, i, &cell);
                    i_draw_cell(&cell, p->ctx, cols + j, lx, y, cols[j].width, state);
                    lx += cols[j].width;
                }
            }

            xmax = lx;

            if (data->focused == TRUE && data->focus_row == i)
            {
                focus_x = 0;
                focus_y = (int32_t)y;
                focus_width = data->content_width;
                focus_height = data->row_height;
                focus_state = state;
            }

            y += data->row_height;
        }

        /* Draw the freezed columns */
        if (freeze_width > 0)
        {
            drawctrl_clear(p->ctx, 0, 0, freeze_width, data->control_height);

            y = head_height + (strow * data->row_height);

            for (i = strow; i < edrow; ++i)
            {
                ctrl_state_t state = data->focused == TRUE ? ekCTRL_STATE_NORMAL : ekCTRL_STATE_BKNORMAL;
                bool_t draw_row = FALSE;
                bool_t selected = i_row_is_selected(data->selected, i);
                uint32_t lx = stx;
                EvTbCell cell;

                if (selected == TRUE && data->focused == TRUE)
                {
                    state = ekCTRL_STATE_PRESSED;
                    draw_row = TRUE;
                }
                else if (i == data->mouse_row)
                {
                    state = data->focused == TRUE ? ekCTRL_STATE_HOT : ekCTRL_STATE_BKHOT;
                    draw_row = TRUE;
                }
                else if (selected == TRUE)
                {
                    state = ekCTRL_STATE_BKPRESSED;
                    draw_row = TRUE;
                }

                /* Row background color fill */
                if (draw_row == TRUE)
                    drawctrl_fill(p->ctx, (int32_t)stx, (int32_t)y, freeze_width, data->row_height, state);

                /* Draw the columns */
                for (j = 0; j <= data->freeze_col_id; ++j)
                {
                    if (cols[j].width > 0)
                    {
                        i_cell_data(view, data, j, i, &cell);
                        i_draw_cell(&cell, p->ctx, cols + j, lx, y, cols[j].width, state);
                        lx += cols[j].width;
                    }
                }

                y += data->row_height;
            }
        }

        if (data->hlines == TRUE || data->vlines == TRUE)
        {
            draw_antialias(p->ctx, FALSE);
            draw_line_color(p->ctx, gui_line_color());

            if (data->hlines == TRUE)
            {
                y = head_height + strow * data->row_height;
                for (i = strow; i < edrow; ++i)
                {
                    drawctrl_line(p->ctx, (int32_t)stx, (int32_t)y, (int32_t)xmax, (int32_t)y);
                    y += data->row_height;
                }

                drawctrl_line(p->ctx, (int32_t)stx, (int32_t)y, (int32_t)xmax, (int32_t)y);
            }

            if (data->vlines == TRUE)
            {
                uint32_t x = xmin;
                uint32_t ymin = head_height + (strow * data->row_height);
                uint32_t ymax = ymin + (edrow - strow) * data->row_height;

                for (j = stcol; j < edcol; ++j)
                {
                    if (cols[j].width > 0)
                    {
                        /* Don't draw vertial line if is in freezed area */
                        if (x > stx + freeze_width)
                            drawctrl_line(p->ctx, (int32_t)x, (int32_t)ymin, (int32_t)x, (int32_t)ymax);

                        x += cols[j].width;
                    }
                }

                drawctrl_line(p->ctx, (int32_t)x, (int32_t)ymin, (int32_t)x, (int32_t)ymax);

                /* Draw vertial line if is in freezed area */
                if(data->freeze_col_id != UINT32_MAX)
                {
                    x = stx;
                    for(j = 0; j <= data->freeze_col_id; ++j)
                    {
                        x += cols[j].width;
                        if (cols[j].width > 0)
                            drawctrl_line(p->ctx, (int32_t)x, (int32_t)ymin, (int32_t)x, (int32_t)ymax);
                    }
                }
            }

            draw_antialias(p->ctx, TRUE);
        }

        /* The focus rectangle will we draw on top of the grid lines */
        if (focus_x != INT32_MAX)
            drawctrl_focus(p->ctx, focus_x, focus_y, focus_width, focus_height, focus_state);

        if (data->OnData != NULL)
        {
            listener_event(data->OnData, ekGUI_EVENT_TBL_END, view, NULL, NULL, TableView, void, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_header(DCtx *ctx, const TData *data, const Column *col, const uint32_t col_i, const int32_t x)
{
    ctrl_state_t state = ekCTRL_STATE_NORMAL;
    int32_t tx = x + (int32_t)i_COLUMN_LEFT_PADDING;
    int32_t ty = (int32_t)data->head_yoffset;

    cassert_no_null(data);
    cassert_no_null(col);

    if (col_i == data->mouse_head)
    {
        if (data->mouse_down == TRUE && data->head_clickable == TRUE)
        {
            state = ekCTRL_STATE_PRESSED;
            tx += 1;
            ty += 1;
        }
        else
        {
            state = ekCTRL_STATE_HOT;
        }
    }
    else if (col_i == data->mouse_sep)
    {
        state = ekCTRL_STATE_HOT;
    }

    drawctrl_header(ctx, x, 0, col->width, data->head_height, state);

    if (col->width > i_COLUMN_MIN_DISPLAY)
    {
        draw_text_width(ctx, (real32_t)(col->width - i_COLUMN_LEFT_PADDING - i_COLUMN_RIGHT_PADDING));
        draw_text_halign(ctx, col->align);
        drawctrl_text(ctx, tc(col->head_text), tx, ty, data->focused ? ekCTRL_STATE_NORMAL : ekCTRL_STATE_BKNORMAL);

        if (col->indicator != 0)
            drawctrl_indicator(ctx, x, 0, col->width, data->head_height, col->indicator);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnOverlay(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);

    if (data->head_visible == TRUE)
    {
        const EvDraw *p = event_params(e, EvDraw);
        const Column *cols = arrst_all(data->columns, Column);
        uint32_t freeze_width = i_freezed_width(data->columns, data->freeze_col_id);
        uint32_t stx = UINT32_MAX;
        uint32_t stcol = UINT32_MAX;
        uint32_t edcol = UINT32_MAX;
        uint32_t x = UINT32_MAX;
        int32_t lx;
        uint32_t i;

        {
            V2Df pos;
            view_viewport((View*)view, &pos, NULL);
            stx = (uint32_t)pos.x;
        }

        i_visible_cols(data->columns, freeze_width, data->freeze_col_id, stx, data->control_width, &stcol, &edcol, &x);
        lx = (int32_t)x - (int32_t)stx;
        draw_font(p->ctx, data->head_font);

        for (i = stcol; i < edcol; ++i)
        {
            i_draw_header(p->ctx, data, &cols[i], i, lx);
            lx += cols[i].width;
        }

        if ((uint32_t)lx < data->control_width)
        {
            drawctrl_header(p->ctx, lx, 0, data->control_width - (uint32_t)lx, data->head_height, ekCTRL_STATE_NORMAL);
        }

        /* Draw the freezed headers */
        if (freeze_width > 0)
        {
            int32_t fx = 0;
            for (i = 0; i <= data->freeze_col_id; ++i)
            {
                i_draw_header(p->ctx, data, &cols[i], i, fx);
                fx += cols[i].width;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_font_height(const Font *font)
{
    uint32_t fheight = (uint32_t)bmath_ceilf(font_size(font));
    uint32_t height = (uint32_t)bmath_ceilf(font_height(font));

    if ((height - fheight) % 2 == 0)
        height += 1;

    return height;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_col_height(const Column *col)
{
    cassert_no_null(col);
    switch(col->type) {
    case ekCTYPE_TEXT:
    {
        uint32_t height = i_font_height(col->font);
        height += drawctrl_row_padding(NULL);
        return height;
    }

    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_col_y_offset(Column *col, const uint32_t row_height)
{
    cassert_no_null(col);
    switch(col->type) {
    case ekCTYPE_TEXT:
    {
        uint32_t height = i_font_height(col->font);
        col->yoffset = (row_height - height) / 2;
        break;
    }

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_head_height(TData *data)
{
    uint32_t height;
    cassert_no_null(data);
    height = i_font_height(data->head_font);
    data->head_height = height;
    data->head_height += 2 * drawctrl_row_padding(NULL);
    data->head_yoffset = (data->head_height - height) / 2;
}

/*---------------------------------------------------------------------------*/

static void i_row_height(TData *data)
{
    cassert_no_null(data);
    data->row_height = 0;
    arrst_foreach(col, data->columns, Column)
        uint32_t height = i_col_height(col);
        if (height > data->row_height)
            data->row_height = height;
    arrst_end();

    arrst_foreach(col, data->columns, Column)
        i_col_y_offset(col, data->row_height);
    arrst_end();

    if (data->hlines == TRUE)
        data->row_height += 1;
}

/*---------------------------------------------------------------------------*/

static void i_document_size(TableView *view, TData *data)
{
    bool_t update = FALSE;
    cassert_no_null(data);

    /* Document width has been invalidated */
    if (data->content_width == UINT32_MAX)
    {
        uint32_t twidth = 0;

        arrst_foreach(col, data->columns, Column)
            twidth += col->width;
        arrst_end();

        data->content_width = twidth;
        update = TRUE;
    }

    /* Document height has been invalidated */
    if (data->content_height == UINT32_MAX)
    {
        uint32_t theight = 0;
        real32_t scroll_height = 0;

        theight = data->num_rows * data->row_height + i_BOTTOM_PADDING;
        view_scroll_size((View*)view, NULL, &scroll_height);
        theight += (uint32_t)scroll_height;

        if (theight < data->control_height)
            theight = data->control_height;

        if (data->head_visible == TRUE)
            theight += data->head_height;

        data->content_height = theight;
        update = TRUE;
    }

    if (update == TRUE)
    {
        real32_t rmargin;
        uint32_t width;
        view_scroll_size((View*)view, &rmargin, NULL);
        width = (uint32_t)rmargin + i_DOCUMENT_RIGHT_MARGIN + data->content_width;

        if (width < data->control_width)
            width = data->control_width;

        view_content_size((View*)view, s2df((real32_t)width, (real32_t)data->content_height), s2df(10, (real32_t)data->row_height));
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvSize *p = event_params(e, EvSize);
    data->control_width = (uint32_t)p->width;
    data->control_height = (uint32_t)p->height;
    data->content_width = UINT32_MAX;
    data->content_height = UINT32_MAX;
    i_document_size(view, data);
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_set_cursor(TableView *view, TData *data, const gui_cursor_t cursor)
{
    if (data->cursor != cursor)
    {
        if (data->window == NULL)
            data->window = _component_window((GuiComponent*)view);
        window_cursor(data->window, cursor, NULL, 0, 0);
        data->cursor = cursor;
    }
}

/*---------------------------------------------------------------------------*/

static void i_mouse_in_header(TData* data, const Column *cols, const uint32_t stcol, const uint32_t edcol, const uint32_t stx, uint32_t mouse_x)
{
    uint32_t i = 0, x = stx;
    cassert_no_null(data);
    cassert(data->mouse_head == UINT32_MAX);
    cassert(data->mouse_sep == UINT32_MAX);

    for (i = stcol; i < edcol; ++i)
    {
        if (data->head_resizable == TRUE && cols[i].resizable == TRUE)
        {
            uint32_t sep_st = x + cols[i].width - i_MOUSE_HEADER_RESIZE;
            uint32_t sep_ed = x + cols[i].width + i_MOUSE_HEADER_RESIZE;

            /* If some columns are width = 0, chose the right-most */
            if (mouse_x >= sep_st && mouse_x <= sep_ed)
            {
                data->mouse_sep = i;
                data->mouse_head = UINT32_MAX;
            }
        }

        if (data->mouse_sep == UINT32_MAX && mouse_x > x && mouse_x <= x + cols[i].width)
        {
            data->mouse_head = i;
            break;
        }

        x += cols[i].width;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnMove(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvMouse *p = event_params(e, EvMouse);
    uint32_t mouse_x = (uint32_t)p->x;
    uint32_t mouse_y = (uint32_t)p->y;
    uint32_t mouse_ly = (uint32_t)p->ly;

    cassert(data->mouse_down == FALSE);

    data->mouse_row = UINT32_MAX;
    data->mouse_head = UINT32_MAX;
    data->mouse_sep = UINT32_MAX;

    if (mouse_x < data->content_width)
    {
        uint32_t head_height = 0;

        if (data->head_visible == TRUE)
            head_height = data->head_height;

        if (head_height > 0)
        {
            if (mouse_ly >= head_height)
                data->mouse_row = (mouse_y - head_height) / data->row_height;
        }
        else
        {
            data->mouse_row = mouse_y / data->row_height;
        }

        /* Mouse is over header */
        if (data->mouse_row == UINT32_MAX)
        {
            const Column *cols = arrst_all(data->columns, Column);
            uint32_t freeze_width = i_freezed_width(data->columns, data->freeze_col_id);
            uint32_t stx = UINT32_MAX;
            uint32_t stcol = UINT32_MAX;
            uint32_t edcol = UINT32_MAX;
            uint32_t x = UINT32_MAX;

            {
                V2Df pos;
                view_viewport((View*)view, &pos, NULL);
                stx = (uint32_t)pos.x;
            }

            i_visible_cols(data->columns, freeze_width, data->freeze_col_id, stx, data->control_width, &stcol, &edcol, &x);

            cassert(mouse_x >= stx);
            if (freeze_width == 0 || (mouse_x - stx) > freeze_width)
                i_mouse_in_header(data, cols, stcol, edcol, x, mouse_x);

            /* Check if mouse is over freezed header */
            if (data->freeze_col_id != UINT32_MAX)
            {
                if (data->mouse_head == UINT32_MAX && data->mouse_sep == UINT32_MAX)
                    i_mouse_in_header(data, cols, 0, data->freeze_col_id + 1, 0, mouse_x - stx);
            }

            if (data->mouse_sep != UINT32_MAX)
                i_set_cursor(view, data, ekGUI_CURSOR_SIZEWE);
            else
                i_set_cursor(view, data, ekGUI_CURSOR_ARROW);
        }
        else
        {
            i_set_cursor(view, data, ekGUI_CURSOR_ARROW);
        }
    }

    view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrag(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    cassert(data->mouse_down == TRUE);

    if (data->mouse_sep != UINT32_MAX)
    {
        /* Column resizing */
        const EvMouse *p = event_params(e, EvMouse);
        int32_t col_width = (int32_t)data->resize_col_width + (int32_t)p->x - (int32_t)data->resize_mouse_x;
        Column *col = arrst_get(data->columns, data->mouse_sep, Column);

        if (col_width < (int32_t)col->min_width)
            col->width = col->min_width;
        else if ((uint32_t)col_width > col->max_width)
            col->width = col->max_width;
        else
            col->width = (uint32_t)col_width;

        data->content_width = UINT32_MAX;
        i_document_size(view, data);
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    data->mouse_row = UINT32_MAX;
    data->mouse_head = UINT32_MAX;
    data->mouse_sep = UINT32_MAX;
    if (data->mouse_down == FALSE)
        i_set_cursor(view, data, ekGUI_CURSOR_ARROW);
    view_update((View*)view);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const bool_t *p = event_params(e, bool_t);
    data->focused = *p;
    if (data->focused == TRUE && data->focus_row == UINT32_MAX && data->num_rows > 0)
        data->focus_row = 0;
    view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_only_selected(const ArrSt(uint32_t) *selected, const uint32_t row)
{
    if (arrst_size(selected, uint32_t) != 1)
        return FALSE;
    return (bool_t)(*arrst_first_const(selected, uint32_t) == row);
}

/*---------------------------------------------------------------------------*/

static bool_t i_select(TableView *view, TData *data, const uint32_t row, const uint32_t strow, const bool_t bymouse)
{
    bool_t change_sel = FALSE;
    cassert_no_null(data);
    if (data->multisel_mode == ekCTRL_MSEL_NO)
    {
        if (data->multisel == FALSE || data->preserve == FALSE)
        {
            if (i_is_only_selected(data->selected, row) == FALSE)
            {
                arrst_clear(data->selected, NULL, uint32_t);
                arrst_append(data->selected, row, uint32_t);
                change_sel = TRUE;
            }
        }
    }
    else if (data->multisel_mode == ekCTRL_MSEL_SINGLE)
    {
        if (bymouse == TRUE)
        {
            uint32_t pos = 0;
            uint32_t *elem = arrst_bsearch(data->selected, i_uint32_cmp, &row, &pos, uint32_t, uint32_t);
            if (elem != NULL)
                arrst_delete(data->selected, pos, NULL, uint32_t);
            else
                arrst_insert(data->selected, pos, row, uint32_t);
            change_sel = TRUE;
        }
    }
    else if (data->multisel_mode == ekCTRL_MSEL_BURST)
    {
        if (bymouse == FALSE)
        {
            uint32_t i, st, ed;
            if (strow == UINT32_MAX)
            {
                st = row;
                ed = row;
            }
            else if (strow < row)
            {
                st = strow;
                ed = row;
            }
            else
            {
                st = row;
                ed = strow;
            }

            for (i = st; i <= ed; ++i)
            {
                uint32_t pos = 0;
                uint32_t *elem = arrst_bsearch(data->selected, i_uint32_cmp, &i, &pos, uint32_t, uint32_t);
                if (elem == NULL)
                {
                    arrst_insert(data->selected, pos, i, uint32_t);
                    change_sel = TRUE;
                }
            }
        }
    }

    if (change_sel == TRUE && data->OnSelect != NULL)
    {
        EvTbSel p;
        p.sel = data->selected;
        listener_event(data->OnSelect, ekGUI_EVENT_TBL_SEL, view, &p, NULL, TableView, EvTbSel, void);
    }

    return change_sel;
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvMouse *p = event_params(e, EvMouse);
    uint32_t n = data->num_rows;

    if (n > 0 && p->button == ekGUI_MOUSE_LEFT)
    {
        data->mouse_down = TRUE;

        if (data->mouse_sep != UINT32_MAX)
        {
            const Column *col = arrst_get_const(data->columns, data->mouse_sep, Column);
            data->resize_mouse_x = (uint32_t)p->x;
            data->resize_col_width = col->width;
        }
        else if (data->mouse_head != UINT32_MAX)
        {
            if (data->head_clickable == TRUE)
            {
                view_update((View*)view);

                if (data->OnHeaderClick != NULL)
                {
                    EvButton params;
                    const Column *col = arrst_get_const(data->columns, data->mouse_head, Column);
                    params.index = data->mouse_head;
                    params.state = ekGUI_ON;
                    params.text = tc(col->head_text);
                    listener_event(data->OnHeaderClick, ekGUI_EVENT_TBL_HEADCLICK, view, &params, NULL, TableView, EvButton, void);
                }
            }
        }
        else if (data->mouse_row != UINT32_MAX)
        {
            bool_t changed = FALSE;

            if (data->mouse_row < n)
            {
                changed |= i_select(view, data, data->mouse_row, UINT32_MAX, TRUE);
                if (data->focus_row != data->mouse_row)
                {
                    data->focus_row = data->mouse_row;
                    changed |= TRUE;
                }
            }
            else
            {
                if (arrst_size(data->selected, uint32_t) > 0)
                {
                    arrst_clear(data->selected, NULL, uint32_t);
                    changed |= TRUE;
                }
            }

            if (changed == TRUE)
                view_update((View*)view);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnUp(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    data->mouse_down = FALSE;
    data->resize_mouse_x = UINT32_MAX;
    data->resize_col_width = UINT32_MAX;
    i_set_cursor(view, data, ekGUI_CURSOR_ARROW);
    view_update((View*)view);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static bool_t i_update_sel_top(TableView *view, TData *data, const uint32_t scroll_y)
{
    if (scroll_y > 0)
    {
        uint32_t ypos = data->focus_row * data->row_height;
        if (scroll_y > ypos)
        {
            view_scroll_y((View*)view, (real32_t)ypos);
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_update_sel_bottom(TableView *view, TData *data, const uint32_t scroll_y)
{
    uint32_t ypos = (data->focus_row + 1) * data->row_height + i_BOTTOM_PADDING;
    real32_t scroll_height = 0;

    if (data->head_visible == TRUE)
        ypos += data->head_height;

    view_scroll_size((View*)view, NULL, &scroll_height);
    if (scroll_y + data->control_height - scroll_height < ypos)
    {
        view_scroll_y((View*)view, (real32_t)ypos - (real32_t)data->control_height + scroll_height);
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDown(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvKey *p = event_params(e, EvKey);
    uint32_t n = data->num_rows;

    if (n > 0)
    {
        uint32_t scroll_x, scroll_y;

        {
            V2Df pos;
            view_viewport((View*)view, &pos, NULL);
            scroll_x = (uint32_t)pos.x;
            scroll_y = (uint32_t)pos.y;
        }

        if (p->key == ekKEY_UP)
        {
            bool_t update = FALSE;
            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row > 0)
            {
                data->focus_row -= 1;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, UINT32_MAX, FALSE);
                i_update_sel_top(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_DOWN)
        {
            bool_t update = FALSE;
            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row < n - 1)
            {
                data->focus_row += 1;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, UINT32_MAX, FALSE);
                i_update_sel_bottom(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_HOME)
        {
            bool_t update = FALSE;
            uint32_t focus_row = data->focus_row;

            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = 0;
                focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row > 0)
            {
                data->focus_row = 0;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, focus_row, FALSE);
                i_update_sel_top(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_END)
        {
            bool_t update = FALSE;
            uint32_t focus_row = data->focus_row;

            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = n - 1;
                focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row < n - 1)
            {
                data->focus_row = n - 1;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, focus_row, FALSE);
                i_update_sel_bottom(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_PAGEUP)
        {
            bool_t update = FALSE;
            uint32_t head_height = data->head_visible ? data->head_height : 0;
            uint32_t psize = i_num_visible_rows(data->control_height, head_height, data->row_height, head_height);
            uint32_t focus_row = data->focus_row;

            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = 0;
                focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row > 0)
            {
                if (data->focus_row > (psize - 1))
                    data->focus_row -= (psize - 1);
                else
                    data->focus_row = 0;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, focus_row, FALSE);
                i_update_sel_top(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_PAGEDOWN)
        {
            bool_t update = FALSE;
            uint32_t head_height = data->head_visible ? data->head_height : 0;
            uint32_t psize = i_num_visible_rows(data->control_height, head_height, data->row_height, head_height);
            uint32_t focus_row = data->focus_row;

            if (data->focus_row == UINT32_MAX)
            {
                data->focus_row = 0;
                focus_row = 0;
                update = TRUE;
            }
            else if (data->focus_row < n - 1)
            {
                data->focus_row += (psize - 1);
                if (data->focus_row >= n)
                    data->focus_row = n - 1;
                update = TRUE;
            }

            if (update == TRUE)
            {
                i_select(view, data, data->focus_row, focus_row, FALSE);
                i_update_sel_bottom(view, data, scroll_y);
                view_update((View*)view);
            }
        }
        else if (p->key == ekKEY_LEFT)
        {
            if (data->content_width > data->control_width)
            {
                if (scroll_x > 0)
                {
                    view_scroll_x((View*)view, (real32_t)(scroll_x > i_HORIZONTAL_KEY_SCROLL ? scroll_x - i_HORIZONTAL_KEY_SCROLL : 0));
                    view_update((View*)view);
                }
            }
        }
        else if (p->key == ekKEY_RIGHT)
        {
            real32_t scroll_width = 0;
            view_scroll_size((View*)view, &scroll_width, NULL);
            if (data->content_width > data->control_width - scroll_width)
            {
                if (scroll_x < data->content_width - data->control_width + scroll_width)
                {
                    view_scroll_x((View*)view, (real32_t)(scroll_x + i_HORIZONTAL_KEY_SCROLL));
                    view_update((View*)view);
                }
            }
        }
        else if (data->multisel == TRUE)
        {
            ctrl_msel_t msel = drawctrl_multisel(NULL, p->key);
            /* Pressed a key that activate multiselect (p.e. Ctrl) */
            if (msel != ekCTRL_MSEL_NO)
                data->multisel_mode = msel;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyUp(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvKey *p = event_params(e, EvKey);
    ctrl_msel_t msel = drawctrl_multisel(NULL, p->key);
    /* Released a key that activate multiselect (p.e. Ctrl) */
    if (msel != ekCTRL_MSEL_NO)
        data->multisel_mode = ekCTRL_MSEL_NO;
}

/*---------------------------------------------------------------------------*/

TableView *tableview_create(void)
{
    View *view = _view_create(ekVIEW_HSCROLL | ekVIEW_VSCROLL | ekVIEW_BORDER | ekVIEW_CONTROL | ekVIEW_NOERASE);
    TData *data = i_create_data();

    /*
    i_IMAGE = image_from_file("/Users/fran/Desktop/disk64.png", NULL);
    i_IMAGE = image_from_file("C:\\Users\\USUARIO\\Desktop\\disk64.png", NULL);
    */

    view_OnDraw(view, listener((TableView*)view, i_OnDraw, TableView));
    view_OnOverlay(view, listener((TableView*)view, i_OnOverlay, TableView));
    view_OnSize(view, listener((TableView*)view, i_OnSize, TableView));
    view_OnMove(view, listener((TableView*)view, i_OnMove, TableView));
    view_OnDrag(view, listener((TableView*)view, i_OnDrag, TableView));
    view_OnExit(view, listener((TableView*)view, i_OnExit, TableView));
    view_OnFocus(view, listener((TableView*)view, i_OnFocus, TableView));
    view_OnDown(view, listener((TableView*)view, i_OnDown, TableView));
    view_OnUp(view, listener((TableView*)view, i_OnUp, TableView));
    view_OnKeyDown(view, listener((TableView*)view, i_OnKeyDown, TableView));
    view_OnKeyUp(view, listener((TableView*)view, i_OnKeyUp, TableView));
    _view_set_subtype(view, "TableView");
    view_size(view, s2df(256, 128));
    i_head_height(data);
    i_document_size((TableView*)view, data);
    view_data(view, &data, i_destroy_data, TData);
    return (TableView*)view;
}

/*---------------------------------------------------------------------------*/

void tableview_OnData(TableView *view, Listener *listener)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    listener_update(&data->OnData, listener);
}

/*---------------------------------------------------------------------------*/

void tableview_OnSelect(TableView *view, Listener *listener)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    listener_update(&data->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void tableview_OnHeaderClick(TableView *view, Listener *listener)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    listener_update(&data->OnHeaderClick, listener);
}

/*---------------------------------------------------------------------------*/

void tableview_font(TableView* view, const Font* font)
{
    TData *data = view_get_data((View*)view, TData);
    bool_t updated = FALSE;
    cassert_no_null(data);
    _gui_update_font(&data->font, NULL, font);
    if (_gui_update_font(&data->head_font, NULL, font) == TRUE)
    {
        i_head_height(data);
        view_update((View*)view);
    }

    arrst_foreach(col, data->columns, Column)
        updated |= _gui_update_font(&col->font, NULL, font);
    arrst_end();

    if(updated == TRUE)
    {
        data->content_width = UINT32_MAX;
        data->content_height = UINT32_MAX;
        i_row_height(data);
        i_document_size(view, data);
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_size(TableView *view, S2Df size)
{
    view_size((View*)view, size);
}

/*---------------------------------------------------------------------------*/

uint32_t tableview_new_column_text(TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    uint32_t n = 0;
    Column *column = NULL;
    cassert_no_null(data);
    n = arrst_size(data->columns, Column);
    column = arrst_new0(data->columns, Column);
    column->type = ekCTYPE_TEXT;
    column->head_text = str_printf("Column %d", n);
    column->font = font_copy(data->font);
    column->width = 150;
    column->min_width = 0;
    column->max_width = UINT32_MAX;
    column->align = ekLEFT;
    column->editable = FALSE;
    column->resizable = TRUE;
    i_row_height(data);
    data->content_width = UINT32_MAX;
    i_document_size(view, data);
    view_update((View*)view);
    return n;
}

/*---------------------------------------------------------------------------*/

void tableview_column_width(TableView *view, const uint32_t column_id, const real32_t width)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    if (column->width != (uint32_t)width)
    {
        column->width = (uint32_t)width;

        if (column->width < column->min_width)
            column->width = column->min_width;
        else if (column->width > column->max_width)
            column->width = column->max_width;

        data->content_width = UINT32_MAX;
        i_document_size(view, data);
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_column_limits(TableView *view, const uint32_t column_id, const real32_t min, const real32_t max)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    cassert(min >= 0);
    cassert(max >= min);
    if (column->min_width != (uint32_t)min || column->max_width != (uint32_t)max)
    {
        column->min_width = (uint32_t)min;
        column->max_width = (uint32_t)max;
        if (column->width < column->min_width)
        {
            column->width = column->min_width;
            data->content_width = UINT32_MAX;
            i_document_size(view, data);
            view_update((View*)view);
        }
        else if (column->width > column->max_width)
        {
            column->width = column->max_width;
            data->content_width = UINT32_MAX;
            i_document_size(view, data);
            view_update((View*)view);
        }
    }
}

/*---------------------------------------------------------------------------*/

void tableview_column_resizable(TableView *view, const uint32_t column_id, const bool_t resizable)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    if (column->resizable != resizable)
    {
        column->resizable = resizable;
    }
}

/*---------------------------------------------------------------------------*/

void tableview_column_freeze(TableView *view, const uint32_t last_column_id)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    if (data->freeze_col_id != last_column_id)
    {
        cassert(last_column_id == UINT32_MAX || last_column_id < arrst_size(data->columns, Column));
        data->freeze_col_id = last_column_id;
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_header_title(TableView *view, const uint32_t column_id, const char_t *text)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    const char_t *ltext = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    ltext = _gui_respack_text(text, &column->head_textid);
    str_upd(&column->head_text, ltext);
    view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

void tableview_header_align(TableView *view, const uint32_t column_id, const align_t align)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    if (column->align != align)
    {
        column->align = align;
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_header_indicator(TableView *view, const uint32_t column_id, const uint32_t indicator)
{
    TData *data = view_get_data((View*)view, TData);
    Column *column = NULL;
    cassert_no_null(data);
    column = arrst_get(data->columns, column_id, Column);
    if (column->indicator != indicator)
    {
        column->indicator = indicator;
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_header_visible(TableView *view, const bool_t visible)
{
    TData *data = view_get_data((View*)view, TData);
    if (data->head_visible != visible)
    {
        data->head_visible = visible;
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_header_clickable(TableView *view, const bool_t clickable)
{
    TData *data = view_get_data((View*)view, TData);
    if (data->head_clickable != clickable)
    {
        data->head_clickable = clickable;
    }
}

/*---------------------------------------------------------------------------*/

void tableview_header_resizable(TableView *view, const bool_t resizable)
{
    TData *data = view_get_data((View*)view, TData);
    if (data->head_resizable != resizable)
    {
        data->head_resizable = resizable;
    }
}

/*---------------------------------------------------------------------------*/

void tableview_multisel(TableView *view, const bool_t multisel, const bool_t preserve)
{
    TData *data = view_get_data((View*)view, TData);
    data->multisel = multisel;
    data->preserve = preserve;
    if (multisel == FALSE)
    {
        uint32_t sel = UINT32_MAX;
        if (arrst_size(data->selected, uint32_t) > 0)
            sel = *arrst_first_const(data->selected, uint32_t);
        arrst_clear(data->selected, NULL, uint32_t);
        if (sel != UINT32_MAX)
            arrst_append(data->selected, sel, uint32_t);
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

void tableview_grid(TableView *view, const bool_t hlines, const bool_t vlines)
{
    TData *data = view_get_data((View*)view, TData);
    bool_t update = FALSE;

    if (data->hlines != hlines)
    {
        data->hlines = hlines;
        data->content_height = UINT32_MAX;
        i_row_height(data);
        i_document_size(view, data);
        update = TRUE;
    }

    if (data->vlines != vlines)
    {
        data->vlines = vlines;
        update = TRUE;
    }

    if (update == TRUE)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static void i_delete_out_bounds(ArrSt(uint32_t) *selected, const uint32_t bound)
{
    uint32_t i, n = arrst_size(selected, uint32_t);
    for(i = 0; i < n;)
    {
        uint32_t *v = arrst_get(selected, i, uint32_t);
        if (*v < bound)
        {
            i += 1;
        }
        else
        {
            arrst_delete(selected, i, NULL, uint32_t);
            n -= 1;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_num_rows(TableView *view, TData *data)
{
    cassert_no_null(data);
    data->num_rows = 0;
    if (data->OnData != NULL)
    {
        listener_event(data->OnData, ekGUI_EVENT_TBL_NROWS, view, NULL, &data->num_rows, TableView, void, uint32_t);
    }

    i_delete_out_bounds(data->selected, data->num_rows);

    if (data->focus_row != UINT32_MAX)
    {
        if (data->num_rows == 0)
            data->focus_row = UINT32_MAX;
        else if (data->focus_row >= data->num_rows)
            data->focus_row = data->num_rows - 1;
    }
}

/*---------------------------------------------------------------------------*/

void tableview_update(TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    i_num_rows(view, data);
    data->content_height = UINT32_MAX;
    i_document_size(view, data);
    view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

void tableview_select(TableView *view, const uint32_t *rows, const uint32_t n)
{
    TData *data = view_get_data((View*)view, TData);
    bool_t updated = FALSE;

    cassert_no_null(data);
    if (data->multisel == TRUE)
    {
        uint32_t i;
        for (i = 0; i < n; ++i)
        {
            uint32_t row = rows[i];
            if (row < data->num_rows)
            {
                uint32_t pos = 0;
                uint32_t *elem = arrst_bsearch(data->selected, i_uint32_cmp, &row, &pos, uint32_t, uint32_t);
                if (elem == NULL)
                {
                    arrst_insert(data->selected, pos, row, uint32_t);
                    updated = TRUE;
                }
            }
        }
    }
    else
    {
        if (n > 0 && rows[0] < data->num_rows)
        {
            if (i_is_only_selected(data->selected, rows[0]) == FALSE)
            {
                arrst_clear(data->selected, NULL, uint32_t);
                arrst_append(data->selected, rows[0], uint32_t);
                updated = TRUE;
            }
        }
    }

    if (updated == TRUE)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

void tableview_deselect(TableView *view, const uint32_t *rows, const uint32_t n)
{
    TData *data = view_get_data((View*)view, TData);
    bool_t updated = FALSE;

    cassert_no_null(data);
    if (data->multisel == TRUE)
    {
        uint32_t i;
        for (i = 0; i < n; ++i)
        {
            uint32_t row = rows[i];
            uint32_t pos = 0;
            uint32_t *elem = arrst_bsearch(data->selected, i_uint32_cmp, &row, &pos, uint32_t, uint32_t);
            if (elem != NULL)
            {
                arrst_delete(data->selected, pos, NULL, uint32_t);
                updated = TRUE;
            }
        }
    }
    else
    {
        if (n > 0)
        {
            if (i_is_only_selected(data->selected, rows[0]) == TRUE)
            {
                arrst_clear(data->selected, NULL, uint32_t);
                updated = TRUE;
            }
        }
    }

    if (updated == TRUE)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

void tableview_deselect_all(TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    bool_t updated = FALSE;
    cassert_no_null(data);

    if (arrst_size(data->selected, uint32_t) > 0)
    {
        arrst_clear(data->selected, NULL, uint32_t);
        updated = TRUE;
    }

    if (updated == TRUE)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

const ArrSt(uint32_t) *tableview_selected(const TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    return data->selected;
}
