/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: sublayout.c
 *
 */

/* Sublayouts */
    
#include "sublayout.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

static Layout *i_updown_layout(void)
{
    Layout *layout = layout_create(2, 1);
    Label *label = label_create();
    UpDown *updown = updown_create();
    label_text(label, "UpDown");
    layout_label(layout, label, 0, 0);
    layout_updown(layout, updown, 1, 0);
    layout_hexpand(layout, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_left_grid_layout(void)
{
    Layout *layout1 = layout_create(2, 4);
    Layout *layout2 = i_updown_layout();
    Label *label = label_create();
    Button *button1 = button_push();
    Button *button2 = button_check();
    Slider *slider = slider_create();
    PopUp *popup = popup_create();
    Edit *edit = edit_create();
    Progress *progress = progress_create();
    label_text(label, "Hello!, I'm a label.");
    button_text(button1, "Push Button");
    button_text(button2, "Check Button");
    popup_add_elem(popup, "Option 1", NULL);
    popup_add_elem(popup, "Option 2", NULL);
    popup_add_elem(popup, "Option 3", NULL);
    popup_add_elem(popup, "Option 4", NULL);
    progress_undefined(progress, TRUE);
    layout_label(layout1, label, 0, 0);
    layout_button(layout1, button1, 0, 1);
    layout_button(layout1, button2, 0, 2);
    layout_slider(layout1, slider, 0, 3);
    layout_popup(layout1, popup, 1, 0);
    layout_edit(layout1, edit, 1, 1);
    layout_layout(layout1, layout2, 1, 2);
    layout_progress(layout1, progress, 1, 3);
    layout_hsize(layout1, 0, 150);
    layout_hsize(layout1, 1, 150);
    layout_hmargin(layout1, 0, 5);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_left_layout(void)
{
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = i_left_grid_layout();
    Button *button = button_push();
    button_text(button, "Clear");
    layout_layout(layout1, layout2, 0, 0);
    layout_button(layout1, button, 0, 1);
    layout_vmargin(layout1, 0, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_top_layout(void)
{
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = i_left_layout();
    TextView *view = textview_create();
    layout_layout(layout1, layout2, 0, 0);
    layout_textview(layout1, view, 1, 0);
    layout_hsize(layout1, 1, 230);
    layout_hmargin(layout1, 0, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_bottom_layout(void)
{
    Layout *layout = layout_create(6, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    label_text(label1, "Select 1");
    label_text(label2, "Select 2");
    label_text(label3, "Select 3");
    label_text(label4, "Select 4");
    label_text(label5, "Select 5");
    label_text(label6, "Select 6");
    label_style_over(label1, ekFUNDERLINE);
    label_style_over(label2, ekFUNDERLINE);
    label_style_over(label3, ekFUNDERLINE);
    label_style_over(label4, ekFUNDERLINE);
    label_style_over(label5, ekFUNDERLINE);
    label_style_over(label6, ekFUNDERLINE);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 1, 0);
    layout_label(layout, label3, 2, 0);
    layout_label(layout, label4, 3, 0);
    layout_label(layout, label5, 4, 0);
    layout_label(layout, label6, 5, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_main_layout(void)
{
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = i_top_layout();
    Layout *layout3 = i_bottom_layout();
    layout_layout(layout1, layout2, 0, 0);
    layout_layout(layout1, layout3, 0, 1);
    layout_margin(layout1, 5);
    layout_vmargin(layout1, 0, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

Panel *sublayouts(void)
{
    Panel *panel = panel_create();
    Layout *layout = i_main_layout();
    panel_layout(panel, layout);
    return panel;
}
