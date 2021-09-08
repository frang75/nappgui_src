/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tableview.c
 *
 */

/* TableView */

#include "tableview.h"
#include "draw2d.inl"
#include "dctx.h"
#include "draw.h"
#include "draw.inl"
#include "drawctrl.inl"
//#include "header.inl"
#include "view.h"
#include "view.inl"
//#include "column.inl"
#include "component.inl"
#include "gui.inl"
#include "obj.inl"

#include "arrst.h"
//#include "bmath.h"
#include "cassert.h"
#include "color.h"
//#include "draw.h"
#include "event.h"
#include "font.h"
#include "heap.h"
//#include "log.h"
//#include "ptr.h"
#include "r2d.h"
#include "s2d.h"
//#include "v2d.h"
#include "strings.h"
#include "types.h"

typedef struct _tdata_t TData;
typedef struct _column_t Column;

struct _column_t
{
    ResId textid;
    String *text;
    real32_t width;
    align_t align;
};

struct _tdata_t
{
    View *cview;
    Font *font;
    uint32_t flags;
    real32_t row_height;
    real32_t total_width;
    uint32_t num_rows;
    uint32_t start_row;
    uint32_t end_row;
    real32_t offset_row;
    real32_t mouse_x;
    real32_t mouse_y;
    mouse_t mouse_button;
    bool_t mouse_scells;
    S2Df control_size;
    ArrSt(Column) *columns;
    Listener *OnNotify;
    uint32_t selected_row;
    real32_t padding_top;
    real32_t padding_bottom;
    real32_t padding_left;
    real32_t padding_right;
};

/*---------------------------------------------------------------------------*/

static void i_remove_column(Column *column)
{
    str_destroy(&column->text);
}

/*---------------------------------------------------------------------------*/

