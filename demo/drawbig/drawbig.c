/* Drawing a big area with scrollbars */

#include <nappgui.h>

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    Window *flyout;
    Panel *panel;
    ListBox *list1;
    ListBox *list2;
    ListBox *list3;
    TableView *table;
    Edit *edit1;
    Edit *edit2;
    View *view;
    View *fullview;
    Label *cells_label;
    Label *status_label;
    Progress *progress;
    uint32_t col_id;
    uint32_t row_id;
    uint32_t margin;
    uint32_t mouse_cell_x;
    uint32_t mouse_cell_y;
    uint32_t sel_cell_x;
    uint32_t sel_cell_y;
    bool_t focus;
    bool_t overlay;
    Layout *main_layout;
    Layout *middle_layout;
    Layout *control_layout;
    Layout *info_layout;
    char_t temptxt[256];
    uint32_t anim_frame;
    uint32_t anim_total;
    Font *fullfont;
    color_t drawcolor;
    color_t backcolor;
};

static const uint32_t i_NUM_COLS = 2000;
static const uint32_t i_NUM_ROWS = 2000;
static const real32_t i_CELL_SIZE = 50;
static const char_t *i_CELLS_INFO = "Draw cells: [%d, %d] x [%d, %d]";
static const char_t *i_STATUS_TEXT = "Application status...";

/*---------------------------------------------------------------------------*/

