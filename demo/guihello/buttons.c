/* Buttons demo */

#include "buttons.h"
#include "res_guihello.h"
#include <gui/guiall.h>

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
    button_image(button1, gui_image(FOLDER24_PNG));
    button_image(button2, gui_image(DISK24_PNG));
    button_image(button3, gui_image(SEARCH24_PNG));
    button_image(button4, gui_image(EDIT24_PNG));
    button_image(button5, gui_image(PLUS24_PNG));
    button_image(button6, gui_image(ERROR24_PNG));
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
    button_text(radio1, "&Wireframe");
    button_text(radio2, "&Shaded");
    button_text(radio3, "&Realistic");
    button_text(radio4, "&V-Ray");
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
    button_text(check1, "&Lines");
    button_text(check2, "M&eshes");
    button_text(check3, "M&aterials");
    button_text(check4, "L&ights");
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

static Layout *i_pushes(Button **defbutton)
{
    Layout *layout = layout_create(4, 1);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    button_text(button1, "Re&try");
    button_text(button2, "&Cancel");
    button_text(button3, "&Ok");
    button_image(button1, gui_image(RETRY_PNG));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 2, 0);
    layout_button(layout, button3, 3, 0);
    layout_hmargin(layout, 2, 5);
    layout_hexpand(layout, 1);
    *defbutton = button1;
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_buttons(Button **defbutton)
{
    Layout *layout = layout_create(1, 3);
    Layout *layout1 = i_flatbuttons();
    Layout *layout2 = layout_create(2, 2);
    Layout *layout3 = i_radios();
    Layout *layout4 = i_checks();
    Layout *layout5 = i_pushes(defbutton);
    Button *check1 = button_check();
    Button *check2 = button_check3();
    button_text(check1, "Enable 3&D Render");
    button_text(check2, "Enable &Preview Settings");
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

Panel *buttons_basics(Button **defbutton)
{
    Layout *layout = i_buttons(defbutton);
    Panel *panel = panel_create();
    panel_layout(panel, layout);
    return panel;
}
