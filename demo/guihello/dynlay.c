/* Dynamic layouts */

#include "dynlay.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct dyn_data_t DynData;

struct dyn_data_t
{
    Layout *top_layout;
    Layout *bottom_layout;
    uint32_t hmargin;
    uint32_t vmargin;
    color_t bgcolor[3];
    color_t skcolor[3];
};

/*---------------------------------------------------------------------------*/

static void i_destroy_dyndata(DynData **data)
{
    heap_delete(data, DynData);
}

/*---------------------------------------------------------------------------*/

static void i_top_layout_margins(DynData *data)
{
    uint32_t i, ncols, nrows;
    cassert_no_null(data);
    ncols = layout_ncols(data->top_layout);
    nrows = layout_nrows(data->top_layout);
    cassert(ncols >= 2); /* At lest one column and expand column */
    cassert(nrows >= 1); /* At lest one row */
    if (ncols > 2)
    {
        for (i = 0; i < ncols - 2; ++i)
            layout_hmargin(data->top_layout, i, (real32_t)data->hmargin);
    }

    if (nrows > 1)
    {
        for (i = 0; i < nrows - 1; ++i)
            layout_vmargin(data->top_layout, i, (real32_t)data->vmargin);
    }
}

/*---------------------------------------------------------------------------*/

