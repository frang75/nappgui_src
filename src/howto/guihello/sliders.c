/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: sliders.c
 *
 */

/* Sliders */

#include "sliders.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

static void i_OnSlider(Progress *prog, Event *event)
{
    const EvSlider *params = event_params(event, EvSlider);
    progress_value(prog, params->pos);
}

/*---------------------------------------------------------------------------*/

Panel *sliders(void)
{
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = layout_create(1, 8);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Slider *slider1 = slider_create();
    Slider *slider2 = slider_create();
    Slider *slider3 = slider_vertical();
    Progress *prog1 = progress_create();
    Progress *prog2 = progress_create();
    Panel *panel = panel_create();
    label_text(label1, "Slider");
    label_text(label2, "Slider (discrete 6 steps)");
    label_text(label3, "Progress Bar");
    label_text(label4, "Progress Undefined");
    slider_steps(slider2, 6);
    slider_tooltip(slider1, "Horizontal Slider");
    slider_tooltip(slider2, "Horizontal Discrete Slider");
    slider_tooltip(slider3, "Vertical Slider");
    slider_OnMoved(slider1, listener(prog1, i_OnSlider, Progress));
    progress_undefined(prog2, TRUE);
    layout_label(layout2, label1, 0, 0);
    layout_label(layout2, label2, 0, 2);
    layout_label(layout2, label3, 0, 4);
    layout_label(layout2, label4, 0, 6);
    layout_slider(layout2, slider1, 0, 1);
    layout_slider(layout2, slider2, 0, 3);
    layout_slider(layout1, slider3, 1, 0);
    layout_progress(layout2, prog1, 0, 5);
    layout_progress(layout2, prog2, 0, 7);
    layout_hsize(layout2, 0, 300);
    layout_layout(layout1, layout2, 0, 0);
    layout_vmargin(layout2, 0, 5);
    layout_vmargin(layout2, 1, 5);
    layout_vmargin(layout2, 2, 5);
    layout_vmargin(layout2, 3, 5);
    layout_vmargin(layout2, 4, 5);
    layout_vmargin(layout2, 5, 5);
    layout_vmargin(layout2, 6, 5);
    layout_hmargin(layout1, 0, 10);
    panel_layout(panel, layout1);
    return panel;
}
