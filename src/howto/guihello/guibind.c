/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: guibind.c
 *
 */

/* GUI data binding */

#include "guibind.h"
#include "guiall.h"

typedef struct _basictypes_t BasicTypes;

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

struct _basictypes_t
{
    bool_t bool_val;
    uint16_t uint16_val;
    real32_t real32_val;
    myenum_t enum_val;
    state_t enum3_val;
    String* str_val;
};

#define i_NUM_CONTROLS 9

/*---------------------------------------------------------------------------*/

static void i_destroy_data(BasicTypes **data)
{
    str_destroy(&(*data)->str_val);
    heap_delete(data, BasicTypes);
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

static void i_title_labels(Layout* layout)
{
    Font* font = font_system(font_regular_size(), ekFBOLD);
    const char_t* strs[] = { "Label", "EditBox", "Check", "Check3", "Radio", "PopUp", "ListBox", "Slider", "UpDown" };
    uint32_t i = 0;
    for (i = 0; i < i_NUM_CONTROLS; ++i)
    {
        Label* label = label_create();
        label_text(label, strs[i]);
        label_font(label, font);
        layout_label(layout, label, 0, i);
    }

    layout_hmargin(layout, 0, 10);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static void i_value_labels(Layout* layout)
{
    uint32_t i = 0;
    for (i = 0; i < i_NUM_CONTROLS; ++i)
    {
        Label* label = label_create();
        label_align(label, ekCENTER);
        layout_label(layout, label, 2, i);
        layout_halign(layout, 2, i, ekJUSTIFY);
    }

    layout_hsize(layout, 2, 80);
    layout_hmargin(layout, 0, 10);
    for (i = 0; i < i_NUM_CONTROLS - 1; ++i)
        layout_vmargin(layout, i, 5);

    cell_dbind(layout_cell(layout, 2, 0), BasicTypes, String*, str_val);
    cell_dbind(layout_cell(layout, 2, 1), BasicTypes, String*, str_val);
    cell_dbind(layout_cell(layout, 2, 2), BasicTypes, bool_t, bool_val);
    cell_dbind(layout_cell(layout, 2, 3), BasicTypes, state_t, enum3_val);
    cell_dbind(layout_cell(layout, 2, 4), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout, 2, 5), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout, 2, 6), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout, 2, 7), BasicTypes, real32_t, real32_val);
    cell_dbind(layout_cell(layout, 2, 8), BasicTypes, real32_t, real32_val);
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(void)
{
    Layout *layout = layout_create(3, 9);
    Label *label = label_create();
    Edit *edit = edit_create();
    Button *check = button_check();
    Button *check3 = button_check3();
    Layout *radios = i_radio_layout();
    PopUp *popup = popup_create();
    ListBox *listbox = listbox_create();
    Slider *slider = slider_create();
    UpDown *updown = updown_create();
    layout_label(layout, label, 1, 0);
    layout_edit(layout, edit, 1, 1);
    layout_button(layout, check, 1, 2);
    layout_button(layout, check3, 1, 3);
    layout_layout(layout, radios, 1, 4);
    layout_popup(layout, popup, 1, 5);
    layout_listbox(layout, listbox, 1, 6);
    layout_slider(layout, slider, 1, 7);
    layout_updown(layout, updown, 1, 8);
    layout_halign(layout, 1, 0, ekJUSTIFY);
    layout_halign(layout, 1, 8, ekLEFT);
    cell_dbind(layout_cell(layout, 1, 0), BasicTypes, String*, str_val);
    cell_dbind(layout_cell(layout, 1, 1), BasicTypes, String*, str_val);
    cell_dbind(layout_cell(layout, 1, 2), BasicTypes, bool_t, bool_val);
    cell_dbind(layout_cell(layout, 1, 3), BasicTypes, state_t, enum3_val);
    cell_dbind(layout_cell(layout, 1, 4), BasicTypes, uint16_t, uint16_val);
    cell_dbind(layout_cell(layout, 1, 5), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout, 1, 6), BasicTypes, myenum_t, enum_val);
    cell_dbind(layout_cell(layout, 1, 7), BasicTypes, real32_t, real32_val);
    cell_dbind(layout_cell(layout, 1, 8), BasicTypes, real32_t, real32_val);
    i_title_labels(layout);
    i_value_labels(layout);
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel* guibind(void)
{
    Layout *layout = NULL;
    Panel *panel = NULL;
    BasicTypes *data = heap_new(BasicTypes);
    data->bool_val = TRUE;
    data->uint16_val = 4;
    data->real32_val = 15.5f;
    data->enum3_val = ekMIXED;
    data->enum_val = ekCYAN;
    data->str_val = str_c("Text String");

    dbind_enum(state_t, ekOFF, "");
    dbind_enum(state_t, ekON, "");
    dbind_enum(state_t, ekMIXED, "");
    dbind_enum(myenum_t, ekRED, "Red");
    dbind_enum(myenum_t, ekBLUE, "Blue");
    dbind_enum(myenum_t, ekGREEN, "Green");
    dbind_enum(myenum_t, ekBLACK, "Black");
    dbind_enum(myenum_t, ekMAGENTA, "Magenta");
    dbind_enum(myenum_t, ekCYAN, "Cyan");
    dbind_enum(myenum_t, ekYELLOW, "Yellow");
    dbind_enum(myenum_t, ekWHITE, "While");
    dbind(BasicTypes, bool_t, bool_val);
    dbind(BasicTypes, uint16_t, uint16_val);
    dbind(BasicTypes, real32_t, real32_val);
    dbind(BasicTypes, state_t, enum3_val);
    dbind(BasicTypes, myenum_t, enum_val);
    dbind(BasicTypes, String*, str_val);
    dbind_range(BasicTypes, real32_t, real32_val, -50, 50);
    dbind_increment(BasicTypes, real32_t, real32_val, 5);

    layout = i_layout();
    panel = panel_create();
    layout_dbind(layout, NULL, BasicTypes);
    layout_dbind_obj(layout, data, BasicTypes);
    panel_data(panel, &data, i_destroy_data, BasicTypes);
    panel_layout(panel, layout);
    return panel;
}
