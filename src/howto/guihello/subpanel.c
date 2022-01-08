/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: subpanel.c
 *
 */

/* Use of subpanels */

#include "subpanel.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

Panel *subpanels(void)
{
    Panel *panel1 = panel_create();
    Panel *panel2 = panel_create();
    Layout *layout1 = layout_create(2, 2);
    Layout *layout2 = layout_create(2, 2);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Button *button = button_push();
    Slider *slider = slider_create();
    Edit *edit = edit_create();
    label_text(label1, "Main Panel");
    label_text(label2, "Subpanel");
    button_text(button, "Push Button");
    edit_text(edit, "EditBox");

    layout_label(layout2, label2, 0, 0);
    layout_button(layout2, button, 0, 1);
    layout_slider(layout2, slider, 1, 1);
    layout_hsize(layout2, 1, 150);
    layout_hmargin(layout2, 0, 10);
    layout_vmargin(layout2, 0, 10);
    layout_margin4(layout2, 5, 10, 10, 10);
    layout_skcolor(layout2, gui_label_color());
    panel_layout(panel2, layout2);

    layout_label(layout1, label1, 0, 0);
    layout_edit(layout1, edit, 1, 1);
    layout_panel(layout1, panel2, 0, 1);
    layout_hsize(layout1, 1, 100);
    layout_hmargin(layout1, 0, 10);
    layout_vmargin(layout1, 0, 10);
    layout_margin4(layout1, 5, 10, 10, 10);
    panel_layout(panel1, layout1);
    return panel1;
}
