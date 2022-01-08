/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: labels.c
 *
 */

/* Labels basics */
    
#include "labels.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

static const char_t *i_LABEL_01 = "Hello.";
static const char_t *i_LABEL_02 = "Hello, I'm a Label.";
static const char_t *i_LABEL_03 = "Hello, I'm a Label, longer than first.";
static const char_t *i_LABEL_04 = "Hello, I'm a Label, longer than first and longer than second.";
static const char_t *i_LABEL_05 = "Hello, I'm a Label, longer than first, longer than second and longer than third.";
static const char_t *i_LABEL_06 = "Hello, I'm a Label, longer than first, longer than second, longer than third and longer than fourth.";
static const char_t *i_LABEL_07 = "Mouse sensitive label";

/*---------------------------------------------------------------------------*/

static void i_OnLayoutWidth(Layout *layout, Event *event)
{
    const EvButton *p = event_params(event, EvButton);
    real32_t width = 0;
    switch (p->index) {
    case 0:
        width = 0;
        break;
    case 1:
        width = 100;
        break;
    case 2:
        width = 200;
        break;
    case 3:
        width = 300;
        break;
    case 4:
        width = 400;
        break;
    cassert_default();
    }

    layout_hsize(layout, 0, width);
    layout_update(layout);
}

/*---------------------------------------------------------------------------*/

static PopUp *i_width_popup(Layout *layout)
{
    PopUp *popup = popup_create();
    popup_add_elem(popup, "Natural", NULL);
    popup_add_elem(popup, "100px", NULL);
    popup_add_elem(popup, "200px", NULL);
    popup_add_elem(popup, "300px", NULL);
    popup_add_elem(popup, "400px", NULL);
    popup_OnSelect(popup, listener(layout, i_OnLayoutWidth, Layout));
    return popup;
}

/*---------------------------------------------------------------------------*/

Panel *labels_single_line(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 7);
    PopUp *popup = i_width_popup(layout);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    color_t c1 = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    color_t c2 = gui_alt_color(color_rgb(255, 192, 255), color_rgb(128, 48, 112));
    color_t c3 = gui_alt_color(color_rgb(255, 255, 192), color_rgb(112, 112, 48));
    label_text(label1, i_LABEL_01);
    label_text(label2, i_LABEL_02);
    label_text(label3, i_LABEL_03);
    label_text(label4, i_LABEL_04);
    label_text(label5, i_LABEL_05);
    label_text(label6, i_LABEL_06);
    label_bgcolor(label1, c1);
    label_bgcolor(label2, c2);
    label_bgcolor(label3, c3);
    label_bgcolor(label4, c1);
    label_bgcolor(label5, c2);
    label_bgcolor(label6, c3);
    layout_popup(layout, popup, 0, 0);
    layout_label(layout, label1, 0, 1);
    layout_label(layout, label2, 0, 2);
    layout_label(layout, label3, 0, 3);
    layout_label(layout, label4, 0, 4);
    layout_label(layout, label5, 0, 5);
    layout_label(layout, label6, 0, 6);
    layout_vmargin(layout, 0, 5);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

Panel *labels_multi_line(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 7);
    PopUp *popup = i_width_popup(layout);
    Label *label1 = label_multiline();
    Label *label2 = label_multiline();
    Label *label3 = label_multiline();
    Label *label4 = label_multiline();
    Label *label5 = label_multiline();
    Label *label6 = label_multiline();
    color_t c1 = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    color_t c2 = gui_alt_color(color_rgb(255, 192, 255), color_rgb(128, 48, 112));
    color_t c3 = gui_alt_color(color_rgb(255, 255, 192), color_rgb(112, 112, 48));
    label_text(label1, i_LABEL_01);
    label_text(label2, i_LABEL_02);
    label_text(label3, i_LABEL_03);
    label_text(label4, i_LABEL_04);
    label_text(label5, i_LABEL_05);
    label_text(label6, i_LABEL_06);
    label_bgcolor(label1, c1);
    label_bgcolor(label2, c2);
    label_bgcolor(label3, c3);
    label_bgcolor(label4, c1);
    label_bgcolor(label5, c2);
    label_bgcolor(label6, c3);
    label_align(label4, ekLEFT);
    label_align(label5, ekCENTER);
    label_align(label6, ekRIGHT);
    layout_popup(layout, popup, 0, 0);
    layout_label(layout, label1, 0, 1);
    layout_label(layout, label2, 0, 2);
    layout_label(layout, label3, 0, 3);
    layout_label(layout, label4, 0, 4);
    layout_label(layout, label5, 0, 5);
    layout_label(layout, label6, 0, 6);
    layout_vmargin(layout, 0, 5);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

Panel *labels_mouse_over(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 5);
    Font *font = font_system(20, ekFNORMAL | ekFPIXELS);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    label_text(label1, i_LABEL_07);
    label_text(label2, i_LABEL_07);
    label_text(label3, i_LABEL_07);
    label_text(label4, i_LABEL_07);
    label_text(label5, i_LABEL_07);
    label_font(label1, font);
    label_font(label2, font);
    label_font(label3, font);
    label_font(label4, font);
    label_font(label5, font);
    label_color_over(label1, kCOLOR_RED);
    label_color_over(label2, kCOLOR_RED);
    label_color_over(label3, kCOLOR_RED);
    label_color_over(label4, kCOLOR_RED);
    label_color_over(label5, kCOLOR_RED);
    label_style_over(label1, ekFBOLD);
    label_style_over(label2, ekFITALIC);
    label_style_over(label3, ekFSTRIKEOUT);
    label_style_over(label4, ekFUNDERLINE);
    label_bgcolor_over(label5, kCOLOR_CYAN);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_label(layout, label3, 0, 2);
    layout_label(layout, label4, 0, 3);
    layout_label(layout, label5, 0, 4);
    panel_layout(panel, layout);
    font_destroy(&font);
    return panel;
}

