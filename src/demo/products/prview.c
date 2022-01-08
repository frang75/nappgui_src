/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: prview.c
 *
 */

/* Products View */

#include "prview.h"
#include "prctrl.h"
#include "guiall.h"
#include "res_gui.h"

/*---------------------------------------------------------------------------*/

static Layout *i_toolbar(Ctrl *ctrl)
{
    Layout *layout = layout_create(8, 1);
    Button *button0 = button_flat();
    Button *button1 = button_flat();
    Button *button2 = button_flat();
    Button *button3 = button_flat();
    Button *button4 = button_flat();
    Button *button5 = button_flat();
    Button *button6 = button_flatgle();
    Combo *combo = combo_create();
    button_text(button0, TWIN_FIRST);
    button_text(button1, TWIN_BACK);
    button_text(button2, TWIN_NEXT);
    button_text(button3, TWIN_LAST);
    button_text(button4, TWIN_ADD);
    button_text(button5, TWIN_DEL);
    button_text(button6, TWIN_SETTINGS_PANEL);
    combo_tooltip(combo, TWIN_FILTER_DESC);
    combo_bgcolor_focus(combo, kEDITBG);
    combo_phtext(combo, TWIN_FILTER);
    combo_phcolor(combo, kHOLDER);
    combo_phstyle(combo, ekFITALIC | ekFUNDERLINE);
    layout_button(layout, button0, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_button(layout, button3, 3, 0);
    layout_button(layout, button4, 4, 0);
    layout_button(layout, button5, 5, 0);
    layout_combo(layout, combo, 6, 0);
    layout_button(layout, button6, 7, 0);
    layout_hmargin(layout, 5, 5);
    layout_hmargin(layout, 6, 5);
    layout_hexpand(layout, 6);
    ctrl_first_cell(ctrl, layout_cell(layout, 0, 0));
    ctrl_back_cell(ctrl, layout_cell(layout, 1, 0));
    ctrl_next_cell(ctrl, layout_cell(layout, 2, 0));
    ctrl_last_cell(ctrl, layout_cell(layout, 3, 0));
    ctrl_add_cell(ctrl, layout_cell(layout, 4, 0));
    ctrl_minus_cell(ctrl, layout_cell(layout, 5, 0));
    ctrl_filter_cell(ctrl, layout_cell(layout, 6, 0));
    ctrl_setting_cell(ctrl, layout_cell(layout, 7, 0));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_code_desc(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 4);
    Label *label0 = label_create();
    Label *label1 = label_create();
    Edit *edit0 = edit_create();
    Edit *edit1 = edit_multiline();
    label_text(label0, TWIN_CODE);
    label_text(label1, TWIN_DESC);
    edit_phtext(edit0, TWIN_TYPE_CODE);
    edit_phtext(edit1, TWIN_TYPE_DESC);
    edit_bgcolor_focus(edit0, kEDITBG);
    edit_bgcolor_focus(edit1, kEDITBG);
    edit_phcolor(edit0, kHOLDER);
    edit_phcolor(edit1, kHOLDER);
    edit_phstyle(edit0, ekFITALIC | ekFUNDERLINE);
    edit_phstyle(edit1, ekFITALIC | ekFUNDERLINE);
    layout_label(layout, label0, 0, 0);
    layout_edit(layout, edit0, 0, 1);
    layout_label(layout, label1, 0, 2);
    layout_edit(layout, edit1, 0, 3);
    layout_vmargin(layout, 1, 10);
    layout_vexpand(layout, 3);
    ctrl_code_cell(ctrl, layout_cell(layout, 0, 1));
    ctrl_desc_cell(ctrl, layout_cell(layout, 0, 3));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_type(void)
{
    Layout *layout = layout_create(4, 1);
    Button *button0 = button_radio();
    Button *button1 = button_radio();
    Button *button2 = button_radio();
    Button *button3 = button_radio();
    button_text(button0, TWIN_CPU);
    button_text(button1, TWIN_GPU);
    button_text(button2, TWIN_HDD);
    button_text(button3, TWIN_SCD);
    layout_button(layout, button0, 0, 0);
    layout_button(layout, button1, 1, 0);
    layout_button(layout, button2, 2, 0);
    layout_button(layout, button3, 3, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_n_img(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 2);
    Label *label = label_create();
    ImageView *view = imageview_create();
    label_align(label, ekCENTER);
    layout_halign(layout, 0, 0, ekJUSTIFY);
    layout_label(layout, label, 0, 0);
    layout_imageview(layout, view, 0, 1);
    layout_vexpand(layout, 1);
    ctrl_counter_cell(ctrl, layout_cell(layout, 0, 0));
    ctrl_image_cell(ctrl, layout_cell(layout, 0, 1));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_price(void)
{
    Layout *layout = layout_create(2, 1);
    Edit *edit = edit_create();
    Font *font = font_system(18, ekFBOLD);
    UpDown *updown = updown_create();
    edit_phtext(edit, TWIN_TYPE_PRICE);
    edit_font(edit, font);
    edit_align(edit, ekRIGHT);
    edit_color(edit, kTXTRED);
    edit_bgcolor_focus(edit, kEDITBG);
    edit_phcolor(edit, kHOLDER);
    edit_phstyle(edit, ekFITALIC | ekFUNDERLINE);
    layout_edit(layout, edit, 0, 0);
    layout_updown(layout, updown, 1, 0);
    layout_hsize(layout, 1, 24);
    layout_hexpand(layout, 0);
    font_destroy(&font);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_product(Ctrl *ctrl)
{
    Layout *layout = layout_create(2, 3);
    Layout *layout0 = i_code_desc(ctrl);
    Layout *layout1 = i_type();
    Layout *layout2 = i_n_img(ctrl);
    Layout *layout3 = i_price();
    Label *label0 = label_create();
    Label *label1 = label_create();
    label_text(label0, TWIN_TYPE);
    label_text(label1, TWIN_PRICE);
    layout_layout(layout, layout0, 0, 0);
    layout_label(layout, label0, 0, 1);
    layout_layout(layout, layout1, 0, 2);
    layout_layout(layout, layout2, 1, 0);
    layout_label(layout, label1, 1, 1);
    layout_layout(layout, layout3, 1, 2);
    layout_halign(layout, 1, 1, ekRIGHT);
    layout_hsize(layout, 1, 200);
    layout_vsize(layout, 0, 200);
    layout_hmargin(layout, 0, 10);
    layout_vmargin(layout, 0, 10);
    layout_margin4(layout, 0, 10, 10, 10);
    layout_vexpand(layout, 0);
    ctrl_type_cell(ctrl, layout_cell(layout, 0, 2));
    ctrl_price_cell(ctrl, layout_cell(layout, 1, 2));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_form(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 3);
    Layout *layout0 = i_toolbar(ctrl);
    Layout *layout1 = i_product(ctrl);
    Slider *slider = slider_create();
    Cell *cell = NULL;
    layout_layout(layout, layout0, 0, 0);
    layout_slider(layout, slider, 0, 1);
    layout_layout(layout, layout1, 0, 2);
    layout_vexpand(layout, 2);
    cell = layout_cell(layout, 0, 1);
    cell_padding4(cell, 0, 10, 0, 10);
    ctrl_slider_cell(ctrl, cell);    
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_login(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 10);
    Label *label0 = label_create();
    Label *label1 = label_multiline();
    Label *label2 = label_create();
    Label *label3 = label_create();
    PopUp *popup0 = popup_create();
    ImageView *view0 = imageview_create();
    Edit *edit0 = edit_create();
    Edit *edit1 = edit_create();
    Button *button = button_push();
    label_text(label0, TWIN_SETLANG);
    label_text(label1, TWIN_LOGIN_MSG);
    label_text(label2, TWIN_USER);
    label_text(label3, TWIN_PASS);
    popup_add_elem(popup0, ENGLISH, (const Image*)USA_PNG);
    popup_add_elem(popup0, SPANISH, (const Image*)SPAIN_PNG);
    popup_add_elem(popup0, PORTUGUESE, (const Image*)PORTUGAL_PNG);
    popup_add_elem(popup0, ITALIAN, (const Image*)ITALY_PNG);
    popup_add_elem(popup0, VIETNAMESE, (const Image*)VIETNAM_PNG);
    popup_add_elem(popup0, RUSSIAN, (const Image*)RUSSIA_PNG);
    popup_add_elem(popup0, JAPANESE, (const Image*)JAPAN_PNG);
    popup_tooltip(popup0, TWIN_SETLANG);   
    imageview_image(view0, (const Image*)USER_PNG);
    edit_passmode(edit1, TRUE);
    button_text(button, TWIN_LOGIN);
    layout_label(layout, label0, 0, 0);
    layout_popup(layout, popup0, 0, 1);
    layout_label(layout, label1, 0, 2);
    layout_imageview(layout, view0, 0, 3);
    layout_label(layout, label2, 0, 4);
    layout_edit(layout, edit0, 0, 5);
    layout_label(layout, label3, 0, 6);
    layout_edit(layout, edit1, 0, 7);
    layout_button(layout, button, 0, 9);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 10);
    layout_vmargin(layout, 2, 10);
    layout_vmargin(layout, 5, 5);
    layout_vmargin(layout, 8, 5);
    layout_margin4(layout, 5, 10, 10, 10);
    layout_hsize(layout, 0, 200);
    layout_vexpand(layout, 8);
    ctrl_lang_cell(ctrl, layout_cell(layout, 0, 1));
    ctrl_user_cell(ctrl, layout_cell(layout, 0, 5));
    ctrl_pass_cell(ctrl, layout_cell(layout, 0, 7));
    ctrl_login_cell(ctrl, layout_cell(layout, 0, 9));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_logout(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 6);
    ImageView *view = imageview_create();
    Label *label0 = label_create();
    Label *label1 = label_create();
    View *cview = view_create();
    Button *button = button_push();
    label_align(label0, ekCENTER);
    label_align(label1, ekCENTER);
    button_text(button, TWIN_LOGOUT);
    view_size(cview, s2df(160, 160));
    layout_imageview(layout, view, 0, 0);
    layout_label(layout, label0, 0, 1);
    layout_label(layout, label1, 0, 2);
    layout_view(layout, cview, 0, 3);
    layout_button(layout, button, 0, 5);
    layout_halign(layout, 0, 1, ekJUSTIFY);
    layout_halign(layout, 0, 2, ekJUSTIFY);
    layout_halign(layout, 0, 3, ekCENTER);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 2, 5);
    layout_vexpand(layout, 4);
    layout_hsize(layout, 0, 200);
    layout_margin(layout, 10);
    ctrl_stats_cell(ctrl, layout_cell(layout, 0, 3));
    ctrl_logout_cell(ctrl, layout_cell(layout, 0, 5));
    return layout;
}

/*---------------------------------------------------------------------------*/

static Panel *i_login_panel(Ctrl *ctrl)
{
    Panel *panel = panel_create();
    Layout *layout0 = i_login(ctrl);
    Layout *layout1 = i_logout(ctrl);
    panel_layout(panel, layout0);
    panel_layout(panel, layout1);
    ctrl_login_panel(ctrl, panel);
    return panel;
}

/*---------------------------------------------------------------------------*/

static Layout *i_status_bar(Ctrl *ctrl)
{
    Layout *layout = layout_create(2, 1);
    ImageView *view = imageview_create();
    Label *label = label_create();
    imageview_size(view, s2df(16, 16));
    layout_imageview(layout, view, 0, 0);
    layout_label(layout, label, 1, 0);
    layout_halign(layout, 1, 0, ekJUSTIFY);
    layout_hexpand(layout, 1);
    layout_hmargin(layout, 0, 5);
    layout_margin(layout, 5);
    layout_bgcolor(layout, kSTATBG);
    layout_skcolor(layout, kSTATSK);
    ctrl_status_layout(ctrl, layout);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(Ctrl *ctrl)
{
    Layout *layout = layout_create(1, 2);
    Layout *layout0 = layout_create(2, 1);
    Layout *layout1 = i_form(ctrl);
    Layout *layout2 = i_status_bar(ctrl);
    Panel *panel1 = i_login_panel(ctrl);
    layout_layout(layout0, layout1, 0, 0);
    layout_panel(layout0, panel1, 1, 0);
    layout_layout(layout, layout0, 0, 0);
    layout_layout(layout, layout2, 0, 1);
    ctrl_main_layout(ctrl, layout0);
    return layout;
}

/*---------------------------------------------------------------------------*/

Window *prview_create(Ctrl *ctrl)
{
    Panel *panel = panel_create();
    Layout *layout = i_layout(ctrl);
    Window *window = NULL;
    ctrl_theme_images(ctrl);
    panel_layout(panel, layout);
    window = window_create(ekWNSTD);
    window_panel(window, panel);
    window_title(window, TWIN_TITLE);
    ctrl_window(ctrl, window);
    return window;
}
