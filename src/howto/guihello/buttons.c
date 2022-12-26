/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: buttons.c
 *
 */

/* Buttons demo */

#include "buttons.h"
#include "guiall.h"
#include "res_guihello.h"

/*---------------------------------------------------------------------------*/

static Layout *i_flatbuttons(void)
{
    Layout *layout = layout_create(6, 1);
    Button *button1 = button_flat();
    Button *button2 = button_flat();
    Button *button3 = button_flat();
    Button *button4 = button_flat();
    Button *button5 = button_flat();
    Button *button6 = button_flat();
    button_text(button1, "Open File");
    button_text(button2, "Save File");
    button_text(button3, "Search File");
    button_text(button4, "Edit File");
    button_text(button5, "Add File");
    button_text(button6, "Delete File");
    button_image(button1, (const Image*)FOLDER24_PNG);
    button_image(button2, (const Image*)DISK24_PNG);
    button_image(button3, (const Image*)SEARCH24_PNG);
    button_image(button4, (const Image*)EDIT24_PNG);
    button_image(button5, (const Image*)PLUS24_PNG);
    button_image(button6, (const Image*)ERROR24_PNG);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 1, 0);
    layout_button(layout, button3, 2, 0);
    layout_button(layout, button4, 3, 0);
    layout_button(layout, button5, 4, 0);
    layout_button(layout, button6, 5, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_radios(void)
{
    Layout *layout = layout_create(1, 4);
    Button *radio1 = button_radio();
    Button *radio2 = button_radio();
    Button *radio3 = button_radio();
    Button *radio4 = button_radio();
    button_text(radio1, "Wireframe");
    button_text(radio2, "Shaded");
    button_text(radio3, "Realistic");
    button_text(radio4, "V-Ray");
    button_state(radio1, ekGUI_ON);
    layout_button(layout, radio1, 0, 0);
    layout_button(layout, radio2, 0, 1);
    layout_button(layout, radio3, 0, 2);
    layout_button(layout, radio4, 0, 3);
    layout_margin(layout, 5);
    layout_vmargin(layout, 0, 3);
    layout_vmargin(layout, 1, 3);
    layout_vmargin(layout, 2, 3);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_checks(void)
{
    Layout *layout = layout_create(1, 4);
    Button *check1 = button_check();
    Button *check2 = button_check();
    Button *check3 = button_check();
    Button *check4 = button_check();
    button_text(check1, "Lines");
    button_text(check2, "Meshes");
    button_text(check3, "Materials");
    button_text(check4, "Lights");
    button_state(check1, ekGUI_ON);
    button_state(check2, ekGUI_OFF);
    button_state(check3, ekGUI_OFF);
    button_state(check4, ekGUI_ON);
    layout_button(layout, check1, 0, 0);
    layout_button(layout, check2, 0, 1);
    layout_button(layout, check3, 0, 2);
    layout_button(layout, check4, 0, 3);
    layout_margin(layout, 5);
    layout_vmargin(layout, 0, 3);
    layout_vmargin(layout, 1, 3);
    layout_vmargin(layout, 2, 3);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_pushes(void)
{
    Layout *layout = layout_create(4, 1);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    button_text(button1, "Retry");
    button_text(button2, "Cancel");
    button_text(button3, "Ok");
    button_image(button1, (const Image*)RETRY_PNG);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 2, 0);
    layout_button(layout, button3, 3, 0);
    layout_hmargin(layout, 2, 5);
    layout_hexpand(layout, 1);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_buttons(void)
{
    Layout *layout = layout_create(1, 3);
    Layout *layout1 = i_flatbuttons();
    Layout *layout2 = layout_create(2, 2);
    Layout *layout3 = i_radios();
    Layout *layout4 = i_checks();
    Layout *layout5 = i_pushes();
    Button *check1 = button_check();
    Button *check2 = button_check3();
    button_text(check1, "Enable 3D Render");
    button_text(check2, "Enable Preview Settings");
    button_state(check1, ekGUI_ON);
    button_state(check2, ekGUI_MIXED);
    layout_layout(layout, layout1, 0, 0);
    layout_button(layout2, check1, 0, 0);
    layout_layout(layout2, layout3, 0, 1);
    layout_button(layout2, check2, 1, 0);
    layout_layout(layout2, layout4, 1, 1);
    layout_layout(layout, layout2, 0, 1);
    layout_layout(layout, layout5, 0, 2);
    layout_halign(layout, 0, 0, ekLEFT);
    layout_margin(layout2, 5);
    layout_hmargin(layout2, 0, 10);
    layout_margin(layout5, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel *buttons_basics(void)
{
    Layout *layout = i_buttons();
    Panel *panel = panel_create();
    panel_layout(panel, layout);
    return panel;
}

