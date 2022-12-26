/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawbig.c
 *
 */

/* Drawing a big area with scrollbars */

#include "nappgui.h"

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    View *view;
    Label *label;
    uint32_t col_id;
    uint32_t row_id;
    uint32_t margin;
    uint32_t mouse_cell_x;
    uint32_t mouse_cell_y;
    uint32_t sel_cell_x;
    uint32_t sel_cell_y;
    bool_t focus;
};

static const uint32_t i_NUM_COLS = 2000;
static const uint32_t i_NUM_ROWS = 2000;
static const real32_t i_CELL_SIZE = 50;

/*---------------------------------------------------------------------------*/

static void i_dbind(void)
{
    dbind(App, uint32_t, col_id);
    dbind(App, uint32_t, row_id);
    dbind(App, uint32_t, margin);
    dbind_range(App, uint32_t, col_id, 0, i_NUM_COLS - 1);
    dbind_range(App, uint32_t, row_id, 0, i_NUM_ROWS - 1);
    dbind_range(App, uint32_t, margin, 10, 50);
}

/*---------------------------------------------------------------------------*/

static void i_content_size(App *app)
{
    real32_t width = i_NUM_COLS * i_CELL_SIZE + (i_NUM_COLS + 1) * app->margin;
    real32_t height = i_NUM_ROWS * i_CELL_SIZE + (i_NUM_ROWS + 1) * app->margin;
    view_content_size(app->view, s2df((real32_t)width, (real32_t)height), s2df(10, 10));
}

/*---------------------------------------------------------------------------*/

static void i_scroll_to_cell(App *app)
{
    real32_t xpos = app->col_id * i_CELL_SIZE + (app->col_id + 1) * app->margin;
    real32_t ypos = app->row_id * i_CELL_SIZE + (app->row_id + 1) * app->margin;
    xpos -= 5;
    ypos -= 5;
    view_scroll_x(app->view, xpos);
    view_scroll_y(app->view, ypos);
}

/*---------------------------------------------------------------------------*/

