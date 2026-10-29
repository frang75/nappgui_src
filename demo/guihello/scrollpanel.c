/* Panel with scroll */

#include "scrollpanel.h"
#include <gui/guiall.h>

static const uint32_t i_ROWS_FEW = 10;
static const uint32_t i_ROWS_MANY = 20;

/*---------------------------------------------------------------------------*/

static Layout *i_rows_layout(const uint32_t nrows)
{
    Layout *layout = layout_create(3, nrows);
    uint32_t i = 0;

    for (i = 0; i < nrows; ++i)
    {
        char_t text[128];
        Label *label = label_create();
        Edit *edit = edit_create();
        Button *button = button_push();
        bstd_sprintf(text, sizeof(text), "User %d", i + 1);
        label_text(label, text);
        bstd_sprintf(text, sizeof(text), "Name of User %d", i + 1);
        edit_text(edit, text);
        bstd_sprintf(text, sizeof(text), "Edit %d", i + 1);
        button_text(button, text);
        layout_label(layout, label, 0, i);
        layout_edit(layout, edit, 1, i);
        layout_button(layout, button, 2, i);
    }

    for (i = 0; i < nrows - 1; ++i)
        layout_vmargin(layout, i, 5);

    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout, 1, 10);
    layout_hsize(layout, 1, 150);
    layout_hexpand(layout, 1);
    layout_margin4(layout, 0, 0, 0, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnRows(Panel *panel, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    panel_visible_layout(panel, params->index);
    layout_update(panel_get_layout(panel, params->index));
}

/*---------------------------------------------------------------------------*/

static void i_OnNarrow(Panel *panel, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    bool_t narrow = (bool_t)(params->state == ekGUI_ON);
    panel_scroll_mode(panel, narrow, narrow);
    panel_update(panel);
}

/*---------------------------------------------------------------------------*/

Panel *scrollpanel(void)
{
    Panel *panel1 = panel_create();
    Panel *panel2 = panel_scroll(FALSE, TRUE);
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = layout_create(2, 2);
    Layout *layout3 = i_rows_layout(i_ROWS_FEW);
    Layout *layout4 = i_rows_layout(i_ROWS_MANY);
    Button *radio1 = button_radio();
    Button *radio2 = button_radio();
    Button *check1 = button_check();
    button_text(radio1, "Few rows");
    button_text(radio2, "Many rows");
    button_text(check1, "Narrow content");
    button_state(radio2, ekGUI_ON);
    button_state(check1, ekGUI_ON);
    button_OnClick(radio2, listener(panel2, i_OnRows, Panel));
    button_OnClick(check1, listener(panel2, i_OnNarrow, Panel));
    panel_size(panel2, s2df(-1, 400));
    panel_layout(panel2, layout3);
    panel_layout(panel2, layout4);
    panel_visible_layout(panel2, 1);
    layout_button(layout2, radio1, 0, 0);
    layout_button(layout2, radio2, 1, 0);
    layout_button(layout2, check1, 0, 1);
    layout_hmargin(layout2, 0, 10);
    layout_vmargin(layout2, 0, 10);
    layout_layout(layout1, layout2, 0, 0);
    layout_panel(layout1, panel2, 0, 1);
    layout_halign(layout1, 0, 0, ekLEFT);
    layout_vmargin(layout1, 0, 10);
    panel_layout(panel1, layout1);
    return panel1;
}
