/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: modalwin.c
 *
 */

/* Listboxes */

#include "modalwin.h"
#include "guiall.h"

typedef struct _modal_data_t ModalData;

struct _modal_data_t
{
    uint32_t type;
    Label *label;
    Window *parent;
};

/*---------------------------------------------------------------------------*/

static const char_t *i_MODAL0 = "Modal Window without [Return] nor [Esc]";
static const char_t *i_MODAL1 = "Modal Window with [Return]";
static const char_t *i_MODAL2 = "Modal Window with [Esc]";
static const char_t *i_MODAL3 = "Modal Window with [Return] and [Esc]";

/*---------------------------------------------------------------------------*/

static Layout *i_modal_layout(ModalData *data);

/*---------------------------------------------------------------------------*/

static ModalData* i_modal_data(Window* parent)
{
    ModalData *data = heap_new0(ModalData);
    data->parent = parent;
    data->type = UINT32_MAX;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_modal_data(ModalData** data)
{
    heap_delete(data, ModalData);
}

/*---------------------------------------------------------------------------*/

static void i_OnCloseModal(Window* window, Event* e)
{
    Button *button = event_sender(e, Button);
    window_stop_modal(window, button_get_tag(button));
}

/*---------------------------------------------------------------------------*/

static Layout* i_close_layout(Window *window)
{
    Layout *layout = layout_create(1, 4);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    button_text(button1, "Close modal with 10 value");
    button_text(button2, "Close modal with 20 value");
    button_text(button3, "Close modal with 30 value");
    button_text(button4, "Close modal with 40 value");
    button_tag(button1, 10);
    button_tag(button2, 20);
    button_tag(button3, 30);
    button_tag(button4, 40);
    button_OnClick(button1, listener(window, i_OnCloseModal, Window));
    button_OnClick(button2, listener(window, i_OnCloseModal, Window));
    button_OnClick(button3, listener(window, i_OnCloseModal, Window));
    button_OnClick(button4, listener(window, i_OnCloseModal, Window));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_window_flags(const uint32_t type)
{
    uint32_t flags = ekWINDOW_TITLE | ekWINDOW_CLOSE;
    switch(type) {
    case 0:
        return flags;
    case 1:
        return flags | ekWINDOW_RETURN;
    case 2:
        return flags | ekWINDOW_ESC;
    case 3:
        return flags | ekWINDOW_RETURN | ekWINDOW_ESC;
    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_window_title(const uint32_t type)
{
    switch(type) {
    case 0:
        return i_MODAL0;
    case 1:
        return i_MODAL1;
    case 2:
        return i_MODAL2;
    case 3:
        return i_MODAL3;
    cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_modal_window(ModalData *data)
{
    uint32_t flags = i_window_flags(data->type);
    Window *window = window_create(flags);
    ModalData *ndata = i_modal_data(window);
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = i_modal_layout(ndata);
    Layout *layout3 = i_close_layout(window);
    uint32_t retval = UINT32_MAX;
    V2Df pos = window_get_origin(data->parent);
    char_t text[128];
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 1, 0);
    layout_hmargin(layout1, 0, 10);
    layout_valign(layout1, 1, 0, ekTOP);
    layout_margin(layout1, 10);
    panel_data(panel, &ndata, i_destroy_modal_data, ModalData);
    panel_layout(panel, layout1);
    window_panel(window, panel);
    window_title(window, i_window_title(data->type));
    window_origin(window, v2df(pos.x + 20, pos.y + 20));
    retval = window_modal(window, data->parent);
    
    if (retval == (uint32_t)ekGUI_CLOSE_ESC)
        bstd_sprintf(text, sizeof(text), "Modal stop: [Esc] (%d)", retval);
    else if (retval == (uint32_t)ekGUI_CLOSE_INTRO)
        bstd_sprintf(text, sizeof(text), "Modal stop: [Return] (%d)", retval);
    else if (retval == (uint32_t)ekGUI_CLOSE_BUTTON)
        bstd_sprintf(text, sizeof(text), "Modal stop: [X] (%d)", retval);
    else
        bstd_sprintf(text, sizeof(text), "Modal stop: %d", retval);

    label_text(data->label, text);
    window_destroy(&window);
}

/*---------------------------------------------------------------------------*/

static void i_OnClickModal(ModalData* data, Event* e)
{
    Button *button = event_sender(e, Button);
    data->type = button_get_tag(button);
    i_modal_window(data);
}

/*---------------------------------------------------------------------------*/

static Layout *i_modal_layout(ModalData *data)
{
    Layout *layout = layout_create(1, 5);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Label *label = label_create();
    cassert(data->label == NULL);
    data->label = label;
    button_text(button1, i_MODAL0);
    button_text(button2, i_MODAL1);
    button_text(button3, i_MODAL2);
    button_text(button4, i_MODAL3);
    label_text(label, "Modal stop: --");
    button_tag(button1, 0);
    button_tag(button2, 1);
    button_tag(button3, 2);
    button_tag(button4, 3);
    button_OnClick(button1, listener(data, i_OnClickModal, ModalData));
    button_OnClick(button2, listener(data, i_OnClickModal, ModalData));
    button_OnClick(button3, listener(data, i_OnClickModal, ModalData));
    button_OnClick(button4, listener(data, i_OnClickModal, ModalData));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_label(layout, label, 0, 4);
    layout_halign(layout, 0, 4, ekJUSTIFY);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 20);
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel *modal_windows(Window *parent)
{
    Panel *panel = panel_create();
    ModalData *data = i_modal_data(parent);
    Layout *layout = i_modal_layout(data);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_modal_data, ModalData);
    return panel;
}