static void i_draw_clipped(App *app, DCtx *ctx, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    register uint32_t sti, edi;
    register uint32_t stj, edj;
    real32_t cellsize = i_CELL_SIZE + (real32_t)app->margin;
    real32_t hcell = i_CELL_SIZE / 2;
    register real32_t posx = 0;
    register real32_t posy = 0;
    register uint32_t i, j;

    /* Calculate the visible cols */
    sti = (uint32_t)bmath_floorf(x / cellsize);
    edi = sti + (uint32_t)bmath_ceilf(width / cellsize) + 1;
    if (edi > i_NUM_COLS)
        edi = i_NUM_COLS;

    /* Calculate the visible rows */
    stj = (uint32_t)bmath_floorf(y / cellsize);
    edj = stj + (uint32_t)bmath_ceilf(height / cellsize) + 1;
    if (edj > i_NUM_ROWS)
        edj = i_NUM_ROWS;

    posy = (real32_t)app->margin + stj * cellsize;

    {
        char_t text[256];
        bstd_sprintf(text, sizeof(text), "Real draw cols: [%d - %d], rows: [%d - %d]", sti, edi, stj, edj);
        label_text(app->label, text);
    }

    draw_fill_color(ctx, color_gray(240));
    draw_rect(ctx, ekFILL, x, y, width, height);
    draw_fill_color(ctx, color_gray(200));
    draw_line_color(ctx, kCOLOR_BLUE);
    draw_line_width(ctx, 1);
    draw_text_align(ctx, ekCENTER, ekCENTER);
    draw_text_halign(ctx, ekCENTER);

    for (j = stj; j < edj; ++j)
    {
        posx = (real32_t)app->margin + sti * cellsize;
        for (i = sti; i < edi; ++i)
        {
            char_t text[128];
            bool_t special_cell = FALSE;

            bstd_sprintf(text, sizeof(text), "%d\n%d", i, j);

            if (app->sel_cell_x == i && app->sel_cell_y == j)
            {
                draw_line_width(ctx, 6);
                if (app->focus == TRUE)
                    draw_line_color(ctx, kCOLOR_RED);
                else
                    draw_line_color(ctx, color_gray(100));

                special_cell = TRUE;
            }
            else if (app->mouse_cell_x == i && app->mouse_cell_y == j)
            {
                draw_line_width(ctx, 3);
                draw_line_color(ctx, kCOLOR_BLUE);
                special_cell = TRUE;
            }

            draw_rect(ctx, ekSKFILL, posx, posy, i_CELL_SIZE, i_CELL_SIZE);
            draw_text(ctx, text, posx + hcell, posy + hcell);

            if (special_cell == TRUE)
            {
                draw_line_width(ctx, 1);
                draw_line_color(ctx, kCOLOR_BLUE);
            }

            posx += cellsize;
        }

        posy += cellsize;
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    i_draw_clipped(app, p->ctx, p->x, p->y, p->width, p->height);
}

/*---------------------------------------------------------------------------*/

static void i_mouse_cell(App *app, const real32_t x, const real32_t y, const uint32_t action)
{
    real32_t cellsize = i_CELL_SIZE + (real32_t)app->margin;
    uint32_t mx = (uint32_t)bmath_floorf(x / cellsize);
    uint32_t my = (uint32_t)bmath_floorf(y / cellsize);
    real32_t xmin = mx * cellsize + (real32_t)app->margin;
    real32_t xmax = xmin + i_CELL_SIZE;
    real32_t ymin = my * cellsize + (real32_t)app->margin;
    real32_t ymax = ymin + i_CELL_SIZE;

    if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
    {
        if (action == 0)
        {
            app->mouse_cell_x = mx;
            app->mouse_cell_y = my;
        }
        else
        {
            app->sel_cell_x = mx;
            app->sel_cell_y = my;
        }
    }
    else
    {
        app->mouse_cell_x = UINT32_MAX;
        app->mouse_cell_y = UINT32_MAX;
    }

    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnMove(App *app, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    i_mouse_cell(app, p->x, p->y, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnUp(App *app, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    i_mouse_cell(app, p->x, p->y, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(App *app, Event *e)
{
    const EvMouse *p = event_params(e, EvMouse);
    i_mouse_cell(app, p->x, p->y, 1);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(App *app, Event *e)
{
    const bool_t *p = event_params(e, bool_t);
    app->focus = *p;
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDown(App *app, Event *e)
{
    const EvKey *p = event_params(e, EvKey);
    View *view = event_sender(e, View);
    real32_t margin = (real32_t)app->margin;
    real32_t cellsize = i_CELL_SIZE + margin;
    V2Df scroll;
    S2Df size;

    view_viewport(view, &scroll, &size);

    if (p->key == ekKEY_DOWN && app->sel_cell_y < i_NUM_ROWS - 1)
    {
        real32_t ymin = (app->sel_cell_y + 1) * cellsize + margin;
        ymin += i_CELL_SIZE;

        if (scroll.y + size.height <= ymin)
        {
            view_scroll_y(view, ymin - size.height + margin);
            app->mouse_cell_x = UINT32_MAX;
            app->mouse_cell_y = UINT32_MAX;
        }

        app->sel_cell_y += 1;
        view_update(app->view);
    }

    if (p->key == ekKEY_UP && app->sel_cell_y > 0)
    {
        real32_t ymin = (app->sel_cell_y - 1) * cellsize + (real32_t)app->margin;

        if (scroll.y >= ymin)
        {
            view_scroll_y(view, ymin - margin);
            app->mouse_cell_x = UINT32_MAX;
            app->mouse_cell_y = UINT32_MAX;
        }

        app->sel_cell_y -= 1;
        view_update(app->view);
    }

    if (p->key == ekKEY_RIGHT && app->sel_cell_x < i_NUM_COLS - 1)
    {
        real32_t xmin = (app->sel_cell_x + 1) * cellsize + margin;
        xmin += i_CELL_SIZE;

        if (scroll.x + size.width <= xmin)
        {
            view_scroll_x(view, xmin - size.width + margin);
            app->mouse_cell_x = UINT32_MAX;
            app->mouse_cell_y = UINT32_MAX;
        }

        app->sel_cell_x += 1;
        view_update(app->view);
    }

    if (p->key == ekKEY_LEFT && app->sel_cell_x > 0)
    {
        real32_t xmin = (app->sel_cell_x - 1) * cellsize + (real32_t)app->margin;

        if (scroll.x >= xmin)
        {
            view_scroll_x(view, xmin - margin);
            app->mouse_cell_x = UINT32_MAX;
            app->mouse_cell_y = UINT32_MAX;
        }

        app->sel_cell_x -= 1;
        view_update(app->view);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDataChange(App *app, Event *e)
{
    unref(e);
    i_scroll_to_cell(app);
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(6, 1);
    Layout *layout2 = layout_create(1, 3);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Slider *slider = slider_create();
    View *view = view_scroll();
    label_text(label1, "Goto column:");
    label_text(label2, "Goto row:");
    label_text(label3, "Margin:");
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    view_size(view, s2df(256, 256));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    view_OnMove(view, listener(app, i_OnMove, App));
    view_OnUp(view, listener(app, i_OnUp, App));
    view_OnDown(view, listener(app, i_OnDown, App));
    view_OnFocus(view, listener(app, i_OnFocus, App));
    view_OnKeyDown(view, listener(app, i_OnKeyDown, App));
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 2, 0);
    layout_label(layout1, label3, 4, 0);
    layout_edit(layout1, edit1, 1, 0);
    layout_edit(layout1, edit2, 3, 0);
    layout_slider(layout1, slider, 5, 0);
    layout_layout(layout2, layout1, 0, 0);
    layout_label(layout2, label4, 0, 1);
    layout_view(layout2, view, 0, 2);
    layout_tabstop(layout2, 0, 2, TRUE);
    layout_margin2(layout1, 0, 5);
    layout_hmargin(layout1, 0, 5);
    layout_hmargin(layout1, 1, 10);
    layout_hmargin(layout1, 2, 5);
    layout_hmargin(layout1, 3, 10);
    layout_hmargin(layout1, 4, 5);
    layout_vmargin(layout2, 0, 5);
    layout_vmargin(layout2, 1, 5);
    layout_halign(layout2, 0, 0, ekLEFT);
    layout_halign(layout2, 0, 1, ekJUSTIFY);
    layout_vexpand(layout2, 2);
    cell_padding2(layout_cell(layout2, 0, 1), 0, 5);
    cell_dbind(layout_cell(layout1, 1, 0), App, uint32_t, col_id);
    cell_dbind(layout_cell(layout1, 3, 0), App, uint32_t, row_id);
    cell_dbind(layout_cell(layout1, 5, 0), App, uint32_t, margin);
    layout_dbind(layout2, listener(app, i_OnDataChange, App), App);
    layout_dbind_obj(layout2, app, App);
    panel_layout(panel, layout2);
    app->view = view;
    app->label = label4;
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    Panel *panel = NULL;
    i_dbind();
    app->col_id = 50;
    app->row_id = 50;
    app->margin = 10;
    app->mouse_cell_x = UINT32_MAX;
    app->mouse_cell_y = UINT32_MAX;
    app->sel_cell_x = app->col_id;
    app->sel_cell_y = app->row_id;
    app->focus = FALSE;
    panel = i_panel(app);
    app->window = window_create(ekWINDOW_STDRES);
    i_content_size(app);
    window_panel(app->window, panel);
    window_title(app->window, "Big drawing area");
    window_origin(app->window, v2df(500, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    i_scroll_to_cell(app);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include "osmain.h"
osmain(i_create, i_destroy, "", App)