static void i_bottom_layout_margins(DynData *data)
{
    uint32_t i, nrows;
    cassert_no_null(data);
    nrows = layout_nrows(data->bottom_layout);
    cassert(nrows >= 1); /* At lest one row */

    if (nrows > 1)
    {
        for (i = 0; i < nrows - 1; ++i)
            layout_vmargin(data->bottom_layout, i, 5);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnVMargin(DynData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cassert_no_null(data);
    if (p->index == 1 && data->vmargin > 0)
        data->vmargin -= 1;
    else if (p->index == 0)
        data->vmargin += 1;
    i_top_layout_margins(data);
    layout_update(data->top_layout);
}

/*---------------------------------------------------------------------------*/

static void i_OnHMargin(DynData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    cassert_no_null(data);
    if (p->index == 1 && data->hmargin > 0)
        data->hmargin -= 1;
    else if (p->index == 0)
        data->hmargin += 1;
    i_top_layout_margins(data);
    layout_update(data->top_layout);
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout_1(DynData *data)
{
    Layout *layout = layout_create(5, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    UpDown *updown1 = updown_create();
    UpDown *updown2 = updown_create();
    label_text(label1, "V-Margin:");
    label_text(label2, "H-Margin:");
    updown_OnClick(updown1, listener(data, i_OnVMargin, DynData));
    updown_OnClick(updown2, listener(data, i_OnHMargin, DynData));
    layout_label(layout, label1, 0, 0);
    layout_updown(layout, updown1, 1, 0);
    layout_label(layout, label2, 2, 0);
    layout_updown(layout, updown2, 3, 0);

    /*
     * Static margin between columns
     */
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 2, 5);

    /*
     * By default, updown and button cells are JUSTIFICABLE
     * We force keep the buttons into their original size
     */
    layout_halign(layout, 1, 0, ekLEFT);
    layout_halign(layout, 3, 0, ekLEFT);

    /*
     * The horizontal expansion is delegated to a fifth empty cell.
     * This prevents excess pixels from being distributed across all columns,
     * keeping the left four columns together.
     */
    layout_hexpand(layout, 4);

    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnSlider(DynData *data, Event *e)
{
    Slider *slider = event_sender(e, Slider);
    GuiControl *control = guicontrol(slider);
    uint32_t tag = guicontrol_get_tag(control);
    uint32_t col = tag & 0x0000FFFF;
    uint32_t row = ((tag & 0xFFFF0000) >> 16);
    uint32_t ncols = layout_ncols(data->top_layout);

    /* Exists a column at the right of slider */
    if (ncols > col + 1)
    {
        const EvSlider *p = event_params(e, EvSlider);
        /* The element at the right of slider is a progress always */
        Progress *progress = layout_get_progress(data->top_layout, col + 1, row);

        /* We syncro the progress with its neighbor slider */
        progress_value(progress, p->pos);
    }
}

/*---------------------------------------------------------------------------*/

static void i_fill_cell(Layout *layout, const uint32_t col, const uint32_t row, DynData *data)
{
    /* We are sure not to overwrite a cell */
    cassert(cell_empty(layout_cell(layout, col, row)) == TRUE);

    switch (col % 5)
    {
    case 0:
    {
        Label *label = label_create();
        String *text = str_printf("Label (%d, %d)", col, row);
        label_text(label, tc(text));
        layout_label(layout, label, col, row);
        str_destroy(&text);
        break;
    }

    case 1:
    {
        Button *button = button_push();
        String *text = str_printf("Button (%d, %d)", col, row);
        button_text(button, tc(text));
        layout_button(layout, button, col, row);
        str_destroy(&text);
        break;
    }

    case 2:
    {
        Slider *slider = slider_create();
        GuiControl *control = guicontrol(slider);
        real32_t pos = bmath_randf(0, 1);
        uint32_t tag = (row << 16) | col;
        slider_value(slider, pos);
        slider_OnMoved(slider, listener(data, i_OnSlider, DynData));
        guicontrol_tag(control, tag);
        layout_slider(layout, slider, col, row);
        break;
    }

    case 3:
    {
        /* Progress is in syncro with its left slider */
        Progress *progress = progress_create();
        Slider *slider = layout_get_slider(layout, col - 1, row);
        real32_t pos = slider_get_value(slider);
        progress_value(progress, pos);
        layout_progress(layout, progress, col, row);
        break;
    }

    case 4:
    {
        PopUp *popup = popup_create();
        uint32_t i;
        for (i = 0; i < 5; ++i)
        {
            String *text = str_printf("Item (%d, %d) - %d", col, row, i);
            popup_add_elem(popup, tc(text), NULL);
            str_destroy(&text);
        }
        layout_popup(layout, popup, col, row);
        break;
    }
    }
}

/*---------------------------------------------------------------------------*/

static void i_fill_row(Layout *layout, const uint32_t row, DynData *data)
{
    uint32_t i, cols = layout_ncols(layout);
    cassert(cols > 1);
    for (i = 0; i < cols - 1; ++i)
        i_fill_cell(layout, i, row, data);
}

/*---------------------------------------------------------------------------*/

static void i_fill_col(Layout *layout, const uint32_t col, DynData *data)
{
    uint32_t i, rows = layout_nrows(layout);
    cassert(rows > 1);
    for (i = 0; i < rows; ++i)
        i_fill_cell(layout, col, i, data);
}

/*---------------------------------------------------------------------------*/

static const Image *i_image(const uint32_t row)
{
    switch (row % 6)
    {
    case 0:
        return gui_image(FOLDER64_PNG);
    case 1:
        return gui_image(DISK64_PNG);
    case 2:
        return gui_image(SEARCH64_PNG);
    case 3:
        return gui_image(EDIT64_PNG);
    case 4:
        return gui_image(PLUS64_PNG);
    case 5:
        return gui_image(ERROR64_PNG);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_title_text(const uint32_t row)
{
    switch (row % 6)
    {
    case 0:
        return "Title: Folder";
    case 1:
        return "Title: Disk";
    case 2:
        return "Title: Search";
    case 3:
        return "Title: Edit";
    case 4:
        return "Title: Plus";
    case 5:
        return "Title: Error";
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_desc_text(const uint32_t row)
{
    switch (row % 6)
    {
    case 0:
        return "Desc: This is a folder icon";
    case 1:
        return "Desc: This is a disk icon";
    case 2:
        return "Desc: This is a search icon";
    case 3:
        return "Desc: This is a edit icon";
    case 4:
        return "Desc: This is a plus icon";
    case 5:
        return "Desc: This is a error icon";
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_price_text(const uint32_t row)
{
    switch (row % 6)
    {
    case 0:
        return "Price: 45.12€";
    case 1:
        return "Price: 12.34€";
    case 2:
        return "Price: 66.19€";
    case 3:
        return "Price: 22.65€";
    case 4:
        return "Price: 99.99€";
    case 5:
        return "Price: 32.56€";
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_stock_text(const uint32_t row)
{
    switch (row % 6)
    {
    case 0:
        return "Stock: 25";
    case 1:
        return "Stock: 18";
    case 2:
        return "Stock: 10";
    case 3:
        return "Stock: 22";
    case 4:
        return "Stock:  7";
    case 5:
        return "Stock:  0";
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static void i_fill_sublayout(Layout *layout, const uint32_t row, DynData *data)
{
    /*
     * layout1 is a new row-sublayout added to bottom_layout
     * composed by tree cells:
     * Image cell
     * Data cell: vertical sublayout with 4 labels in a stack
     * Expand cell: Only for expand the excess of pixels of wider sublayout (top_layout)
     */
    Layout *layout1 = layout_create(3, 1);
    Layout *layout2 = layout_create(1, 4);
    ImageView *view = imageview_create();
    const Image *image = i_image(row);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    /* We are sure not to overwrite a cell */
    cassert(cell_empty(layout_cell(layout, 0, row)) == TRUE);
    imageview_image(view, image);
    layout_imageview(layout1, view, 0, 0);
    label_text(label1, i_title_text(row));
    label_text(label2, i_desc_text(row));
    label_text(label3, i_price_text(row));
    label_text(label4, i_stock_text(row));
    layout_label(layout2, label1, 0, 0);
    layout_label(layout2, label2, 0, 1);
    layout_label(layout2, label3, 0, 2);
    layout_label(layout2, label4, 0, 3);
    layout_layout(layout1, layout2, 1, 0);
    layout_valign(layout1, 0, 0, ekTOP);
    layout_valign(layout1, 1, 0, ekTOP);
    layout_hmargin(layout1, 0, 10);
    layout_hexpand(layout1, 2);
    layout_margin(layout1, 10);
    layout_bgcolor(layout1, data->bgcolor[row % 3]);
    layout_skcolor(layout1, data->skcolor[row % 3]);
    layout_layout(layout, layout1, 0, row);
}

/*---------------------------------------------------------------------------*/

static void i_OnTopAddCol(DynData *data, Event *e)
{
    uint32_t ncols = 0;
    cassert_no_null(data);
    ncols = layout_ncols(data->top_layout);
    cassert(ncols > 1);
    unref(e);

    /* Insert new column in penultimate position. The last is the empty-resizable column */
    /* Because empty cells with 0-margin are added, the visual appearance does not change after insert */
    layout_insert_col(data->top_layout, ncols - 1);

    /* Add the new widget to recent-created cells */
    i_fill_col(data->top_layout, ncols - 1, data);

    /* Update the margins, because the new column has 0-margin */
    i_top_layout_margins(data);

    /* Recompute the layout appearance and update widgets */
    layout_update(data->top_layout);
}

/*---------------------------------------------------------------------------*/

static void i_OnTopDelCol(DynData *data, Event *e)
{
    uint32_t ncols = 0;
    cassert_no_null(data);
    ncols = layout_ncols(data->top_layout);
    unref(e);
    if (ncols > 3)
    {
        layout_remove_col(data->top_layout, ncols - 2);

        /* Update the margins, because the new column has 0-margin */
        i_top_layout_margins(data);

        /* Recompute the layout appearance and update widgets */
        layout_update(data->top_layout);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnTopAddRow(DynData *data, Event *e)
{
    uint32_t nrows = 0;
    cassert_no_null(data);
    nrows = layout_nrows(data->top_layout);
    cassert(nrows >= 1);
    unref(e);

    /* Insert new row in last position */
    /* Because empty cells with 0-margin are added, the visual appearance does not change after insert */
    layout_insert_row(data->top_layout, nrows);

    /* Add the new widget to recent-created cells */
    i_fill_row(data->top_layout, nrows, data);

    /* Update the margins, because the new row has 0-margin */
    i_top_layout_margins(data);

    /* Recompute the layout appearance and update widgets */
    layout_update(data->top_layout);
}

/*---------------------------------------------------------------------------*/

static void i_OnTopDelRow(DynData *data, Event *e)
{
    uint32_t nrows = 0;
    cassert_no_null(data);
    nrows = layout_nrows(data->top_layout);
    unref(e);
    if (nrows > 2)
    {
        layout_remove_row(data->top_layout, nrows - 1);

        /* Update the margins, because the new column has 0-margin */
        i_top_layout_margins(data);

        /* Recompute the layout appearance and update widgets */
        layout_update(data->top_layout);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnTopAddLayout(DynData *data, Event *e)
{
    uint32_t nrows = 0;
    cassert_no_null(data);
    nrows = layout_nrows(data->bottom_layout);
    cassert(nrows >= 1);
    unref(e);

    /* Insert new row in last position */
    layout_insert_row(data->bottom_layout, nrows);

    /* Add a new sublayout to last cell */
    i_fill_sublayout(data->bottom_layout, nrows, data);

    /* Update the margins, because the new row has 0-margin */
    i_bottom_layout_margins(data);

    /* Recompute the layout appearance and update widgets */
    layout_update(data->bottom_layout);
}

/*---------------------------------------------------------------------------*/

static void i_OnTopDelLayout(DynData *data, Event *e)
{
    uint32_t nrows = 0;
    cassert_no_null(data);
    nrows = layout_nrows(data->bottom_layout);
    unref(e);

    if (nrows > 1)
    {
        /* Remove the row in last position */
        layout_remove_row(data->bottom_layout, nrows - 1);

        /* Update the margins, because the new row has 0-margin */
        i_bottom_layout_margins(data);

        /* Recompute the layout appearance and update widgets */
        layout_update(data->bottom_layout);
    }
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout_2(DynData *data)
{
    Layout *layout = layout_create(7, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    label_text(label1, "Top columns");
    label_text(label2, "Top rows");
    button_text(button1, "Add");
    button_text(button2, "Remove");
    button_text(button3, "Add");
    button_text(button4, "Remove");
    button_OnClick(button1, listener(data, i_OnTopAddCol, DynData));
    button_OnClick(button2, listener(data, i_OnTopDelCol, DynData));
    button_OnClick(button3, listener(data, i_OnTopAddRow, DynData));
    button_OnClick(button4, listener(data, i_OnTopDelRow, DynData));
    layout_label(layout, label1, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_label(layout, label2, 3, 0);
    layout_button(layout, button3, 4, 0);
    layout_button(layout, button4, 5, 0);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 2, 5);
    layout_hmargin(layout, 3, 5);
    layout_hmargin(layout, 4, 5);
    layout_halign(layout, 1, 0, ekLEFT);
    layout_halign(layout, 2, 0, ekLEFT);
    layout_halign(layout, 4, 0, ekLEFT);
    layout_halign(layout, 5, 0, ekLEFT);
    layout_hexpand(layout, 6);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout_3(DynData *data)
{
    Layout *layout = layout_create(4, 1);
    Label *label = label_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    label_text(label, "Bottom sublayouts");
    button_text(button1, "Add");
    button_text(button2, "Remove");
    button_OnClick(button1, listener(data, i_OnTopAddLayout, DynData));
    button_OnClick(button2, listener(data, i_OnTopDelLayout, DynData));
    layout_label(layout, label, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_halign(layout, 1, 0, ekLEFT);
    layout_halign(layout, 2, 0, ekLEFT);
    layout_hexpand(layout, 3);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_control_layout(DynData *data)
{
    Layout *layout = layout_create(1, 3);
    Layout *layout1 = i_control_layout_1(data);
    Layout *layout2 = i_control_layout_2(data);
    Layout *layout3 = i_control_layout_3(data);
    layout_layout(layout, layout1, 0, 0);
    layout_layout(layout, layout2, 0, 1);
    layout_layout(layout, layout3, 0, 2);
    layout_vmargin(layout, 0, 3);
    layout_vmargin(layout, 1, 3);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_top_layout(DynData *data)
{
    Layout *layout = layout_create(5, 3);
    i_fill_row(layout, 0, data);
    i_fill_row(layout, 1, data);
    i_fill_row(layout, 2, data);
    layout_hexpand(layout, 4);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_bottom_layout(DynData *data)
{
    Layout *layout = layout_create(1, 3);
    i_fill_sublayout(layout, 0, data);
    i_fill_sublayout(layout, 1, data);
    i_fill_sublayout(layout, 2, data);
    return layout;
}

/*---------------------------------------------------------------------------*/

static DynData *i_panel_data(void)
{
    DynData *data = heap_new0(DynData);
    data->hmargin = 5;
    data->vmargin = 0;

    /* Alternative colors for Light and Dark themes */
    data->bgcolor[0] = gui_alt_color(color_rgb(255, 232, 232), color_rgb(128, 0, 0));
    data->bgcolor[1] = gui_alt_color(color_rgb(232, 255, 232), color_rgb(0, 128, 0));
    data->bgcolor[2] = gui_alt_color(color_rgb(232, 232, 255), color_rgb(0, 0, 128));
    data->skcolor[0] = gui_alt_color(color_rgb(255, 85, 0), color_rgb(255, 0, 0));
    data->skcolor[1] = gui_alt_color(color_rgb(5, 163, 0), color_rgb(0, 255, 0));
    data->skcolor[2] = gui_alt_color(color_rgb(109, 0, 163), color_rgb(0, 0, 255));
    return data;
}

/*---------------------------------------------------------------------------*/
/* 
* Dynamic layouts example
* 
* The main layout is a stack with three rows: 
* Control Layout: Buttons to add/remove dynamic layouts cells 
* Top Layout: Grid layout with simple widgets where we can add/remove columns/rows dynamically 
* Bottom Layout: A stack where we can add/remove complex sublayouts dynamically
* 
* Main layout lives in a scroll panel with fixed size. 
* When main layout grows, scrollbars will be activated to browse all the content.
*
*/
/*---------------------------------------------------------------------------*/
Panel *dynlay_panel(void)
{
    DynData *data = i_panel_data();
    Panel *panel = panel_scroll(TRUE, TRUE);
    Layout *layout = layout_create(1, 4);
    Layout *control_layout = i_control_layout(data);
    Layout *top_layout = i_top_layout(data);
    Layout *bottom_layout = i_bottom_layout(data);
    data->top_layout = top_layout;
    data->bottom_layout = bottom_layout;

    /* Main layout composition */
    layout_layout(layout, control_layout, 0, 0);
    layout_layout(layout, top_layout, 0, 1);
    layout_layout(layout, bottom_layout, 0, 2);

    /* Grid layout dynamic margins */
    i_top_layout_margins(data);

    /* Articles layout margins */
    i_bottom_layout_margins(data);

    /* Static vertical separation between three layouts */
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 1, 10);

    /* 
    * The main layout vertical expansion is delegated to a fourth empty cell.
    * This prevents excess pixels from being distributed across all rows, 
    * keeping the top three rows together.
    */
    layout_vexpand(layout, 3);

    /* 
     * Main container fixed size 
     * Scrollbars will be activated when layouts grow
     */
    panel_size(panel, s2df(400, 500));

    /* Panel-Layout binding */
    panel_layout(panel, layout);

    /* DynData is a dynamic structure that will be destroyed the Panel destroys */
    panel_data(panel, &data, i_destroy_dyndata, DynData);
    return panel;
}
