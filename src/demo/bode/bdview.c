/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bdview.c
 *
 */

/* Bode View */

#include "bdview.h"
#include "bdctrl.h"
#include "guiall.h"

static const real32_t kEDIT_WIDTH = 60;

/*---------------------------------------------------------------------------*/

static Cell *i_coeff(Layout *layout, const char_t *text, const uint32_t col, const uint32_t row)
{
    Label *label = label_create();
    Edit *edit = edit_create();
    label_text(label, text);
    edit_align(edit, ekRIGHT);
    layout_halign(layout, col * 2, row, ekCENTER);
    layout_label(layout, label, col * 2, row);
    layout_edit(layout, edit, col * 2 + 1, row);
    return layout_cell(layout, col * 2 + 1, row);
}

/*---------------------------------------------------------------------------*/

static Layout *i_coeffs(void)
{
    Layout *layout = layout_create(4, 9);
    cell_dbind(i_coeff(layout, "P[0]", 0, 0), Params, real32_t, P[0]);
    cell_dbind(i_coeff(layout, "P[1]", 1, 0), Params, real32_t, P[1]);
    cell_dbind(i_coeff(layout, "P[2]", 0, 1), Params, real32_t, P[2]);
    cell_dbind(i_coeff(layout, "P[3]", 1, 1), Params, real32_t, P[3]);
    cell_dbind(i_coeff(layout, "P[4]", 0, 2), Params, real32_t, P[4]);
    cell_dbind(i_coeff(layout, "Q[0]", 0, 3), Params, real32_t, Q[0]);
    cell_dbind(i_coeff(layout, "Q[1]", 1, 3), Params, real32_t, Q[1]);
    cell_dbind(i_coeff(layout, "Q[2]", 0, 4), Params, real32_t, Q[2]);
    cell_dbind(i_coeff(layout, "Q[3]", 1, 4), Params, real32_t, Q[3]);
    cell_dbind(i_coeff(layout, "Q[4]", 0, 5), Params, real32_t, Q[4]);
    cell_dbind(i_coeff(layout, "Q[5]", 1, 5), Params, real32_t, Q[5]);
    cell_dbind(i_coeff(layout, "Q[6]", 0, 6), Params, real32_t, Q[6]);
    cell_dbind(i_coeff(layout, "Q[7]", 1, 6), Params, real32_t, Q[7]);
    cell_dbind(i_coeff(layout, "Q[8]", 0, 7), Params, real32_t, Q[8]);
    cell_dbind(i_coeff(layout, "T", 0, 8), Params, real32_t, T);
    cell_dbind(i_coeff(layout, "R", 1, 8), Params, real32_t, R);
    layout_hsize(layout, 1, kEDIT_WIDTH);
    layout_hsize(layout, 3, kEDIT_WIDTH);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 10);
    layout_vmargin(layout, 3, 5);
    layout_vmargin(layout, 4, 5);
    layout_vmargin(layout, 5, 5);
    layout_vmargin(layout, 6, 5);
    layout_vmargin(layout, 7, 10);
    layout_hmargin(layout, 1, 5);
    layout_hmargin(layout, 0, 3);
    layout_hmargin(layout, 2, 3);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_range(Layout *layout, const char_t *text, const uint32_t i)
{
    Label *label = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    label_text(label, text);
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    layout_label(layout, label, 0, i);
    layout_edit(layout, edit1, 1, i);
    layout_edit(layout, edit2, 2, i);
}

/*---------------------------------------------------------------------------*/

