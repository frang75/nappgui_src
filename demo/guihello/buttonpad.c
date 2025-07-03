/* Button padding */

#include "buttonpad.h"
#include "res_guihello.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

static Layout *i_vpadding_layout(void)
{
    Font *font = font_system(20, 0);
    Layout *layout = layout_create(1, 3);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    button_text(button1, "Button with default padding");
    button_text(button2, "Button with zero padding");
    button_text(button3, "Button with high padding");
    button_font(button2, font);
    button_image(button1, gui_image(FOLDER24_PNG));
    button_image(button2, gui_image(FOLDER24_PNG));
    button_image(button3, gui_image(FOLDER24_PNG));
    button_vpadding(button2, 0);
    button_vpadding(button3, 30);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    font_destroy(&font);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_hpadding_layout(void)
{
    Layout *layout = layout_create(1, 5);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Button *button5 = button_push();
    button_text(button1, "Default padding");
    button_text(button2, "Zero padding");
    button_text(button3, "High padding");
    button_text(button4, "Min Width");
    button_text(button5, "Expand");
    button_hpadding(button2, 0);
    button_hpadding(button3, 50);
    button_min_width(button4, 150);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_button(layout, button5, 0, 4);
    layout_hsize(layout, 0, 300);
    layout_halign(layout, 0, 0, ekLEFT);
    layout_halign(layout, 0, 1, ekLEFT);
    layout_halign(layout, 0, 2, ekLEFT);
    layout_halign(layout, 0, 3, ekLEFT);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_flatpadding_layout(void)
{
    Layout *layout = layout_create(3, 2);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Button *button1 = button_flatgle();
    Button *button2 = button_flatgle();
    Button *button3 = button_flatgle();
    label_text(label1, "Default padding");
    label_text(label2, "Zero padding");
    label_text(label3, "High padding");
    button_image(button1, gui_image(FOLDER24_PNG));
    button_image(button2, gui_image(FOLDER24_PNG));
    button_image(button3, gui_image(FOLDER24_PNG));
    button_hpadding(button2, 0);
    button_vpadding(button2, 0);
    button_hpadding(button3, 30);
    button_vpadding(button3, 30);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 1, 0);
    layout_label(layout, label3, 2, 0);
    layout_button(layout, button1, 0, 1);
    layout_button(layout, button2, 1, 1);
    layout_button(layout, button3, 2, 1);
    layout_halign(layout, 0, 1, ekCENTER);
    layout_halign(layout, 1, 1, ekCENTER);
    layout_halign(layout, 2, 1, ekCENTER);
    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout, 1, 10);
    return layout;
}

/*---------------------------------------------------------------------------*/

Panel *buttonpad(void)
{
    Layout *layout1 = layout_create(1, 6);
    Layout *layout2 = i_vpadding_layout();
    Layout *layout3 = i_hpadding_layout();
    Layout *layout4 = i_flatpadding_layout();
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Panel *panel = panel_create();
    label_text(label1, "Vertical padding");
    label_text(label2, "Horizontal padding");
    label_text(label3, "Flat padding");
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 2);
    layout_label(layout1, label3, 0, 4);
    layout_layout(layout1, layout2, 0, 1);
    layout_layout(layout1, layout3, 0, 3);
    layout_layout(layout1, layout4, 0, 5);
    layout_halign(layout1, 0, 1, ekLEFT);
    layout_halign(layout1, 0, 5, ekLEFT);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    layout_vmargin(layout1, 4, 5);
    panel_layout(panel, layout1);
    return panel;
}
