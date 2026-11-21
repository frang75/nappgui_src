/* GUI data binding */

#include "guibind.h"
#include <gui/guiall.h>

typedef enum _myenum_t
{
    ekRED,
    ekBLUE,
    ekGREEN,
    ekBLACK,
    ekMAGENTA,
    ekCYAN,
    ekYELLOW,
    ekWHITE
} myenum_t;

typedef struct _basictypes_t BasicTypes;

struct _basictypes_t
{
    bool_t bool_val;
    uint16_t uint16_val;
    real32_t real32_val;
    myenum_t enum_val;
    gui_state_t enum3_val;
    int32_t date_val;
    String *str_val;
    String *date_str;
};

/*---------------------------------------------------------------------------*/

static const char_t *i_COLORS[] = {"Red", "Blue", "Green", "Black", "Magenta", "Cyan", "Yellow", "White"};

/*---------------------------------------------------------------------------*/

static void i_data_bind(void)
{
    dbind_enum(gui_state_t, ekGUI_OFF, "");
    dbind_enum(gui_state_t, ekGUI_ON, "");
    dbind_enum(gui_state_t, ekGUI_MIXED, "");
    dbind_enum(myenum_t, ekRED, "Red");
    dbind_enum(myenum_t, ekBLUE, "Blue");
    dbind_enum(myenum_t, ekGREEN, "Green");
    dbind_enum(myenum_t, ekBLACK, "Black");
    dbind_enum(myenum_t, ekMAGENTA, "Magenta");
    dbind_enum(myenum_t, ekCYAN, "Cyan");
    dbind_enum(myenum_t, ekYELLOW, "Yellow");
    dbind_enum(myenum_t, ekWHITE, "White");
    dbind(BasicTypes, bool_t, bool_val);
    dbind(BasicTypes, uint16_t, uint16_val);
    dbind(BasicTypes, real32_t, real32_val);
    dbind(BasicTypes, gui_state_t, enum3_val);
    dbind(BasicTypes, myenum_t, enum_val);
    dbind(BasicTypes, int32_t, date_val);
    dbind(BasicTypes, String *, str_val);
    dbind(BasicTypes, String *, date_str);
    dbind_range(BasicTypes, real32_t, real32_val, -50, 50);
    dbind_increment(BasicTypes, real32_t, real32_val, 5);
}

/*---------------------------------------------------------------------------*/

