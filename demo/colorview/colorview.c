/* Color View */

#include <nappgui.h>

typedef struct _viewitem_t ViewItem;
typedef struct _app_t App;

struct _viewitem_t
{
    const char_t *name;
    color_t color;
};

struct _app_t
{
    Window *window;
    View *view;
    ArrSt(ViewItem) *items;
    uint32_t num_cols;
    Font *font;
};

DeclSt(ViewItem);
static const real32_t i_ITEM_WIDTH = 64;
static const real32_t i_VER_MARGIN = 10;
static const real32_t i_HOR_MARGIN = 15;

/*---------------------------------------------------------------------------*/

static void i_add(ArrSt(ViewItem) *items, const char_t *name, const color_t color)
{
    ViewItem *item = arrst_new(items, ViewItem);
    item->name = name;
    item->color = color;
}

/*---------------------------------------------------------------------------*/

static ArrSt(ViewItem) *i_colors(void)
{
    ArrSt(ViewItem) *items = arrst_create(ViewItem);
    i_add(items, "Label", gui_label_color());
    i_add(items, "View", gui_view_color());
    i_add(items, "Line", gui_line_color());
    i_add(items, "Border", gui_border_color());
    i_add(items, "Link", gui_link_color());
    i_add(items, "Alt1", gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112)));
    i_add(items, "Alt2", gui_alt_color(color_rgb(255, 192, 255), color_rgb(128, 48, 112)));
    i_add(items, "Alt3", gui_alt_color(color_rgb(255, 255, 192), color_rgb(112, 112, 48)));
    i_add(items, "Black", kCOLOR_BLACK);
    i_add(items, "White", kCOLOR_WHITE);
    i_add(items, "Red", kCOLOR_RED);
    i_add(items, "Green", kCOLOR_GREEN);
    i_add(items, "Blue", kCOLOR_BLUE);
    i_add(items, "Yellow", kCOLOR_YELLOW);
    i_add(items, "Cyan", kCOLOR_CYAN);
    i_add(items, "Magenta", kCOLOR_MAGENTA);
    i_add(items, "Silver", color_rgb(192, 192, 192));
    i_add(items, "Gray", color_rgb(128, 128, 128));
    i_add(items, "Maroon", color_rgb(128, 0, 0));
    i_add(items, "Olive", color_rgb(128, 128, 0));
    i_add(items, "DGreen", color_rgb(0, 128, 0));
    i_add(items, "Teal", color_rgb(0, 128, 128));
    i_add(items, "Navy", color_rgb(0, 0, 128));
    i_add(items, "Purple", color_rgb(128, 0, 128));
    return items;
}

/*---------------------------------------------------------------------------*/

static void i_draw(DCtx *ctx, real32_t x, real32_t y, real32_t width, real32_t height, const ViewItem *item)
{
    real32_t cx1 = x + width / 2;
    real32_t cx2 = x + (width - i_ITEM_WIDTH) / 2;
    real32_t cy = y + height - i_ITEM_WIDTH;
    draw_fill_color(ctx, item->color);
    draw_rect(ctx, ekFILL, cx2, cy, i_ITEM_WIDTH, i_ITEM_WIDTH);
    draw_text_color(ctx, gui_label_color());
    draw_text(ctx, item->name, cx1, cy);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(App *app, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    real32_t cwidth = (p->width - 2 * i_HOR_MARGIN) / app->num_cols;
    real32_t cheight = i_ITEM_WIDTH + font_height(app->font);

    draw_font(p->ctx, app->font);
    draw_text_align(p->ctx, ekCENTER, ekBOTTOM);

    arrst_foreach(item, app->items, ViewItem)
        uint32_t row = item_i / app->num_cols;
        uint32_t col = item_i % app->num_cols;
        real32_t x = i_HOR_MARGIN + col * cwidth;
        real32_t y = row * cheight + (row + 1) * i_VER_MARGIN;
        i_draw(p->ctx, x, y, cwidth, cheight, item);
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_OnSize(App *app, Event *e)
{
    const EvSize *p = event_params(e, EvSize);
    View *view = event_sender(e, View);
    real32_t minwidth = i_ITEM_WIDTH + 2 * i_HOR_MARGIN;
    real32_t cwidth = 0, cheight = 0;

    cwidth = p->width;

    if (cwidth < minwidth)
    {
        cwidth = minwidth;
        app->num_cols = 1;
    }
    else
    {
        uint32_t n, num_rows;
        app->num_cols = (uint32_t)((cwidth - i_HOR_MARGIN) / (i_ITEM_WIDTH + i_HOR_MARGIN));
        n = arrst_size(app->items, ViewItem);
        num_rows = (n / app->num_cols);
        if ((n % app->num_cols) > 0)
            num_rows += 1;

        cheight = num_rows * (i_ITEM_WIDTH + font_height(app->font) + i_VER_MARGIN) + i_VER_MARGIN;
        if (cheight < p->height)
            cheight = p->height;
    }

    view_content_size(view, s2df(cwidth, cheight), s2df(1, 1));
    view_update(view);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(App *app)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 1);
    View *view = view_scroll();
    view_size(view, s2df(300, 200));
    view_OnDraw(view, listener(app, i_OnDraw, App));
    view_OnSize(view, listener(app, i_OnSize, App));
    layout_view(layout, view, 0, 0);
    panel_layout(panel, layout);
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
    Panel *panel = i_panel(app);
    app->items = i_colors();
    app->font = font_system(font_regular_size(), 0);
    app->window = window_create(ekWINDOW_STDRES);
    window_panel(app->window, panel);
    window_title(app->window, "Color View");
    window_origin(app->window, v2df(500, 200));
    window_size(app->window, s2df(500, 300));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    arrst_destroy(&(*app)->items, NULL, ViewItem);
    window_destroy(&(*app)->window);
    font_destroy((&(*app)->font));
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include <osapp/osmain.h>
osmain(i_create, i_destroy, "", App)
