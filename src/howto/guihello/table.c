/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: table.c
 *
 */

/* Use of tables */

#include "table.h"
#include "guiall.h"

typedef struct _appdata_t AppData;

struct _appdata_t
{
    TableView *table;
    TextView *text;
    char_t temp_string[256];
};

/*---------------------------------------------------------------------------*/

static void i_destroy_appdata(AppData** data)
{
    heap_delete(data, AppData);
}

/*---------------------------------------------------------------------------*/

/* AppData must contain the real data access(array, stream, etc) */
static void i_OnTableData(AppData *data, Event *e)
{
    uint32_t etype = event_type(e);

    switch(etype) {
    case ekGUI_EVENT_TBL_NROWS:
    {
        uint32_t *n = event_result(e, uint32_t);
        *n = 100;
        break;
    }

    case ekGUI_EVENT_TBL_CELL:
    {
        const EvTbPos *pos = event_params(e, EvTbPos);
        EvTbCell *cell = event_result(e, EvTbCell);

        switch(pos->col) {
        case 0:
            cell->align = ekLEFT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Name %d", pos->row);
            break;

        case 1:
            cell->align = ekLEFT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Adress %d", pos->row);
            break;

        case 2:
            cell->align = ekLEFT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "City %d", pos->row);
            break;

        case 3:
            cell->align = ekRIGHT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "%d", pos->row);
            break;

        case 4:
            cell->align = ekRIGHT;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "%.2f", 10.5f + pos->row);
            break;

        case 5:
            cell->align = ekCENTER;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Extra Data 1 %d", pos->row);
            break;

        case 6:
            cell->align = ekCENTER;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Extra Data 2 %d", pos->row);
            break;

        case 7:
            cell->align = ekCENTER;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Extra Data 3 %d", pos->row);
            break;

        case 8:
            cell->align = ekCENTER;
            bstd_sprintf(data->temp_string, sizeof(data->temp_string), "Extra Data 4 %d", pos->row);
            break;

        cassert_default();
        }

        cell->text = data->temp_string;
        break;
    }
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnHeaderClick(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    textview_printf(data->text, "Click on Header: %d\n", p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnMultisel(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    if (p->index == 0)
        tableview_multisel(data->table, FALSE, FALSE);
    else if (p->index == 1)
        tableview_multisel(data->table, TRUE, FALSE);
    else if (p->index == 2)
        tableview_multisel(data->table, TRUE, TRUE);
}

/*---------------------------------------------------------------------------*/

static void i_OnResizeCheck(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    bool_t resizable = p->state == ekGUI_ON ? TRUE : FALSE;
    tableview_header_resizable(data->table, resizable);
}

/*---------------------------------------------------------------------------*/

static void i_OnHeaderCheck(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    bool_t clickable = p->state == ekGUI_ON ? TRUE : FALSE;
    tableview_header_clickable(data->table, clickable);
}

/*---------------------------------------------------------------------------*/

static void i_OnFreezeCheck(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    uint32_t col_freeze = p->state == ekGUI_ON ? 1 : UINT32_MAX;
    tableview_column_freeze(data->table, col_freeze);
}

/*---------------------------------------------------------------------------*/

static void i_OnGridCheck(AppData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    bool_t grid = p->state == ekGUI_ON ? TRUE : FALSE;
    tableview_grid(data->table, grid, grid);
}

/*---------------------------------------------------------------------------*/

static void i_OnPrintsel(AppData *data, Event *e)
{
    const ArrSt(uint32_t) *sel = tableview_selected(data->table);
    uint32_t n = arrst_size(sel, uint32_t);
    textview_writef(data->text, "Selected rows: ");
    arrst_foreach_const(row, sel, uint32_t)
        textview_printf(data->text, "%d", *row);
        if (row_i < n - 1)
            textview_writef(data->text, ", ");
    arrst_end();
    textview_writef(data->text, "\n");
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Layout* i_table_control_layout(AppData *data)
{
    Layout *layout1 = layout_create(3, 1);
    Layout *layout2 = layout_create(1, 6);
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Button *button3 = button_radio();
    Button *button4 = button_check();
    Button *button5 = button_check();
    Button *button6 = button_check();
    Button *button7 = button_check();
    Button *button8 = button_push();
    button_text(button1, "Single select");
    button_text(button2, "Multi select");
    button_text(button3, "Preserve select");
    button_text(button4, "Resizable headers");
    button_text(button5, "Clickable headers");
    button_text(button6, "Freeze 0 and 1 columns");
    button_text(button7, "Draw grid lines");
    button_text(button8, "Print selected rows");
    button_state(button1, ekGUI_ON);
    button_state(button4, ekGUI_ON);
    button_state(button5, ekGUI_ON);
    button_state(button6, ekGUI_ON);
    button_state(button7, ekGUI_ON);
    layout_button(layout1, button1, 0, 0);
    layout_button(layout1, button2, 1, 0);
    layout_button(layout1, button3, 2, 0);
    layout_layout(layout2, layout1, 0, 0);
    layout_button(layout2, button4, 0, 1);
    layout_button(layout2, button5, 0, 2);
    layout_button(layout2, button6, 0, 3);
    layout_button(layout2, button7, 0, 4);
    layout_button(layout2, button8, 0, 5);
    layout_hmargin(layout1, 0, 5.f);
    layout_hmargin(layout1, 1, 5.f);
    layout_vmargin(layout2, 0, 5.f);
    layout_vmargin(layout2, 1, 5.f);
    layout_vmargin(layout2, 2, 5.f);
    layout_vmargin(layout2, 3, 5.f);
    layout_vmargin(layout2, 4, 5.f);
    layout_halign(layout2, 0, 0, ekLEFT);
    layout_halign(layout2, 0, 5, ekLEFT);
    button_OnClick(button1, listener(data, i_OnMultisel, AppData));
    button_OnClick(button2, listener(data, i_OnMultisel, AppData));
    button_OnClick(button3, listener(data, i_OnMultisel, AppData));
    button_OnClick(button4, listener(data, i_OnResizeCheck, AppData));
    button_OnClick(button5, listener(data, i_OnHeaderCheck, AppData));
    button_OnClick(button6, listener(data, i_OnFreezeCheck, AppData));
    button_OnClick(button7, listener(data, i_OnGridCheck, AppData));
    button_OnClick(button8, listener(data, i_OnPrintsel, AppData));
    return layout2;
}

/*---------------------------------------------------------------------------*/

Panel *table_view(void)
{
    Panel *panel = panel_create();
    AppData *data = heap_new0(AppData);
    TableView *table = tableview_create();
    TextView *text = textview_create();
    Layout *layout1 = layout_create(1, 3);
    Layout *layout2 = i_table_control_layout(data);
    data->table = table;
    data->text = text;
    tableview_size(table, s2df(500, 300));
    tableview_OnData(table, listener(data, i_OnTableData, AppData));
    tableview_OnHeaderClick(table, listener(data, i_OnHeaderClick, AppData));
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_new_column_text(table);
    tableview_header_clickable(table, TRUE);
    tableview_header_resizable(table, TRUE);
    tableview_header_indicator(table, 1, ekINDDOWN_ARROW);
    tableview_header_indicator(table, 2, ekINDUP_ARROW);
    tableview_header_title(table, 0, "Name");
    tableview_header_title(table, 1, "Address");
    tableview_header_title(table, 2, "City");
    tableview_header_title(table, 3, "Age");
    tableview_header_title(table, 4, "Value");
    tableview_header_title(table, 5, "Extra Data 1");
    tableview_header_title(table, 6, "Extra Data 2");
    tableview_header_title(table, 7, "Extra Data 3");
    tableview_header_title(table, 8, "Extra Data 4");
    tableview_column_width(table, 0, 100);
    tableview_column_width(table, 1, 105);
    tableview_column_width(table, 2, 50);
    tableview_column_width(table, 3, 50);
    tableview_column_width(table, 4, 170);
    tableview_column_width(table, 5, 200);
    tableview_column_width(table, 6, 200);
    tableview_column_width(table, 7, 200);
    tableview_column_width(table, 8, 200);
    tableview_column_limits(table, 2, 50, 100);
    tableview_column_freeze(table, 1);
    tableview_header_align(table, 0, ekLEFT);
    tableview_header_align(table, 1, ekLEFT);
    tableview_header_align(table, 2, ekLEFT);
    tableview_header_align(table, 3, ekRIGHT);
    tableview_header_align(table, 4, ekRIGHT);
    tableview_header_align(table, 5, ekCENTER);
    tableview_header_align(table, 6, ekCENTER);
    tableview_header_align(table, 7, ekCENTER);
    tableview_header_align(table, 8, ekCENTER);
    tableview_multisel(table, FALSE, FALSE);
    tableview_header_visible(table, TRUE);
    tableview_grid(table, TRUE, TRUE);
    tableview_update(table);
    layout_layout(layout1, layout2, 0, 0);
    layout_tableview(layout1, table, 0, 1);
    layout_textview(layout1, text, 0, 2);
    layout_vmargin(layout1, 0, 5.f);
    layout_vmargin(layout1, 1, 5.f);
    panel_data(panel, &data, i_destroy_appdata, AppData);
    panel_layout(panel, layout1);
    return panel;
}