static void i_dbind(void)
{
    dbind(App, uint32_t, col_id);
    dbind(App, uint32_t, row_id);
    dbind(App, uint32_t, margin);
    dbind(App, bool_t, overlay);
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
    uint32_t sti, edi;
    uint32_t stj, edj;
    real32_t cellsize = i_CELL_SIZE + (real32_t)app->margin;
    real32_t hcell = i_CELL_SIZE / 2;
    real32_t posx = 0;
    real32_t posy = 0;
    uint32_t i, j;

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
        bstd_sprintf(text, sizeof(text), i_CELLS_INFO, sti, stj, edi, edj);
        label_text(app->cells_label, text);
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

static void i_OnOverlay(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    cassert_no_null(app);
    if (app->overlay == TRUE)
    {
        draw_fill_color(p->ctx, kCOLOR_BLACK);
        draw_text_color(p->ctx, kCOLOR_WHITE);
        draw_rect(p->ctx, ekFILL, 5, 5, 80, 20);
        draw_text(p->ctx, "OVERLAY", 5, 5);
    }
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

    if (p->key == ekKEY_RETURN)
    {
        panel_visible_layout(app->panel, 1);
        panel_update(app->panel);
        window_focus(app->window, guicontrol(app->fullview));
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnDataChange(App *app, Event *e)
{
    unref(e);

    /* If col/row editbox are changed, change the focused cell in view */
    if (evbind_modify(e, App, uint32_t, col_id) == TRUE || evbind_modify(e, App, uint32_t, row_id) == TRUE)
    {
        app->sel_cell_x = app->col_id;
        app->sel_cell_y = app->row_id;
    }

    i_scroll_to_cell(app);
    view_update(app->view);
}

/*---------------------------------------------------------------------------*/

static void i_flyout_over_control(App *app, GuiControl *control, const uint32_t align)
{
    /* Control bounds in window coordinates */
    R2Df frame = window_control_frame(app->window, control);
    /* Top-Left control in screen coordinates */
    V2Df pos = window_client_to_screen(app->window, frame.pos);
    /* Flyout window size */
    S2Df size = window_get_size(app->flyout);

    switch (align)
    {
    case 0:
        break;
    case 1:
        pos.y += (frame.size.height - size.height);
        break;
    case 2:
        pos.x += (frame.size.width - size.width);
        break;
    case 3:
        pos.x += (frame.size.width - size.width);
        pos.y += (frame.size.height - size.height);
        break;
    }

    /* Position in screen coordinates */
    window_origin(app->flyout, pos);
    window_overlay(app->flyout, app->window);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_listbox_sel(ListBox *list)
{
    uint32_t i, n = listbox_count(list);
    for (i = 0; i < n; ++i)
    {
        if (listbox_selected(list, i) == TRUE)
            return i;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_OnIdleLaunch(App *app, Event *e)
{
    GuiControl *control = guicontrol(app->list1);
    uint32_t selctrl = i_listbox_sel(app->list1);
    uint32_t selalign = i_listbox_sel(app->list2);
    switch (selctrl)
    {
    case 0:
        control = guicontrol(app->list1);
        break;
    case 1:
        control = guicontrol(app->list2);
        break;
    case 2:
        control = guicontrol(app->list3);
        break;
    case 3:
        control = guicontrol(app->table);
        break;
    case 4:
        control = guicontrol(app->view);
        break;
    case 5:
        control = guicontrol(app->edit1);
        break;
    case 6:
        control = guicontrol(app->edit2);
        break;
    }

    i_flyout_over_control(app, control, selalign);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnFlyoutClick(App *app, Event *e)
{
    gui_OnIdle(listener(app, i_OnIdleLaunch, App));
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnColored(App *app, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    if (p->state == ekGUI_ON)
    {
        layout_bgcolor(app->main_layout, color_rgb(128, 0, 0));
        layout_bgcolor(app->middle_layout, color_rgb(0, 128, 0));
        layout_bgcolor(app->control_layout, color_rgb(0, 0, 128));
        layout_bgcolor(app->info_layout, color_rgb(128, 128, 0));
    }
    else
    {
        layout_bgcolor(app->main_layout, kCOLOR_DEFAULT);
        layout_bgcolor(app->middle_layout, kCOLOR_DEFAULT);
        layout_bgcolor(app->control_layout, kCOLOR_DEFAULT);
        layout_bgcolor(app->info_layout, kCOLOR_DEFAULT);
    }

    panel_update(app->panel);
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout(App *app)
{
    Layout *layout = layout_create(10, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Slider *slider = slider_create();
    Button *button1 = button_check();
    Button *button2 = button_push();
    Button *button3 = button_check();
    label_text(label1, "Goto Col:");
    label_text(label2, "Row:");
    label_text(label3, "Margin:");
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    button_text(button1, "Draw overlay");
    button_text(button2, "Flyout");
    button_text(button3, "Colored");
    button_tooltip(button1, "Draw a fixed overlay on top of scrolled view");
    button_tooltip(button2, "Display a flyout window over the control selected in list-1, with alignment selected in list-2");
    button_tooltip(button3, "Enable/disable the layout colouring");
    button_OnClick(button2, listener(app, i_OnFlyoutClick, App));
    button_OnClick(button3, listener(app, i_OnColored, App));
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 2, 0);
    layout_label(layout, label3, 4, 0);
    layout_edit(layout, edit1, 1, 0);
    layout_edit(layout, edit2, 3, 0);
    layout_slider(layout, slider, 5, 0);
    layout_button(layout, button1, 6, 0);
    layout_button(layout, button2, 7, 0);
    layout_button(layout, button3, 9, 0);

    /* Force the width of editbox columns */
    layout_hsize(layout, 1, 50);
    layout_hsize(layout, 3, 50);

    /* Horizontal margins between controls */
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 2, 5);
    layout_hmargin(layout, 3, 5);
    layout_hmargin(layout, 4, 5);
    layout_hmargin(layout, 5, 5);
    layout_hmargin(layout, 6, 5);
    layout_hmargin(layout, 7, 5);

    /* All the horizontal expansion will be done in an empty cell between
     * 'Colored' checkbox and 'Flyout' button */
    layout_hexpand(layout, 8);

    /* Data binding */
    cell_dbind(layout_cell(layout, 1, 0), App, uint32_t, col_id);
    cell_dbind(layout_cell(layout, 3, 0), App, uint32_t, row_id);
    cell_dbind(layout_cell(layout, 5, 0), App, uint32_t, margin);
    cell_dbind(layout_cell(layout, 6, 0), App, bool_t, overlay);

    app->edit1 = edit1;
    app->edit2 = edit2;
    app->control_layout = layout;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_info_layout(App *app)
{
    Layout *layout = layout_create(4, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Progress *progress = progress_create();
    char_t text[256];

    label_align(label2, ekRIGHT);

    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 3, 0);
    layout_progress(layout, progress, 1, 0);
    layout_hmargin(layout, 0, 10);

    /* All the horizontal expansion will be done in empty column-cell(2) */
    layout_hexpand(layout, 2);

    /* Keep the labels for futher updates */
    app->status_label = label1;
    app->cells_label = label2;

    /* Text for labels dimensioning */
    bstd_sprintf(text, sizeof(text), i_CELLS_INFO, 1000, 1000, 1000, 1000);
    label_size_text(app->cells_label, text);
    label_size_text(app->status_label, i_STATUS_TEXT);
    label_text(app->status_label, i_STATUS_TEXT);
    app->info_layout = layout;
    app->progress = progress;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_list_layout(App *app)
{
    Layout *layout = layout_create(1, 6);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    ListBox *list1 = listbox_create();
    ListBox *list2 = listbox_create();
    ListBox *list3 = listbox_create();
    label_text(label1, "Flyout control");
    label_text(label2, "Flyout align");
    label_text(label3, "List-3");
    listbox_add_elem(list1, "List 1", NULL);
    listbox_add_elem(list1, "List 2", NULL);
    listbox_add_elem(list1, "List 3", NULL);
    listbox_add_elem(list1, "Table", NULL);
    listbox_add_elem(list1, "View", NULL);
    listbox_add_elem(list1, "Edit 1", NULL);
    listbox_add_elem(list1, "Edit 2", NULL);
    listbox_add_elem(list2, "Left-Top", NULL);
    listbox_add_elem(list2, "Left-Bottom", NULL);
    listbox_add_elem(list2, "Right-Top", NULL);
    listbox_add_elem(list2, "Right-Bottom", NULL);
    listbox_add_elem(list3, "Item 1", NULL);
    listbox_add_elem(list3, "Item 2", NULL);
    listbox_add_elem(list3, "Item 3", NULL);
    listbox_add_elem(list3, "Item 4", NULL);
    listbox_select(list1, 0, TRUE);
    listbox_select(list2, 0, TRUE);
    listbox_select(list3, 0, TRUE);

    /* Natural size of listboxes */
    listbox_size(list1, s2df(150, 100));
    listbox_size(list2, s2df(150, 100));
    listbox_size(list3, s2df(150, 100));

    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 2);
    layout_label(layout, label3, 0, 4);
    layout_listbox(layout, list1, 0, 1);
    layout_listbox(layout, list2, 0, 3);
    layout_listbox(layout, list3, 0, 5);

    /* Vertical margin between a label and the above listbox */
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 3, 5);

    /* The vertical expansion will be distributed equally between listboxes */
    layout_vexpand3(layout, 1, 3, 5, .33f, .33f);

    app->list1 = list1;
    app->list2 = list2;
    app->list3 = list3;
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnTable(App *app, Event *e)
{
    uint32_t etype = event_type(e);

    switch (etype)
    {
    case ekGUI_EVENT_TBL_NROWS:
    {
        uint32_t *n = event_result(e, uint32_t);
        *n = 20;
        break;
    }

    case ekGUI_EVENT_TBL_CELL:
    {
        const EvTbPos *pos = event_params(e, EvTbPos);
        EvTbCell *cell = event_result(e, EvTbCell);
        bstd_sprintf(app->temptxt, sizeof(app->temptxt), "Name %d", pos->row);
        cell->text = app->temptxt;
        break;
    }
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_table_layout(App *app)
{
    Layout *layout = layout_create(1, 1);
    TableView *table = tableview_create();
    tableview_new_column_text(table);
    tableview_size(table, s2df(150, 200));
    tableview_column_width(table, 0, 120);
    tableview_OnData(table, listener(app, i_OnTable, App));
    tableview_update(table);
    layout_tableview(layout, table, 0, 0);
    app->table = table;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_middle_layout(App *app)
{
    Layout *layout1 = layout_create(3, 1);
    Layout *layout2 = i_list_layout(app);
    Layout *layout3 = i_table_layout(app);
    View *view = view_scroll();
    view_size(view, s2df(450, 200));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    view_OnOverlay(view, listener(app, i_OnOverlay, App));
    view_OnMove(view, listener(app, i_OnMove, App));
    view_OnUp(view, listener(app, i_OnUp, App));
    view_OnDown(view, listener(app, i_OnDown, App));
    view_OnFocus(view, listener(app, i_OnFocus, App));
    view_OnKeyDown(view, listener(app, i_OnKeyDown, App));
    layout_layout(layout1, layout2, 0, 0);
    layout_view(layout1, view, 1, 0);
    layout_layout(layout1, layout3, 2, 0);

    /* Add the view to tabstop list */
    layout_tabstop(layout1, 1, 0, TRUE);

    /* A small horizontal margin between view cell and list (left) table (right) layouts */
    layout_hmargin(layout1, 0, 3);
    layout_hmargin(layout1, 1, 3);

    /* All the horizontal expansion will be done in the middle cell (view)
       list_layout (left) and table_layout (right) will preserve the 'natural' width */
    layout_hexpand(layout1, 1);
    app->view = view;
    app->middle_layout = layout1;
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_multi_layout(App *app)
{
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = i_control_layout(app);
    Layout *layout3 = i_middle_layout(app);
    Layout *layout4 = i_info_layout(app);
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 0, 1);
    layout_layout(layout1, layout4, 0, 2);

    /* All the vertical expansion will be done in the middle layout
       control_layout (top) and info_layout (bottom) will preserve the 'natural' height */
    layout_vexpand(layout1, 1);

    /* A vertical margins between middle and (controls, info) */
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);

    /* A border margin for all layout edges */
    layout_margin(layout1, 5);

    /* Object binding to main layout */
    layout_dbind(layout1, listener(app, i_OnDataChange, App), App);
    layout_dbind_obj(layout1, app, App);

    return layout1;
}

/*---------------------------------------------------------------------------*/

static void i_OnDrawFull(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    char_t text[128];
    real32_t twidth = 0, theight = 0;
    real32_t radius = 100;

    /* Background */
    draw_fill_color(p->ctx, app->backcolor);
    draw_rect(p->ctx, ekFILL, p->x, p->y, p->width, p->height);

    /* Draw a text centered into view */
    draw_text_color(p->ctx, gui_label_color());
    draw_font(p->ctx, app->fullfont);
    draw_text_align(p->ctx, ekCENTER, ekCENTER);
    draw_text_halign(p->ctx, ekCENTER);
    bstd_sprintf(text, sizeof(text), "Cell col: %d\nCell row: %d", app->sel_cell_x, app->sel_cell_y);
    draw_text(p->ctx, text, p->width / 2, p->height / 2);

    /* Draw a text border */
    draw_line_color(p->ctx, app->drawcolor);
    draw_line_width(p->ctx, 3);
    draw_text_extents(p->ctx, text, -1, &twidth, &theight);
    draw_rect(p->ctx, ekSTROKE, (p->width - twidth) / 2, (p->height - theight) / 2, twidth, theight);

    /* Draw corner circles */
    draw_line_color(p->ctx, kCOLOR_RED);
    draw_fill_color(p->ctx, app->drawcolor);
    draw_circle(p->ctx, ekSKFILL, 0, 0, radius);
    draw_circle(p->ctx, ekSKFILL, 0, p->height, radius);
    draw_circle(p->ctx, ekSKFILL, p->width, 0, radius);
    draw_circle(p->ctx, ekSKFILL, p->width, p->height, radius);
    unref(app);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDownFull(App *app, Event *e)
{
    /* If we press any key with the fullview active, the normal
     * layout will be restored and draw big view focused*/
    panel_visible_layout(app->panel, 0);
    panel_update(app->panel);
    window_focus(app->window, guicontrol(app->view));
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_full_layout(App *app)
{
    Layout *layout = layout_create(1, 1);
    View *view = view_create();
    view_OnDraw(view, listener(app, i_OnDrawFull, App));
    view_OnKeyDown(view, listener(app, i_OnKeyDownFull, App));
    layout_view(layout, view, 0, 0);
    app->fullview = view;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout1 = i_multi_layout(app);
    Layout *layout2 = i_full_layout(app);
    panel_layout(panel, layout1);
    panel_layout(panel, layout2);
    app->main_layout = layout1;
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnMoved(App *app, Event *e)
{
    const EvPos *p = event_params(e, EvPos);
    bstd_printf("Window moved: (%d, %d)\n", (uint32_t)p->x, (uint32_t)p->y);
    unref(app);
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Window *i_create_flywin(void)
{
    uint32_t nrows = 4;
    Layout *layout = layout_create(2, nrows);
    Panel *panel = panel_create();
    Window *window = window_create(ekWINDOW_RETURN | ekWINDOW_ESC);
    uint32_t i;

    for (i = 0; i < nrows; ++i)
    {
        char_t text[64];
        Label *label = label_create();
        Slider *slider = slider_create();
        bstd_sprintf(text, sizeof(text), "Flyout control %d", i);
        label_text(label, text);
        layout_label(layout, label, 0, i);
        layout_slider(layout, slider, 1, i);

        if (i < nrows - 1)
            layout_vmargin(layout, i, 5);
    }

    layout_hmargin(layout, 0, 5);
    layout_margin(layout, 10);
    layout_skcolor(layout, kCOLOR_RED);
    panel_layout(panel, layout);
    window_panel(window, panel);
    return window;
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
    app->overlay = FALSE;
    app->focus = FALSE;
    panel = i_panel(app);
    app->window = window_create(ekWINDOW_STDRES);
    app->flyout = i_create_flywin();
    app->panel = panel;
    app->anim_frame = 0;
    app->anim_total = str_len_c(i_STATUS_TEXT);
    app->fullfont = font_system(40, 0);
    app->drawcolor = gui_alt_color(color_rgb(80, 80, 240), color_rgb(240, 240, 80));
    app->backcolor = gui_alt_color(color_rgb(200, 240, 200), color_rgb(80, 128, 80));
    i_content_size(app);
    window_panel(app->window, panel);
    window_title(app->window, "Big drawing area");
    window_origin(app->window, v2df(500, 200));
    window_OnMoved(app->window, listener(app, i_OnMoved, App));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    /* The keyboard focus initially into the view */
    window_focus(app->window, guicontrol(app->view));
    i_scroll_to_cell(app);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    window_destroy(&(*app)->flyout);
    font_destroy(&(*app)->fullfont);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

static void i_update(App *app, const real64_t prtime, const real64_t ctime)
{
    char_t text[128];
    unref(prtime);
    unref(ctime);
    str_copy_cn(text, sizeof(text), i_STATUS_TEXT, app->anim_frame);
    label_text(app->status_label, text);
    progress_value(app->progress, (real32_t)app->anim_frame / (real32_t)app->anim_total);
    app->anim_frame += 1;
    if (app->anim_frame > app->anim_total)
        app->anim_frame = 0;
}

/*---------------------------------------------------------------------------*/

#include <osapp/osmain.h>
osmain_sync(0.1, i_create, i_destroy, i_update, "", App)
