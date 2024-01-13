/* Use of tabstops */

#include "tabstops.h"
#include <gui/guiall.h>

typedef struct i_data_t i_Data;

struct i_data_t 
{
    Button *button[2];
    PopUp *popup[2];
    UpDown *updown[2];
    View *view[2];
    TextView *text[2];
    bool_t focus[2];
    char_t temp_string[512];
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(i_Data **data)
{
    heap_delete(data, i_Data);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(i_Data *data, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    View *view = event_sender(e, View);
    bool_t focused = FALSE;
    cassert_no_null(data);
    if (view == data->view[0])
        focused = data->focus[0];
    else
        focused = data->focus[1];

    draw_clear(p->ctx, focused == TRUE ? kCOLOR_RED : kCOLOR_CYAN);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(i_Data *data, Event *e)
{
    const bool_t *p = event_params(e, bool_t);
    View *view = event_sender(e, View);
    cassert_no_null(data);
    view_update(view);
    if (view == data->view[0])
        data->focus[0] = *p;
    else
        data->focus[1] = *p;
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(i_Data *data, Event *e)
{
    Button *button = event_sender(e, Button);
    TextView *text = NULL;
    cassert_no_null(data);
    if (button == data->button[0])
        text = data->text[0];
    else
        text = data->text[1];

    textview_writef(text, "Button click\n");
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(i_Data *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    PopUp *popup = event_sender(e, PopUp);
    TextView *text = NULL;
    cassert_no_null(data);
    if (popup == data->popup[0])
        text = data->text[0];
    else
        text = data->text[1];

    textview_printf(text, "PopUp select %d\n", p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnChange(i_Data *data, Event *e)
{
    const EvText *p = event_params(e, EvText);
    bool_t *ok = event_result(e, bool_t);
    unref(data);
    *ok = str_equ_c(p->text, "VALID_TEXT");
}

/*---------------------------------------------------------------------------*/

static void i_OnUpDown(i_Data *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    UpDown *updown = event_sender(e, UpDown);
    TextView *text = NULL;
    cassert_no_null(data);
    if (updown == data->updown[0])
        text = data->text[0];
    else
        text = data->text[1];

    textview_printf(text, "UpDown %d\n", p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnTableData(i_Data *data, Event *e)
{
    uint32_t etype = event_type(e);

    switch(etype) {
    case ekGUI_EVENT_TBL_NROWS:
    {
        uint32_t *n = event_result(e, uint32_t);
        *n = 10;
        break;
    }

    case ekGUI_EVENT_TBL_CELL:
    {
        const EvTbPos *pos = event_params(e, EvTbPos);
        EvTbCell *cell = event_result(e, EvTbCell);

        switch(pos->col) {
        case 0:
            cell->align = ekLEFT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Data (0, %d)", pos->row);
            break;

        case 1:
            cell->align = ekLEFT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Data (1, %d)", pos->row);
            break;

        cassert_default();
        }

        cell->text = data->temp_string;
        break;
    }
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(i_Data *data, const uint32_t i)
{
    Layout *layout = layout_create(1, 2);
    Layout *layout1 = layout_create(6, 1);
    Layout *layout2 = layout_create(4, 1);
    Button *button = button_push();
    PopUp *popup = popup_create();
    Edit *edit = edit_create();
    Combo *combo = combo_create();
    ListBox *list = listbox_create();
    Slider *slider = slider_create();
    UpDown *updown = updown_create();
    View *view = view_custom(FALSE, TRUE);
    TextView *text = textview_create();
    TableView *table = tableview_create();
    button_text(button, "This is a button");
    button_OnClick(button, listener(data, i_OnClick, i_Data));
    popup_add_elem(popup, "Option 1", NULL);
    popup_add_elem(popup, "Option 2", NULL);
    popup_OnSelect(popup, listener(data, i_OnSelect, i_Data));
    edit_text(edit, "VALID_TEXT");
    edit_OnChange(edit, listener(data, i_OnChange, i_Data));
    combo_text(combo, "ComboBox");
    updown_OnClick(updown, listener(data, i_OnUpDown, i_Data));
    view_OnDraw(view, listener(data, i_OnDraw, i_Data));
    view_OnFocus(view, listener(data, i_OnFocus, i_Data));
    textview_writef(text, "Hello text!\n");
    listbox_add_elem(list, "Elem 1", NULL);
    listbox_add_elem(list, "Elem 2", NULL);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_column_width(table, 0, 100);
    tableview_header_title(table, 0, "Column 0");
    tableview_column_width(table, 1, 100);
    tableview_header_title(table, 1, "Column 1");
    tableview_OnData(table, listener(data, i_OnTableData, i_Data));
    tableview_update(table);
    layout_button(layout1, button, 0, 0);
    layout_popup(layout1, popup, 1, 0);
    layout_edit(layout1, edit, 2, 0);
    layout_combo(layout1, combo, 3, 0);
    layout_slider(layout1, slider, 4, 0);
    layout_updown(layout1, updown, 5, 0);
    layout_view(layout2, view, 0, 0);
    layout_textview(layout2, text, 1, 0);
    layout_listbox(layout2, list, 2, 0);
    layout_tableview(layout2, table, 3, 0);
    layout_tabstop(layout1, 0, 0, (bool_t)(i == 0));  /* Button */
    layout_tabstop(layout1, 1, 0, (bool_t)(i == 0));  /* PopUp */
    layout_tabstop(layout1, 2, 0, (bool_t)(i == 0));  /* Edit */
    layout_tabstop(layout1, 3, 0, (bool_t)(i == 0));  /* Combo */
    layout_tabstop(layout1, 4, 0, (bool_t)(i == 0));  /* Slider */
    layout_tabstop(layout1, 5, 0, FALSE);   /* Updown */
    layout_tabstop(layout2, 0, 0, (bool_t)(i == 0));  /* View */
    layout_tabstop(layout2, 1, 0, (bool_t)(i == 0));  /* TextView */
    layout_tabstop(layout2, 2, 0, (bool_t)(i == 0));  /* ListView */
    layout_tabstop(layout2, 3, 0, (bool_t)(i == 0));  /* TableView */
    layout_hmargin(layout1, 0, 10);
    layout_hmargin(layout1, 1, 10);
    layout_hmargin(layout1, 2, 10);
    layout_hmargin(layout1, 3, 10);
    layout_hmargin(layout1, 4, 10);
    layout_hmargin(layout2, 0, 10);
    layout_hmargin(layout2, 1, 10);
    layout_hmargin(layout2, 2, 10);
    layout_vmargin(layout, 0, 10);
    layout_layout(layout, layout1, 0, 0);
    layout_layout(layout, layout2, 0, 1);
    layout_halign(layout, 0, 0, ekLEFT);
    layout_halign(layout, 0, 1, ekLEFT);
    data->button[i] = button;
    data->popup[i] = popup;
    data->text[i] = text;
    data->view[i] = view;
    data->updown[i] = updown;
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel *tabstops(void)
{
    i_Data *data = heap_new0(i_Data);
    Layout *layout = layout_create(1, 4);
    Layout *layout1 = i_layout(data, 0);
    Layout *layout2 = i_layout(data, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Panel *panel = panel_create();
    label_text(label1, "Controls in the TabList");
    label_text(label2, "Controls NOT in the TabList");
    layout_label(layout, label1, 0, 0);
    layout_layout(layout, layout1, 0, 1);
    layout_label(layout, label2, 0, 2);
    layout_layout(layout, layout2, 0, 3);
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 1, 10);
    layout_vmargin(layout, 2, 10);
    panel_data(panel, &data, i_destroy_data, i_Data);
    panel_layout(panel, layout);
    return panel;
}
