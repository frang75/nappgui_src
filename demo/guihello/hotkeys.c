/* Hotkeys */

#include "hotkeys.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

typedef struct _paneldata_t PanelData;

struct _paneldata_t
{
    TextView *view;
    Window *window;
};

/*---------------------------------------------------------------------------*/

static void i_clear_hotkeys(Window *window)
{
    window_clear_hotkeys(window);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_panel_data(PanelData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    i_clear_hotkeys((*data)->window);
    heap_delete(data, PanelData);
}

/*---------------------------------------------------------------------------*/

static void i_hotkey_event(PanelData *data, Event *e, const char_t *key)
{
    cassert_no_null(data);
    textview_printf(data->view, "[%s] hotkey pressed\n", key);
    textview_select(data->view, -1, -1);
    textview_scroll_caret(data->view);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnF1(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F1");
}

/*---------------------------------------------------------------------------*/

static void i_OnF2(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F2");
}

/*---------------------------------------------------------------------------*/

static void i_OnF3(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F3");
}

/*---------------------------------------------------------------------------*/

static void i_OnF4(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F4");
}

/*---------------------------------------------------------------------------*/

static void i_OnF5(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F5");
}

/*---------------------------------------------------------------------------*/

static void i_OnF6(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F6");
}

/*---------------------------------------------------------------------------*/

static void i_OnF7(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F7");
}

/*---------------------------------------------------------------------------*/

static void i_OnF8(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F8");
}

/*---------------------------------------------------------------------------*/

static void i_OnF9(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F9");
}

/*---------------------------------------------------------------------------*/

static void i_OnF10(PanelData *data, Event *e)
{
    i_hotkey_event(data, e, "F10");
}

/*---------------------------------------------------------------------------*/

static void i_set_hotkeys(PanelData *data)
{
    cassert_no_null(data);
    window_hotkey(data->window, ekKEY_F1, 0, listener(data, i_OnF1, PanelData));
    window_hotkey(data->window, ekKEY_F2, 0, listener(data, i_OnF2, PanelData));
    window_hotkey(data->window, ekKEY_F3, 0, listener(data, i_OnF3, PanelData));
    window_hotkey(data->window, ekKEY_F4, 0, listener(data, i_OnF4, PanelData));
    window_hotkey(data->window, ekKEY_F5, 0, listener(data, i_OnF5, PanelData));
    window_hotkey(data->window, ekKEY_F6, 0, listener(data, i_OnF6, PanelData));
    window_hotkey(data->window, ekKEY_F7, 0, listener(data, i_OnF7, PanelData));
    window_hotkey(data->window, ekKEY_F8, 0, listener(data, i_OnF8, PanelData));
    window_hotkey(data->window, ekKEY_F9, 0, listener(data, i_OnF9, PanelData));
    window_hotkey(data->window, ekKEY_F10, 0, listener(data, i_OnF10, PanelData));
}

/*---------------------------------------------------------------------------*/

static void i_OnClick1(PanelData *data, Event *e)
{
    cassert_no_null(data);
    i_clear_hotkeys(data->window);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static void i_OnClick2(PanelData *data, Event *e)
{
    i_set_hotkeys(data);
    unref(e);
}

/*---------------------------------------------------------------------------*/

Panel *hotkeys(Window *window)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 5);
    Label *label = label_create();
    Edit *edit = edit_create();
    TextView *view = textview_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    PanelData *data = heap_new(PanelData);
    label_text(label, "Press [F1]-[F10] hotkeys");
    button_text(button1, "Unset hotkeys");
    button_text(button2, "Set hotkeys");
    button_OnClick(button1, listener(data, i_OnClick1, PanelData));
    button_OnClick(button2, listener(data, i_OnClick2, PanelData));
    layout_label(layout, label, 0, 0);
    layout_edit(layout, edit, 0, 1);
    layout_textview(layout, view, 0, 2);
    layout_button(layout, button1, 0, 3);
    layout_button(layout, button2, 0, 4);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 5);
    panel_layout(panel, layout);
    data->window = window;
    data->view = view;
    i_set_hotkeys(data);
    panel_data(panel, &data, i_destroy_panel_data, PanelData);
    return panel;
}