static Layout *i_ranges(void)
{
    Layout *layout = layout_create(3, 3);
    i_range(layout, "I", 0);
    i_range(layout, "P", 1);
    i_range(layout, "D", 2);
    layout_hsize(layout, 1, kEDIT_WIDTH);
    layout_hsize(layout, 2, kEDIT_WIDTH);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    cell_dbind(layout_cell(layout, 1, 0), Params, real32_t, KRg[0]);
    cell_dbind(layout_cell(layout, 2, 0), Params, real32_t, KRg[1]);
    cell_dbind(layout_cell(layout, 1, 1), Params, real32_t, KRg[2]);
    cell_dbind(layout_cell(layout, 2, 1), Params, real32_t, KRg[3]);
    cell_dbind(layout_cell(layout, 1, 2), Params, real32_t, KRg[4]);
    cell_dbind(layout_cell(layout, 2, 2), Params, real32_t, KRg[5]);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_left(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 10);
    Layout *layout1 = i_coeffs();
    Button *button = button_push();
    Label *label = label_create();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Label *label2 = label_create();
    Layout *layout2 = i_ranges();
    Button *button5 = button_push();
    button_text(button, "Reset defaults");
    button_text(button2, "Take");
    button_text(button3, "Restore");
    button_text(button4, "Clear");
    button_text(button5, "More info");
    label_text(label, "SnapShot");
    label_text(label2, "Slider Range");
    layout_layout(layout, layout1, 0, 0);
    layout_button(layout, button, 0, 1);
    layout_label(layout, label, 0, 2);
    layout_button(layout, button2, 0, 3);
    layout_button(layout, button3, 0, 4);
    layout_button(layout, button4, 0, 5);
    layout_label(layout, label2, 0, 6);
    layout_layout(layout, layout2, 0, 7);
    layout_button(layout, button5, 0, 9);
    layout_halign(layout, 0, 7, ekLEFT);
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 1, 10);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 5);
    layout_vmargin(layout, 4, 5);
    layout_vmargin(layout, 5, 10);
    layout_vmargin(layout, 6, 5);
    layout_vmargin(layout, 7, 10);
    layout_vexpand(layout, 8);
    ctrl_reset(ctrl, button);
    ctrl_take(ctrl, layout_cell(layout, 0, 3));
    ctrl_restore(ctrl, layout_cell(layout, 0, 4));
    ctrl_clear(ctrl, layout_cell(layout, 0, 5));
    ctrl_info(ctrl, button5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_slider_K(Layout *layout, const char_t *title, const uint32_t row)
{
    Label *label = label_create();
    Slider* slider = slider_create();
    Edit* edit = edit_create();
    label_text(label, title);
    edit_align(edit, ekRIGHT);
    layout_label(layout, label, 0, row);
    layout_slider(layout, slider, 1, row);
    layout_edit(layout, edit, 2, row);
}

/*---------------------------------------------------------------------------*/

static Layout *i_sliders(Ctrl *ctrl)
{
    Layout *layout = layout_create(3, 3);
    i_slider_K(layout, "I", 0);
    i_slider_K(layout, "P", 1);
    i_slider_K(layout, "D", 2);
    layout_hsize(layout, 2, kEDIT_WIDTH);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 5);
    layout_hexpand(layout, 1);
    cell_dbind(layout_cell(layout, 1, 0), Params, real32_t, K[0]);
    cell_dbind(layout_cell(layout, 2, 0), Params, real32_t, K[0]);
    cell_dbind(layout_cell(layout, 1, 1), Params, real32_t, K[1]);
    cell_dbind(layout_cell(layout, 2, 1), Params, real32_t, K[1]);
    cell_dbind(layout_cell(layout, 1, 2), Params, real32_t, K[2]);
    cell_dbind(layout_cell(layout, 2, 2), Params, real32_t, K[2]);
    ctrl_slider1(ctrl, layout_cell(layout, 1, 0));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout* i_right(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 3);
    Layout* layout1 = i_sliders(ctrl);
    View* view1 = view_create();
    View* view2 = view_create();
    layout_view(layout, view1, 0, 0);
    layout_view(layout, view2, 0, 1);
    layout_layout(layout, layout1, 0, 2);
    layout_vmargin(layout, 0, 2);
    layout_vmargin(layout, 1, 5);
    layout_vexpand2(layout, 0, 1, .5f);
    ctrl_view1(ctrl, view1);
    ctrl_view2(ctrl, view2);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(Ctrl *ctrl)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(2, 1);
    Layout *layout1 = i_left(ctrl);
    Layout* layout2 = i_right(ctrl);
    layout_layout(layout, layout1, 0, 0);
    layout_layout(layout, layout2, 1, 0);
    layout_hmargin(layout, 0, 5);
    layout_hexpand(layout, 1);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    layout_dbind(layout1, NULL, Params);
    layout_dbind(layout2, NULL, Params);
    cell_dbind(layout_cell(layout, 0, 0), Model, Params, cparams);
    cell_dbind(layout_cell(layout, 1, 0), Model, Params, cparams);
    layout_dbind(layout, listener(ctrl, ctrl_OnModelChange, Ctrl), Model);
    ctrl_layout(ctrl, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

Window* bdview_create(Ctrl *ctrl)
{
    Panel *panel = i_panel(ctrl);
    Window *window = window_create(ekWINDOW_STDRES);
    window_panel(window, panel);
    window_title(window, "Bode plot");
    return window;
}
