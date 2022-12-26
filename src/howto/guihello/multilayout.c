/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: multilayout.c
 *
 */

/* Panels with multiple layouts */

#include "multilayout.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

static Panel *i_multilayout_panel(void)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(2, 5);
    Layout *layout2 = layout_create(1, 10);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
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
    edit_text(edit1, "Amanda Callister");
    edit_text(edit2, "aQwe56nhjJk");
    edit_text(edit3, "35, Tuam Road");
    edit_text(edit4, "Galway - Ireland");
    edit_text(edit5, "+35 654 333 000");
    edit_passmode(edit2, TRUE);

    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 1);
    layout_label(layout1, label3, 0, 2);
    layout_label(layout1, label4, 0, 3);
    layout_label(layout1, label5, 0, 4);
    layout_edit(layout1, edit1, 1, 0);
    layout_edit(layout1, edit2, 1, 1);
    layout_edit(layout1, edit3, 1, 2);
    layout_edit(layout1, edit4, 1, 3);
    layout_edit(layout1, edit5, 1, 4);
    layout_hsize(layout1, 1, 300);
    layout_hmargin(layout1, 0, 5);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);

    layout_label(layout2, label1, 0, 0);
    layout_label(layout2, label2, 0, 2);
    layout_label(layout2, label3, 0, 4);
    layout_label(layout2, label4, 0, 6);
    layout_label(layout2, label5, 0, 8);
    layout_edit(layout2, edit1, 0, 1);
    layout_edit(layout2, edit2, 0, 3);
    layout_edit(layout2, edit3, 0, 5);
    layout_edit(layout2, edit4, 0, 7);
    layout_edit(layout2, edit5, 0, 9);
    layout_hsize(layout2, 0, 200);
    layout_vmargin(layout2, 1, 5);
    layout_vmargin(layout2, 3, 5);
    layout_vmargin(layout2, 5, 5);
    layout_vmargin(layout2, 7, 5);

    panel_layout(panel, layout1);
    panel_layout(panel, layout2);
    return panel;
}

/*---------------------------------------------------------------------------*/

static void i_OnLayout(Panel *panel, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    panel_visible_layout(panel, params->index);
    panel_update(panel);
}

/*---------------------------------------------------------------------------*/

Panel *multilayouts(void)
{
    Panel *panel1 = panel_create();
    Panel *panel2 = i_multilayout_panel();
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = layout_create(2, 1);
    button_text(button1, "Layout1");
    button_text(button2, "Layout2");
    button_state(button1, ekGUI_ON);
    button_OnClick(button1, listener(panel2, i_OnLayout, Panel));
    layout_button(layout2, button1, 0, 0);
    layout_button(layout2, button2, 1, 0);
    layout_layout(layout1, layout2, 0, 0);
    layout_panel(layout1, panel2, 0, 1);
    layout_vmargin(layout1, 0, 10);
    layout_hmargin(layout2, 0, 10);
    layout_halign(layout1, 0, 0, ekLEFT);
    panel_layout(panel1, layout1);
    return panel1;
}