static void i_date_str(BasicTypes *data)
{
    cassert_no_null(data);
    str_destopt(&data->date_str);
    if (data->date_val != INT32_MIN)
    {
        Date date = date_from_days(data->date_val);
        data->date_str = str_printf("%d/%02d/%02d", date.year, date.month, date.mday);
    }
    else
    {
        data->date_str = str_c("No date");
    }
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(BasicTypes **data)
{
    str_destroy(&(*data)->str_val);
    str_destroy(&(*data)->date_str);
    heap_delete(data, BasicTypes);
    dbind_unreg(BasicTypes);
    dbind_unreg(myenum_t);
}

/*---------------------------------------------------------------------------*/

static void i_OnDataChange(BasicTypes *data, Event *e)
{
    Layout *layout = event_sender(e, Layout);
    cassert(event_type(e) == ekGUI_EVENT_OBJCHANGE);
    if (evbind_modify(e, BasicTypes, int32_t, date_val) == TRUE)
    {
        i_date_str(data);
        layout_dbind_update(layout, BasicTypes, String *, date_str);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnNoDate(Layout *layout, Event *e)
{
    BasicTypes *data = layout_dbind_get_obj(layout, BasicTypes);
    cassert_no_null(data);
    unref(e);
    data->date_val = INT32_MIN;
    i_date_str(data);
    layout_dbind_update(layout, BasicTypes, int32_t, date_val);
    layout_dbind_update(layout, BasicTypes, String *, date_str);
}

/*---------------------------------------------------------------------------*/

static void i_OnTableData(TableView *view, Event *e)
{
    unref(view);
    switch (event_type(e))
    {
    case ekGUI_EVENT_TBL_NROWS:
    {
        uint32_t *n = event_result(e, uint32_t);
        *n = sizeof(i_COLORS) / sizeof(char_t *);
        break;
    }

    case ekGUI_EVENT_TBL_CELL:
    {
        const EvTbPos *pos = event_params(e, EvTbPos);
        EvTbCell *cell = event_result(e, EvTbCell);
        cell->text = i_COLORS[pos->row];
        cell->align = ekLEFT;
        break;
    }

    default:
        break;
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_radio_layout(void)
{
    uint32_t i = 0, n = 6;
    Layout *layout = layout_create(1, n);
    for (i = 0; i < n; ++i)
    {
        Button *radio = button_radio();
        char_t str[64];
        bstd_sprintf(str, sizeof(str), "Radio %d", i + 1);
        button_text(radio, str);
        layout_button(layout, radio, 0, i);
    }

    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_title_labels(Layout *layout, const char_t **strs, const uint32_t n)
{
    Font *font = font_system(font_regular_size(), ekFBOLD);
    uint32_t i = 0;
    for (i = 0; i < n; ++i)
    {
        Label *label = label_create();
        label_text(label, strs[i]);
        label_font(label, font);
        layout_label(layout, label, 0, i);
    }

    layout_hmargin(layout, 0, 10);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_value_labels(Layout *layout, const uint32_t n)
{
    uint32_t i = 0;
    for (i = 0; i < n; ++i)
    {
        Label *label = label_create();
        label_align(label, ekCENTER);
        layout_label(layout, label, 2, i);
        layout_halign(layout, 2, i, ekJUSTIFY);
    }

    layout_hsize(layout, 2, 80);
    layout_hmargin(layout, 1, 10);
}

/*---------------------------------------------------------------------------*/

static Layout *i_left_layout(void)
{
    const char_t *strs[] = {"Label", "EditBox", "Check", "Check3", "Radio", "PopUp", "ComboBox", "ComboBox (index)", "Slider", "UpDown"};
    Layout *layout1 = layout_create(3, 10);
    Layout *layout2 = i_radio_layout();
    Label *label = label_create();
    Edit *edit = edit_create();
    Button *check = button_check();
    Button *check3 = button_check3();
    PopUp *popup = popup_create();
    Combo *combo1 = combo_create();
    Combo *combo2 = combo_create();
    Slider *slider = slider_create();
    UpDown *updown = updown_create();
    uint32_t i = 0;
    combo_add_elem(combo1, "Text String", NULL);
    combo_add_elem(combo1, "Hello World", NULL);
    combo_add_elem(combo1, "NAppGUI", NULL);
    for (i = 0; i < 6; ++i)
    {
        char_t str[64];
        bstd_sprintf(str, sizeof(str), "Option %d", i + 1);
        combo_add_elem(combo2, str, NULL);
    }
    layout_label(layout1, label, 1, 0);
    layout_edit(layout1, edit, 1, 1);
    layout_button(layout1, check, 1, 2);
    layout_button(layout1, check3, 1, 3);
    layout_layout(layout1, layout2, 1, 4);
    layout_popup(layout1, popup, 1, 5);
    layout_combo(layout1, combo1, 1, 6);
    layout_combo(layout1, combo2, 1, 7);
    layout_slider(layout1, slider, 1, 8);
    layout_updown(layout1, updown, 1, 9);
    i_title_labels(layout1, strs, 10);
    i_value_labels(layout1, 10);
    cell_dbind(layout_cell(layout1, 1, 0), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 1, 1), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 1, 2), BasicTypes, bool_t, bool_val);
    cell_dbind(layout_cell(layout1, 1, 3), BasicTypes, gui_state_t, enum3_val);
    cell_dbind(layout_cell(layout1, 1, 4), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout1, 1, 5), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout1, 1, 6), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 1, 7), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout1, 1, 8), BasicTypes, real32_t, real32_val);
    cell_dbind(layout_cell(layout1, 1, 9), BasicTypes, real32_t, real32_val);
    cell_dbind(layout_cell(layout1, 2, 0), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 2, 1), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 2, 2), BasicTypes, bool_t, bool_val);
    cell_dbind(layout_cell(layout1, 2, 3), BasicTypes, gui_state_t, enum3_val);
    cell_dbind(layout_cell(layout1, 2, 4), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout1, 2, 5), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout1, 2, 6), BasicTypes, String *, str_val);
    cell_dbind(layout_cell(layout1, 2, 7), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout1, 2, 8), BasicTypes, real32_t, real32_val);
    cell_dbind(layout_cell(layout1, 2, 9), BasicTypes, real32_t, real32_val);
    layout_halign(layout1, 1, 0, ekJUSTIFY);
    layout_halign(layout1, 1, 9, ekLEFT);
    for (i = 0; i < 9; ++i)
        layout_vmargin(layout1, i, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_date_layout(Layout *layout_main)
{
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = layout_create(2, 1);
    Label *label = label_create();
    Edit *edit = edit_create();
    UpDown *updown = updown_create();
    Button *button = button_push();
    button_text(button, "No date");
    button_OnClick(button, listener(layout_main, i_OnNoDate, Layout));
    layout_label(layout1, label, 0, 0);
    layout_layout(layout1, layout2, 0, 1);
    layout_button(layout1, button, 0, 2);
    layout_edit(layout2, edit, 0, 0);
    layout_updown(layout2, updown, 1, 0);
    cell_dbind(layout_cell(layout1, 0, 0), BasicTypes, String *, date_str);
    cell_dbind(layout_cell(layout2, 0, 0), BasicTypes, int32_t, date_val);
    cell_dbind(layout_cell(layout2, 1, 0), BasicTypes, int32_t, date_val);
    layout_hsize(layout2, 0, 100);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_halign(layout1, 0, 0, ekCENTER);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_right_layout(Layout *layout_main)
{
    const char_t *strs[] = {"ListBox", "TableView", "DateView"};
    Layout *layout1 = layout_create(3, 3);
    Layout *layout2 = i_date_layout(layout_main);
    ListBox *listbox = listbox_create();
    TableView *tableview = tableview_create();
    DateView *dateview = dateview_create();
    tableview_add_column_text(tableview);
    tableview_header_title(tableview, 0, "Color");
    tableview_column_width(tableview, 0, 200);
    tableview_OnData(tableview, listener(tableview, i_OnTableData, TableView));
    tableview_update(tableview);
    layout_listbox(layout1, listbox, 1, 0);
    layout_tableview(layout1, tableview, 1, 1);
    layout_dateview(layout1, dateview, 1, 2);
    layout_layout(layout1, layout2, 2, 2);
    i_title_labels(layout1, strs, 3);
    i_value_labels(layout1, 2);
    cell_dbind(layout_cell(layout1, 1, 0), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout1, 1, 1), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout1, 1, 2), BasicTypes, int32_t, date_val);
    cell_dbind(layout_cell(layout1, 2, 0), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout1, 2, 1), BasicTypes, myenum_t, enum_val);
    layout_hsize(layout1, 2, 130);
    layout_vsize(layout1, 1, 150);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_valign(layout1, 2, 2, ekTOP);
    return layout1;
}

