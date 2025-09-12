/* Reduce components to limit */

#include "reduce.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct _reducedata_t ReduceData;

struct _reducedata_t
{
    Button *button;
    Layout *layout;
    Window *parent;
    Window *window;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(ReduceData **data)
{
    cassert_no_null(data);
    if ((*data)->window != NULL)
        window_destroy(&(*data)->window);
    heap_delete(data, ReduceData);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(void *nonused, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    draw_clear(p->ctx, kCOLOR_BLUE);
    unref(nonused);
}

/*---------------------------------------------------------------------------*/

static void i_set_reduce_mode(Layout *layout, const align_t align)
{
    layout_halign(layout, 0, 0, align);
    layout_halign(layout, 0, 1, align);
    layout_halign(layout, 0, 2, align);
    layout_halign(layout, 0, 3, align);
    layout_halign(layout, 0, 4, align);
    layout_halign(layout, 0, 5, align);
    layout_halign(layout, 0, 6, align);
    layout_halign(layout, 0, 7, align);
    layout_halign(layout, 0, 8, align);
}

/*---------------------------------------------------------------------------*/

static void i_OnReduceClick(ReduceData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cassert_no_null(data);
    i_set_reduce_mode(data->layout, p->state == ekGUI_ON ? ekJUSTIFY : ekLEFT);
    window_update(data->window);
}

/*---------------------------------------------------------------------------*/

static Layout *i_view_layout(ReduceData *data)
{
    Layout *layout = layout_create(1, 2);
    View *view = view_create();
    Button *check = button_check();
    view_OnDraw(view, listener(NULL, i_OnDraw, void));
    button_text(check, "Make reducible");
    button_state(check, ekGUI_ON);
    button_OnClick(check, listener(data, i_OnReduceClick, ReduceData));
    layout_view(layout, view, 0, 0);
    layout_button(layout, check, 0, 1);
    layout_vmargin(layout, 0, 5);
    layout_vexpand(layout, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static PopUp *i_popup(void)
{
    PopUp *popup = popup_create();
    popup_add_elem(popup, "English", gui_image(UKING_PNG));
    popup_add_elem(popup, "Español", gui_image(SPAIN_PNG));
    popup_add_elem(popup, "Portugues", gui_image(PORTUGAL_PNG));
    popup_add_elem(popup, "Italiana", gui_image(ITALY_PNG));
    popup_add_elem(popup, "Tiếng Việt", gui_image(VIETNAM_PNG));
    popup_add_elem(popup, "России", gui_image(RUSSIA_PNG));
    popup_add_elem(popup, "日本語", gui_image(JAPAN_PNG));
    popup_list_height(popup, 10);
    return popup;
}

/*---------------------------------------------------------------------------*/

static Window *i_reducible_window(ReduceData *data)
{
    Layout *layout1 = layout_create(3, 1);
    Layout *layout2 = layout_create(1, 9);
    Layout *layout3 = i_view_layout(data);
    Label *label = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_multiline();
    Button *button = button_push();
    Button *check = button_check();
    Button *radio = button_radio();
    PopUp *popup = i_popup();
    Slider *slider = slider_create();
    Progress *progress = progress_create();
    Panel *panel = panel_create();
    Window *window = window_create(ekWINDOW_STDRES);
    View *view = view_create();
    label_text(label, "Hello, this is a text label.");
    label_bgcolor(label, gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112)));
    button_text(button, "Hello, this is a push button");
    button_text(check, "Hello, this is a checkbox");
    button_text(radio, "Hello, this is a radio button");
    view_OnDraw(view, listener(NULL, i_OnDraw, void));
    layout_label(layout2, label, 0, 0);
    layout_edit(layout2, edit1, 0, 1);
    layout_edit(layout2, edit2, 0, 2);
    layout_button(layout2, button, 0, 3);
    layout_button(layout2, check, 0, 4);
    layout_button(layout2, radio, 0, 5);
    layout_popup(layout2, popup, 0, 6);
    layout_slider(layout2, slider, 0, 7);
    layout_progress(layout2, progress, 0, 8);
    layout_layout(layout1, layout3, 0, 0);
    layout_layout(layout1, layout2, 1, 0);
    layout_view(layout1, view, 2, 0);
    layout_hexpand(layout1, 1);
    layout_margin(layout1, 10);
    layout_hmargin(layout1, 0, 10);
    layout_hmargin(layout1, 1, 10);
    layout_vmargin(layout2, 0, 5);
    layout_vmargin(layout2, 1, 5);
    layout_vmargin(layout2, 2, 5);
    layout_vmargin(layout2, 3, 5);
    layout_vmargin(layout2, 4, 5);
    layout_vmargin(layout2, 5, 5);
    layout_vmargin(layout2, 6, 5);
    layout_vmargin(layout2, 7, 5);
    layout_valign(layout1, 1, 0, ekTOP);
    panel_layout(panel, layout1);
    window_panel(window, panel);
    window_title(window, "Reduce basic components to the limit");
    i_set_reduce_mode(layout2, ekJUSTIFY);
    data->layout = layout2;
    return window;
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(ReduceData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    if (data->window == NULL)
        data->window = i_reducible_window(data);

    {
        R2Df frame = window_control_frame(data->parent, guicontrol(data->button));
        V2Df pos = window_client_to_screen(data->parent, frame.pos);
        window_origin(data->window, pos);
    }

    window_modal(data->window, data->parent);
}

/*---------------------------------------------------------------------------*/

Panel *reduce(Window *window)
{
    ReduceData *data = heap_new0(ReduceData);
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 1);
    Button *button = button_push();
    data->parent = window;
    data->button = button;
    button_text(button, "Press to launch a resizable window");
    button_OnClick(button, listener(data, i_OnClick, ReduceData));
    layout_button(layout, button, 0, 0);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_data, ReduceData);
    return panel;
}
