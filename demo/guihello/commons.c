/* Common windows */

#include "commons.h"
#include <gui/guiall.h>

typedef struct _wcommons_t WCommons;

struct _wcommons_t
{
    Window *parent;
    TextView *view;
    String *folder;
    color_t color;
    color_t colors[16];
};

/*---------------------------------------------------------------------------*/

static void i_write(WCommons *coms, const char_t *text)
{
    cassert_no_null(coms);
    if (str_empty_c(text) == FALSE)
    {
        textview_writef(coms->view, text);
        textview_writef(coms->view, "\n");
        textview_scroll_caret(coms->view);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSelectFolder(WCommons *coms, Event *e)
{
    const char_t *folder = NULL;
    cassert_no_null(coms);
    unref(e);
    folder = comwin_select_dir(coms->parent, "This is a caption text for select folder dialog", tc(coms->folder));
    i_write(coms, folder);
    if (str_empty_c(folder) == FALSE)
        str_upd(&coms->folder, folder);
}

/*---------------------------------------------------------------------------*/

static void i_OnOpenFile(WCommons *coms, Event *e)
{
    const char_t *file = NULL;
    cassert_no_null(coms);
    unref(e);
    file = comwin_open_file(coms->parent, "This is a caption text for open file dialog", NULL, 0, tc(coms->folder));
    i_write(coms, file);
}

/*---------------------------------------------------------------------------*/

static void i_OnSaveFile(WCommons *coms, Event *e)
{
    const char_t *file = NULL;
    cassert_no_null(coms);
    unref(e);
    file = comwin_save_file(coms->parent, "This is a caption text for save file dialog", NULL, 0, tc(coms->folder));
    i_write(coms, file);
}

/*---------------------------------------------------------------------------*/

static void i_OnColorChange(WCommons *coms, Event *e)
{
    color_t *p = event_params(e, color_t);
    uint8_t r, g, b, a;
    String *text = NULL;
    cassert_no_null(coms);
    color_get_rgba(*p, &r, &g, &b, &a);
    text = str_printf("Color: %d %d %d %d", r, g, b, a);
    i_write(coms, tc(text));
    str_destroy(&text);
    coms->color = *p;
}

/*---------------------------------------------------------------------------*/

static void i_OnColorSel(WCommons *coms, Event *e)
{
    V2Df pos;
    S2Df size;
    cassert_no_null(coms);
    unref(e);
    pos = window_get_origin(coms->parent);
    size = window_get_size(coms->parent);
    comwin_color(coms->parent, "This is title for color dialog", pos.x + size.width / 2, pos.y + size.height / 2, ekCENTER, ekCENTER, coms->color, coms->colors, sizeof(coms->colors) / sizeof(color_t), listener(coms, i_OnColorChange, WCommons));
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(WCommons *coms)
{
    Layout *layout = layout_create(1, 5);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    TextView *view = textview_create();
    cassert_no_null(coms);
    button_text(button1, "Select folder");
    button_text(button2, "Open file");
    button_text(button3, "Save file");
    button_text(button4, "Color selector");
    textview_size(view, s2df(300, 100));
    button_OnClick(button1, listener(coms, i_OnSelectFolder, WCommons));
    button_OnClick(button2, listener(coms, i_OnOpenFile, WCommons));
    button_OnClick(button3, listener(coms, i_OnSaveFile, WCommons));
    button_OnClick(button4, listener(coms, i_OnColorSel, WCommons));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_textview(layout, view, 0, 4);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 5);
    coms->view = view;
    return layout;
}

/*---------------------------------------------------------------------------*/

static WCommons *i_coms(Window *parent)
{
    WCommons *coms = heap_new0(WCommons);
    coms->parent = parent;
    coms->color = kCOLOR_BLUE;
    coms->colors[0] = kCOLOR_BLACK;
    coms->colors[1] = kCOLOR_WHITE;
    coms->colors[2] = kCOLOR_RED;
    coms->colors[3] = kCOLOR_GREEN;
    coms->colors[4] = kCOLOR_BLUE;
    coms->colors[5] = kCOLOR_YELLOW;
    coms->colors[6] = kCOLOR_CYAN;
    coms->colors[7] = kCOLOR_MAGENTA;
    coms->colors[8] = 0xFF888888;
    coms->colors[9] = 0xFF000088;
    coms->colors[10] = 0xFF008800;
    coms->colors[11] = 0xFF880000;
    coms->colors[12] = 0xFF008888;
    coms->colors[13] = 0xFF888800;
    coms->colors[14] = 0xFF880088;
    coms->colors[15] = 0xFF440044;
    return coms;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_coms(WCommons **coms)
{
    cassert_no_null(coms);
    cassert_no_null(*coms);
    str_destopt(&(*coms)->folder);
    heap_delete(coms, WCommons);
}

/*---------------------------------------------------------------------------*/

Panel *common_windows(Window *parent)
{
    WCommons *coms = i_coms(parent);
    Layout *layout = i_layout(coms);
    Panel *panel = panel_create();
    panel_layout(panel, layout);
    panel_data(panel, &coms, i_destroy_coms, WCommons);
    return panel;
}
