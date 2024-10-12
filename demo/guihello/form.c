/* Form demo */

#include "form.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

typedef struct _form_data_t FormData;

struct _form_data_t
{
    Window *window;
    Window *modal_window;
    Button *validate_check;
};

#define BUTTON_YES 1000
#define BUTTON_NO 1001

/*---------------------------------------------------------------------------*/

static void i_destroy_data(FormData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    ptr_destopt(window_destroy, &(*data)->modal_window, Window);
    heap_delete(data, FormData);
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(void *noused, Event *e)
{
    const EvText *params = event_params(e, EvText);
    EvTextFilter *result = event_result(e, EvTextFilter);
    uint32_t i = 0, j = 0;
    while (params->text[i] != '\0')
    {
        if (params->text[i] >= '0' && params->text[i] <= '9')
        {
            result->text[j] = params->text[i];
            j += 1;
        }

        i += 1;
    }

    result->text[j] = '\0';
    result->apply = TRUE;
    unref(noused);
}

/*---------------------------------------------------------------------------*/

static void i_OnModalButton(FormData *data, Event *e)
{
    Button *button = event_sender(e, Button);
    uint32_t tag = button_get_tag(button);
    window_stop_modal(data->modal_window, tag);
}

/*---------------------------------------------------------------------------*/

static Window *i_modal_window(FormData *data, Edit *edit, const GuiControl *next, const char_t *field_text, const char_t *action_text)
{
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = layout_create(3, 1);
    Label *label = label_multiline();
    Button *button1 = button_push();
    Button *button2 = button_push();
    Panel *panel = panel_create();
    Window *window = window_create(ekWINDOW_STD | ekWINDOW_ESC);
    String *str = str_printf("Do you want to validate the text '%s' of the EditBox '%p'? The focus will be moved to the '%p' control using the '%s' action.", field_text, (void *)edit, (void *)next, action_text);
    label_text(label, tc(str));
    button_text(button1, "Yes");
    button_text(button2, "No");
    button_tag(button1, BUTTON_YES);
    button_tag(button2, BUTTON_NO);
    button_OnClick(button1, listener(data, i_OnModalButton, FormData));
    button_OnClick(button2, listener(data, i_OnModalButton, FormData));
    layout_label(layout1, label, 0, 0);
    layout_button(layout2, button1, 1, 0);
    layout_button(layout2, button2, 2, 0);
    layout_layout(layout1, layout2, 0, 1);
    layout_hsize(layout1, 0, 250);
    layout_vmargin(layout1, 0, 10);
    layout_hmargin(layout2, 1, 5);
    layout_margin4(layout2, 0, 10, 0, 0);
    layout_margin(layout1, 10);
    layout_hexpand(layout2, 0);
    window_title(window, "Data validation");
    panel_layout(panel, layout1);
    window_panel(window, panel);
    window_defbutton(window, button1);
    str_destroy(&str);
    return window;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_action_text(const gui_tab_t action)
{
    switch (action)
    {
    case ekGUI_TAB_KEY:
        return "TAB_KEY";
    case ekGUI_TAB_BACKKEY:
        return "TAB_BACKKEY";
    case ekGUI_TAB_NEXT:
        return "TAB_NEXT";
    case ekGUI_TAB_PREV:
        return "TAB_PREV";
    case ekGUI_TAB_MOVE:
        return "TAB_MOVE";
    case ekGUI_TAB_CLICK:
        return "TAB_CLICK";
    }

    return "TAB_UNKNOWN";
}

/*---------------------------------------------------------------------------*/

static V2Df i_modal_pos(Window *window, Window *parent)
{
    V2Df pos = window_get_origin(parent);
    S2Df s1 = window_get_size(parent);
    S2Df s2 = window_get_size(window);
    real32_t x = pos.x + ((s1.width - s2.width) / 2);
    real32_t y = pos.y + ((s1.height - s2.height) / 2);
    return v2df(x, y);
}

/*---------------------------------------------------------------------------*/

static bool_t i_validate_field(FormData *data, Edit *edit, const char_t *text)
{
    FocusInfo info;
    const char_t *action_text = NULL;
    uint32_t modal_value = UINT32_MAX;
    V2Df pos;
    cassert_no_null(data);
    cassert(data->modal_window == NULL);
    window_focus_info(data->window, &info);
    action_text = i_action_text(info.action);
    data->modal_window = i_modal_window(data, edit, info.next, text, action_text);
    pos = i_modal_pos(data->modal_window, data->window);
    window_origin(data->modal_window, pos);
    modal_value = window_modal(data->modal_window, data->window);
    window_destroy(&data->modal_window);
    switch (modal_value)
    {
    case ekGUI_CLOSE_BUTTON:
    case ekGUI_CLOSE_ESC:
    case BUTTON_NO:
        return FALSE;
    case BUTTON_YES:
        return TRUE;
        cassert_default();
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_OnEditChange(FormData *data, Event *e)
{
    const EvText *p = event_params(e, EvText);
    Edit *edit = event_sender(e, Edit);
    cassert_no_null(data);
    if (button_get_state(data->validate_check) == ekGUI_ON)
    {
        bool_t *r = event_result(e, bool_t);
        *r = i_validate_field(data, edit, p->text);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnUpDown(Edit *edit, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    int32_t n = str_to_i32(edit_get_text(edit), 10, NULL);
    char_t text[64];
    n += (params->index == 0) ? 1 : -1;
    bstd_sprintf(text, sizeof(text), "%d", n);
    edit_text(edit, text);
}

/*---------------------------------------------------------------------------*/

static Layout *i_numbers(FormData *data, color_t colorbg)
{
    Layout *layout = layout_create(5, 1);
    Label *label = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    UpDown *updown1 = updown_create();
    UpDown *updown2 = updown_create();
    label_text(label, "Height (cm):");
    edit_text(edit1, "25");
    edit_text(edit2, "175");
    edit_autoselect(edit1, TRUE);
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    edit_OnFilter(edit1, listener(NULL, i_OnFilter, void));
    edit_OnFilter(edit2, listener(NULL, i_OnFilter, void));
    edit_OnChange(edit1, listener(data, i_OnEditChange, FormData));
    edit_OnChange(edit2, listener(data, i_OnEditChange, FormData));
    edit_bgcolor_focus(edit1, colorbg);
    edit_bgcolor_focus(edit2, colorbg);
    updown_OnClick(updown1, listener(edit1, i_OnUpDown, Edit));
    updown_OnClick(updown2, listener(edit2, i_OnUpDown, Edit));
    updown_tooltip(updown1, "Increase/Decrease age");
    updown_tooltip(updown2, "Increase/Decrease height");
    layout_label(layout, label, 2, 0);
    layout_edit(layout, edit1, 0, 0);
    layout_edit(layout, edit2, 3, 0);
    layout_updown(layout, updown1, 1, 0);
    layout_updown(layout, updown2, 4, 0);
    layout_hmargin(layout, 1, 10.f);
    layout_hmargin(layout, 2, 10.f);
    layout_hexpand2(layout, 0, 3, .5f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_edits(FormData *data)
{
    color_t colorbg = gui_alt_color(color_rgb(255, 255, 192), color_rgb(112, 112, 48));
    Layout *layout1 = layout_create(2, 6);
    Layout *layout2 = i_numbers(data, colorbg);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    Edit *edit4 = edit_create();
    Edit *edit5 = edit_create();
    label_text(label1, "User Name:");
    label_text(label2, "Password:");
    label_text(label3, "Address:");
    label_text(label4, "City:");
    label_text(label5, "Phone:");
    label_text(label6, "Age:");
    label_color_over(label1, color_rgb(255, 128, 52));
    label_color_over(label2, color_rgb(70, 129, 207));
    label_color_over(label3, color_rgb(119, 188, 31));
    label_style_over(label4, ekFITALIC | ekFUNDERLINE);
    edit_text(edit1, "Amanda Callister");
    edit_text(edit2, "aQwe56nhjJk");
    edit_text(edit3, "35, Tuam Road");
    edit_text(edit4, "Galway - Ireland");
    edit_text(edit5, "+35 654 333 000");
    edit_OnChange(edit1, listener(data, i_OnEditChange, FormData));
    edit_OnChange(edit2, listener(data, i_OnEditChange, FormData));
    edit_OnChange(edit3, listener(data, i_OnEditChange, FormData));
    edit_OnChange(edit4, listener(data, i_OnEditChange, FormData));
    edit_OnChange(edit5, listener(data, i_OnEditChange, FormData));
    edit_select(edit1, 2, 6);
    edit_passmode(edit2, TRUE);
    edit_bgcolor_focus(edit1, colorbg);
    edit_bgcolor_focus(edit2, colorbg);
    edit_bgcolor_focus(edit3, colorbg);
    edit_bgcolor_focus(edit4, colorbg);
    edit_bgcolor_focus(edit5, colorbg);
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 1);
    layout_label(layout1, label3, 0, 2);
    layout_label(layout1, label4, 0, 3);
    layout_label(layout1, label5, 0, 4);
    layout_label(layout1, label6, 0, 5);
    layout_edit(layout1, edit1, 1, 0);
    layout_edit(layout1, edit2, 1, 1);
    layout_edit(layout1, edit3, 1, 2);
    layout_edit(layout1, edit4, 1, 3);
    layout_edit(layout1, edit5, 1, 4);
    layout_layout(layout1, layout2, 1, 5);
    layout_hmargin(layout1, 0, 5);
    layout_hexpand(layout1, 1);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    layout_vmargin(layout1, 4, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_toolbar(FormData *data)
{
    Layout *layout = layout_create(1, 1);
    Button *check = button_check();
    button_text(check, "Field validations");
    layout_button(layout, check, 0, 0);
    data->validate_check = check;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_form(FormData *data)
{
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = i_edits(data);
    Layout *layout3 = i_toolbar(data);
    Label *label = label_multiline();
    cassert_no_null(data);
    label_text(label, "Please fill in all the information on the form. We will use this data to send commercial mail at all hours, not caring much if it bothers you or not.");
    label_color(label, gui_alt_color(color_rgb(255, 0, 0), color_rgb(180, 180, 180)));
    label_bgcolor(label, gui_alt_color(color_rgb(216, 191, 216), color_rgb(80, 40, 40)));
    label_bgcolor_over(label, gui_alt_color(color_rgb(255, 250, 205), color_rgb(105, 100, 55)));
    label_style_over(label, ekFUNDERLINE);
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 0, 1);
    layout_label(layout1, label, 0, 2);
    layout_hsize(layout1, 0, 300);
    layout_halign(layout1, 0, 1, ekLEFT);
    layout_vmargin(layout1, 0, 10);
    layout_vmargin(layout1, 1, 10);
    layout_margin(layout1, 10);
    layout_tabstop(layout1, 0, 1, FALSE);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static FormData *i_form_data(Window *window)
{
    FormData *data = heap_new0(FormData);
    data->window = window;
    return data;
}

/*---------------------------------------------------------------------------*/

Panel *form_basic(Window *window)
{
    FormData *data = i_form_data(window);
    Layout *layout = i_form(data);
    Panel *panel = panel_create();
    panel_data(panel, &data, i_destroy_data, FormData);
    panel_layout(panel, layout);
    return panel;
}