static TData *i_create_data(const uint32_t flags)
{
    TData *data = heap_new0(TData);
    data->font = font_system(font_regular_size(), 0);
    data->flags = flags;
    data->columns = arrst_create(Column);
    data->padding_top = 2;
    data->padding_bottom = 2;
    data->padding_left = 3;
    data->padding_right = 3;
    data->mouse_button = ENUM_MAX(mouse_t);
    data->selected_row = UINT32_MAX;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(TData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    font_destroy(&(*data)->font);
    listener_destroy(&(*data)->OnNotify);
    arrst_destroy(&(*data)->columns, i_remove_column, Column);
    heap_delete(data, TData);
}

/*---------------------------------------------------------------------------*/
#include "log.h"
static void i_OnDraw(TableView *view, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(view);

    data->mouse_scells = FALSE;

    draw_clear(p->ctx, kCOLOR_WHITE);
    //log_printf("Draw Visible Width:%.2f Height:%.2f", p->visible_width, p->visible_height);
    //log_printf("Draw Width:%.2f Height:%.2f", p->width, p->height);
    //draw_clear(p->ctx, color_rgb(200, 180, 180));

    if (data->OnNotify != NULL)
    {
        uint32_t mouse_row = UINT32_MAX;
        real32_t y;
        uint32_t i;

        data->start_row = (uint32_t)p->y / (uint32_t)data->row_height;
        data->end_row = (uint32_t)p->height / (uint32_t)data->row_height;
        data->end_row += data->start_row + 2;
        data->offset_row = p->y - data->start_row * data->row_height;
        data->end_row = min_u32(data->end_row, data->num_rows);
        y = - data->offset_row;

        //log_printf("Start: %d End: %d: Height: %.2f", data->start_row, data->end_row, p->visible_height);

        draw_font(p->ctx, data->font);

        if (data->flags & ekTBROWPRESEL)
        {
            mouse_row = (uint32_t)(data->mouse_y + data->offset_row) / (uint32_t)data->row_height;
            mouse_row += data->start_row;
        }

        for (i = data->start_row; i < data->end_row; ++i)
        {
            real32_t x = - p->x;
            EvTbPos params;
            EvTbRow result;
            params.col = UINT32_MAX;
            params.row = i;
            result.bgcolor = kCOLOR_TRANSPARENT;
            listener_event(data->OnNotify, ekEVTBLROW, view, &params, &result, TableView, EvTbPos, EvTbRow);
            unref(x);
            //if (i == data->selected_row)
            //{
            //    drawctrl_fill(p->ctx, x, y, data->total_width, data->row_height, ekCSTATE_PRESSED);
            //}
            //else if (i == mouse_row)
            //{
            //    drawctrl_fill(p->ctx, x, y, data->total_width, data->row_height, ekCSTATE_HOT);
            //}
            //else if (result.bgcolor != kCOLOR_TRANSPARENT)
            //{
            //    draw_fill_color(p->ctx, result.bgcolor);
            //    draw_rect_raster(p->ctx, ekFILL, x, y, data->total_width, data->row_height);
            //}
            //else
            //{
            //   // draw_bgselect(p->ctx, x, y, data->total_width, data->row_height, ekCSTATE_NORMAL);
            //}

            //arrst_foreach(col, data->columns, Column)
            //{
            //    // The column is visible
            //    if (x + col->width > 0 && x < p->width)
            //    {
            //        EvTbPos cparams;
            //        EvTbCell cresult;

            //        draw_text_width(p->ctx, col->width - data->padding_left - data->padding_right);
            //        cparams.col = col_i;
            //        cparams.row = i;
            //        cresult.flags = 0;
            //        cresult.checked = 0;
            //        cresult.text = NULL;
            //        cresult.bgcolor = kCOLOR_TRANSPARENT;
            //        listener_event(data->OnNotify, ekEVTBLCELL, view, &cparams, &cresult, TableView, EvTbPos, EvTbCell);

            //        if ((cresult.flags & ekTBBGCOLOR) 
            //            && cresult.bgcolor != kCOLOR_TRANSPARENT
            //            && i != mouse_row 
            //            && i != data->selected_row)
            //        {
            //            draw_fill_color(p->ctx, cresult.bgcolor);
            //            draw_rect_raster(p->ctx, ekFILL, x, y, col->width, data->row_height);
            //        }
            //        
            //        // TODO: Center in cells 
            //        if (cresult.flags & ekTBTEXT)
            //        {
            //            draw_text_raster(p->ctx, cresult.text, x + data->padding_left, y + data->padding_top);
            //        }

            //        if (cresult.flags & ekTBCHECK)
            //        {
            //            R2Df cell;
            //            cstate_t state = ekCSTATE_NORMAL;

            //            data->mouse_scells = TRUE;
            //            cell.pos.x = x;
            //            cell.pos.y = y;
            //            cell.size.width = col->width;
            //            cell.size.height = data->row_height;

            //            if (r2d_containsf(&cell, data->mouse_x, data->mouse_y) == TRUE)
            //            {
            //                if (data->mouse_button == ekMLEFT )
            //                    state = ekCSTATE_PRESSED;
            //                else
            //                    state = ekCSTATE_HOT;
            //            }

            //            if (cresult.checked == TRUE)
            //                draw_checkbox(p->ctx, x + data->padding_left, y + data->padding_top, state);
            //            else
            //                draw_uncheckbox(p->ctx, x + data->padding_left, y + data->padding_top, state);
            //        }

            //    }

            //    x += col->width;
            //}
            //arrst_end();

            y += data->row_height;
        }
    }
}

/*---------------------------------------------------------------------------*/

//static void i_OnRedraw(TableView *view, Event *event)
//{
//    GuiContext *context = NULL;
//    TableViewData *data = NULL;
//    const EvDraw *params = NULL;
//    real32_t head_height, table_height;
//    real32_t start_y;
//    uint32_t start_row, end_row, total_rows;
//
//    log_printf("Table Redraw");
//    context = _view_context((CView*)view);
//    cassert_no_null(context);
//    data = view_data((CView*)view, TableViewData);
//    cassert_no_null(data);
//    params = event_params(event, EvDraw);
//    cassert_no_null(params);
//    head_height = header_height(data->_header);
//    table_height = params->height - head_height;
//    header_draw(data->_header, params->context, 0.f, 0.f, - data->hscroll_pos);
//
//    draw_fill_color(params->context, color_rgb(255, 255, 255));
//    draw_rect(params->context, ekFILL, 0.f, head_height, params->width, table_height);
//
//    start_row = (uint32_t)bmath_floorf(data->vscroll_pos / data->row_height);
//    start_y = - bmath_modf(data->vscroll_pos, data->row_height);
//    cassert(start_y > - data->row_height && start_y <= 0.f);
//    total_rows = (uint32_t)bmath_ceilf(table_height / data->row_height);
//    if (start_y + (real32_t)total_rows * data->row_height < table_height)
//        total_rows += 1;
//    end_row = start_row + total_rows;
//
//    if (end_row > data->num_rows)
//        end_row = data->num_rows;
//
//    start_y += head_height;
//    for (; start_row < end_row; ++start_row)
//    {
//        cassert(start_y < params->height);
//        draw_line_color(params->context, color_rgb(0,0,0));
//        draw_rect(params->context, ekSTROKE, 0.f, start_y, params->width - 1.f, data->row_height - 1.f);
//        start_y += data->row_height;
//    }
//}

/*---------------------------------------------------------------------------*/

static real32_t i_row_height(const TData *data)
{
    real32_t height = 0;
    cassert_no_null(data);
    if (data->flags & ekTBTEXT)
        height = max_r32(height, font_height(data->font));

    if (data->flags & ekTBICON16)
        height = max_r32(height, 16);

    if (data->flags & ekTBCHECK)
    {
        height = max_r32(height, 16/*kCHECK_HEIGHT*/);
    }

    return height + data->padding_top + data->padding_bottom;
}

/*---------------------------------------------------------------------------*/

static void i_document_size(TableView *view, TData *data)
{
    real32_t twidth = 0;
    cassert_no_null(data);
    arrst_foreach(col, data->columns, Column)
        twidth += col->width;
    arrst_end();

    data->row_height = i_row_height(data);
    data->total_width = twidth;
    view_content_size((View*)view, s2df(twidth, data->row_height * data->num_rows), s2df(10, data->row_height));
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    if (data->OnNotify != NULL)
    {
        const EvSize *p = event_params(e, EvSize);
        listener_event(data->OnNotify, ekEVTBLSIZE, view, p, NULL, TableView, EvSize, void);
    }

    i_document_size(view, data);
}

/*---------------------------------------------------------------------------*/

static void i_OnMove(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvMouse *p = event_params(e, EvMouse);
    data->mouse_x = p->x;
    data->mouse_y = p->y;

    //log_printf("Mouse: %.2f, %.2f", p->x, p->y);

    if (data->flags & ekTBROWPRESEL || data->mouse_scells)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    data->mouse_x = 1e8f;
    data->mouse_y = 1e8f;

    //log_printf("Mouse: %.2f, %.2f", p->x, p->y);

    if (data->flags & ekTBROWPRESEL || data->mouse_scells)
        view_update((View*)view);

    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    const EvMouse *p = event_params(e, EvMouse);
    data->mouse_button = p->button;

    if (data->mouse_scells)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static void i_OnUp(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    data->mouse_button = ENUM_MAX(mouse_t);
    unref(e);

    if (data->mouse_scells)
        view_update((View*)view);
}

/*---------------------------------------------------------------------------*/

static void i_OnNotify(TableView *view, Event *e)
{
    TData *data = view_get_data((View*)view, TData);
    if (event_type(e) == ekEVHEADSIZE)
    {
        bool_t ok = TRUE;
        if (data->OnNotify != NULL)
        {
            bool_t *result = event_result(e, bool_t);
            listener_pass_event(data->OnNotify, e, view, TableView);
            ok = *result;
        }

        if (ok == TRUE)
        {
            const EvHeader *p = event_params(e, EvHeader);
            Column *col = arrst_get(data->columns, p->index, Column);
            col->width = p->width;
            i_document_size(view, data);
            view_update((View*)view);
        }
    }
    else
    {
        if (data->OnNotify != NULL)
            listener_pass_event(data->OnNotify, e, view, TableView);
    }
}

/*---------------------------------------------------------------------------*/

//static void i_OnHScroll(TableView *view, Event *event)
//{
//    TableViewData *data = NULL;
//    const EvScroll *params = NULL;
//    data = view_data((CView*)view, TableViewData);
//    cassert_no_null(data);
//    params = event_params(event, EvScroll);
//    cassert_no_null(params);
//    data->hscroll_pos = params->pos;
//    log_printf("HScroll: %f", params->pos);
//    view_update(data->cview);
//}

/*---------------------------------------------------------------------------*/

//static void i_OnVScroll(TableView *view, Event *event)
//{
//    TableViewData *data = NULL;
//    const EvScroll *params = NULL;
//    data = view_data((CView*)view, TableViewData);
//    cassert_no_null(data);
//    params = event_params(event, EvScroll);
//    cassert_no_null(params);
//    data->vscroll_pos = params->pos;
//    log_printf("VScroll: %.2f", params->pos);
//    view_update(data->cview);
//}

/*---------------------------------------------------------------------------*/

//static void i_OnWheel(TableView *view, Event *event)
//{
//    const EvWheel *params = event_params(event, EvWheel);
//    cassert_no_null(params);
//    cassert_no_null(view);
//    log_printf("Wheel Dx:%.2f Dy:%.2f", params->dx, params->dy);
//}

/*---------------------------------------------------------------------------*/

static void i_add_column(TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    //GuiContext *ctx = _view_context((View*)view);
    //void *ositem = _view_get_native_imp((View*)view);
    Column *col = NULL;
    ColItem citem;
    cassert_no_null(data);
    col = arrst_new(data->columns, Column);
    col->textid = NULL;
    col->text = str_c("Column");
    col->align = ekLEFT;
    col->width = 100;

    //switch(type) {
    //case ekCOLUMN_TEXT:
    //    col->align = ekLEFT;
    //    col->width = 100;
    //    break;
    //case ekCOLUMN_ICON:
    //case ekCOLUMN_EDIT:
    //case ekCOLUMN_POPUP:
    //case ekCOLUMN_RADIO:
    //case ekCOLUMN_CHECK:
    //cassert_default();
    //}

    citem.align = col->align;
    citem.index = arrst_size(data->columns, Column) - 1;
    citem.op = ekOPINS;
    citem.width = col->width;
    citem.text = tc(col->text);
    //ctx->func_view_header_column(ositem, &citem);
    unref(citem);
}

/*---------------------------------------------------------------------------*/

TableView *tableview_create(const uint32_t num_cols, const uint32_t flags)
{
    View *view = _view_create(ekHSCROLL | ekVSCROLL | /*ekHEADER |*/ ekBORDER | ekNOERASE);
    TData *data = i_create_data(flags != 0 ? flags : ekTBTEXT);
    uint32_t i;
    view_data(view, &data, i_destroy_data, TData);
    view_OnDraw(view, listener((TableView*)view, i_OnDraw, TableView));
    view_OnSize(view, listener((TableView*)view, i_OnSize, TableView));
    view_OnMove(view, listener((TableView*)view, i_OnMove, TableView));
    view_OnExit(view, listener((TableView*)view, i_OnExit, TableView));
    view_OnDown(view, listener((TableView*)view, i_OnDown, TableView));
    view_OnUp(view, listener((TableView*)view, i_OnUp, TableView));
    view_OnNotify(view, listener((TableView*)view, i_OnNotify, TableView));

    for (i = 0; i < num_cols; ++i)
        i_add_column((TableView*)view);


    //TableViewData *data = NULL;
    //CView *view = NULL;
    //
    //data = i_create_data();
    //view = view_create_data(ekCVDRAW, data, i_destroy_data, TableViewData);
    //data->cview = view;

    //{
    //    void *view_imp = _view_get_native_imp(view);
    //    const GuiContext *context = _view_context(view);
    //    real32_t unused, thickness;

    //    data->hscroll = context->func_scroller_create(ekHORIZONTAL, 100.f, ekREGULAR);
    //    context->func_scroller_OnMoved(data->hscroll, listener((TableView*)view, i_OnHScroll, TableView));
    //    context->func_attach_to_panel[ekGUI_COMPONENT_SCROLLER](data->hscroll, view_imp);
    //    context->func_get_size[ekGUI_COMPONENT_SCROLLER](data->hscroll, &unused, &data->scroll_thickness);

    //    data->vscroll = context->func_scroller_create(ekVERTICAL, 100.f, ekREGULAR);
    //    context->func_scroller_OnMoved(data->vscroll, listener((TableView*)view, i_OnVScroll, TableView));
    //    context->func_attach_to_panel[ekGUI_COMPONENT_SCROLLER](data->vscroll, view_imp);
    //    context->func_get_size[ekGUI_COMPONENT_SCROLLER](data->vscroll, &thickness, &unused);
    //    cassert_unref(thickness == data->scroll_thickness, thickness);
    //}

    //view_OnRedrawArea(view, listener((TableView*)view, i_OnRedrawArea, TableView));
    //view_OnWheel(view, listener((TableView*)view, i_OnWheel, TableView));

    return (TableView*)view;
}

/*---------------------------------------------------------------------------*/

void tableview_OnNotify(TableView *view, Listener *listener)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    listener_update(&data->OnNotify, listener);
}

/*---------------------------------------------------------------------------*/

void tableview_size(TableView *view, S2Df size)
{
    view_size((View*)view, size);
}

/*---------------------------------------------------------------------------*/

void tableview_ctext(TableView *view, const uint32_t id, const char_t *text)
{
    TData *data = view_get_data((View*)view, TData);
    Column *col = arrst_get(data->columns, id, Column);
    const char_t *ltext = _gui_respack_text(text, &col->textid);
    //GuiContext *ctx = _view_context((View*)view);
    //void *ositem = _view_get_native_imp((View*)view);
    ColItem citem;
    str_upd(&col->text, ltext);
    citem.op = ekOPSET;
    citem.index = id;
    citem.text = tc(col->text);
    citem.align = ENUM_MAX(align_t);
    citem.width = -1;
    //ctx->func_view_header_column(ositem, &citem);
    unref(citem);
}

/*---------------------------------------------------------------------------*/

void tableview_cwidth(TableView *view, const uint32_t id, const real32_t width)
{
    TData *data = view_get_data((View*)view, TData);
    Column *col = arrst_get(data->columns, id, Column);
    //GuiContext *ctx = _view_context((View*)view);
    //void *ositem = _view_get_native_imp((View*)view);
    ColItem citem;
    col->width = width;
    citem.op = ekOPSET;
    citem.index = id;
    citem.text = NULL;
    citem.align = ENUM_MAX(align_t);
    citem.width = width;
    //ctx->func_view_header_column(ositem, &citem);
    i_document_size(view, data);
    unref(citem);
}

/*---------------------------------------------------------------------------*/

void tableview_calign(TableView *view, const uint32_t id, const align_t align)
{
    TData *data = view_get_data((View*)view, TData);
    Column *col = arrst_get(data->columns, id, Column);
    //GuiContext *ctx = _view_context((View*)view);
    //void *ositem = _view_get_native_imp((View*)view);
    ColItem citem;
    col->align = align;
    citem.op = ekOPSET;
    citem.index = id;
    citem.text = NULL;
    citem.align = align;
    citem.width = -1;
    //ctx->func_view_header_column(ositem, &citem);
    unref(citem);
}

/*---------------------------------------------------------------------------*/

real32_t tableview_get_cwidth(TableView *view, const uint32_t id)
{
    TData *data = view_get_data((View*)view, TData);
    Column *col = arrst_get(data->columns, id, Column);
    return col->width;
}

/*---------------------------------------------------------------------------*/

void tableview_get_size(const TableView *view, S2Df *size)
{
    view_get_size((const View*)view, size);
}

/*---------------------------------------------------------------------------*/

void tableview_update(TableView *view)
{
    TData *data = view_get_data((View*)view, TData);
    cassert_no_null(data);
    if (data->OnNotify != NULL)
    {
        listener_event(data->OnNotify, ekEVTBLNROWS, view, NULL, &data->num_rows, TableView, void, uint32_t);
        i_document_size(view, data);
        view_update((View*)view);
    }
}

/*---------------------------------------------------------------------------*/

//void tableview_set_header_font(TableView *view, const Font *font)
//{
//    unref(view);
//    unref(font);
//    //GuiContext *context = _view_context((CView*)view);
//    //TableViewData *data = view_data((CView*)view, TableViewData);
//    //cassert_no_null(data);
//    //_gui_update_font(&data->header_font, NULL, font);
//    //context->func_header_set_font(data->header, font_imp(data->header_font));
//    //BIT_CLEAR(data->control_flags, HEADER_HEIGHT_UPDATED);
//    ////context->func_set_visible[ekGUI_COMPONENT_HEADER](header, TRUE);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_header_text(TableView *view, const uint32_t column_id, const char_t *text)
//{
//    GuiContext *context = NULL;
//    TableViewData *data = NULL;
//    Column *column = NULL;
//    context = _view_context((View*)view);
//    cassert_no_null(context);
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    column = arrpt_get(data->columns, column_id, Column);
//    _column_set_header_text(column, text);
//    header_set_text(data->_header, column_id, text);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_header_align(TableView *view, const uint32_t column_id, const align_t align)
//{
//    GuiContext *context = NULL;
//    TableViewData *data = NULL;
//    context = _view_context((View*)view);
//    cassert_no_null(context);
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    header_set_align(data->_header, column_id, align);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_column_width(TableView *view, const uint32_t column_id, const real32_t width)
//{
//    GuiContext *context = NULL;
//    TableViewData *data = NULL;
//    Column *column = NULL;
//    context = _view_context((View*)view);
//    cassert_no_null(context);
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    column = arrpt_get(data->columns, column_id, Column);
//    _column_set_width(column, width);
//    header_set_width(data->_header, column_id, width);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_column_font(TableView *view, const uint32_t column_id, const Font *font)
//{
//    TableViewData *data = NULL;
//    Column *column = NULL;
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    column = arrpt_get(data->columns, column_id, Column);
//    _column_set_font(column, font);
//    BIT_CLEAR(data->control_flags, ROW_HEIGHT_UPDATED);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_column_align(TableView *view, const uint32_t column_id, const align_t align)
//{
//    TableViewData *data = NULL;
//    Column *column = NULL;
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    column = arrpt_get(data->columns, column_id, Column);
//    _column_set_align(column, align);
//}

/*---------------------------------------------------------------------------*/

//void tableview_set_column_content_size(TableView *view, const uint32_t column_id, const S2Df *size)
//{
//    TableViewData *data = NULL;
//    Column *column = NULL;
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    column = arrpt_get(data->columns, column_id, Column);
//    _column_set_content_size(column, size);
//    BIT_CLEAR(data->control_flags, ROW_HEIGHT_UPDATED);
//}

/*---------------------------------------------------------------------------*/

//void tableview_OnGetNumRows(TableView *view, Listener *listener)
//{
//    TableViewData *data = NULL;
//    data = view_data((View*)view, TableViewData);
//    cassert_no_null(data);
//    listener_update(&data->OnGetNumRows_listener, listener);
//}