/*---------------------------------------------------------------------------*/

Panel *guibind(void)
{
    BasicTypes *data = heap_new(BasicTypes);
    Panel *panel = NULL;
    Layout *layout1 = NULL;
    Layout *layout2 = NULL;
    Layout *layout3 = NULL;
    Date today = date_system();
    i_data_bind();
    panel = panel_create();
    layout1 = layout_create(2, 1);
    layout2 = i_left_layout();
    layout3 = i_right_layout(layout1);
    data->bool_val = TRUE;
    data->uint16_val = 4;
    data->real32_val = 15.5f;
    data->enum3_val = ekGUI_MIXED;
    data->enum_val = ekCYAN;
    data->date_val = date_to_days(&today);
    data->str_val = str_c("Text String");
    data->date_str = NULL;
    i_date_str(data);
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 1, 0);
    layout_hmargin(layout1, 0, 20);
    layout_valign(layout1, 0, 0, ekTOP);
    layout_valign(layout1, 1, 0, ekTOP);
    panel_layout(panel, layout1);
    layout_dbind(layout1, listener(data, i_OnDataChange, BasicTypes), BasicTypes);
    layout_dbind_obj(layout1, data, BasicTypes);
    panel_data(panel, &data, i_destroy_data, BasicTypes);
    return panel;
}
