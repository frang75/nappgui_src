/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: form.c
 *
 */

/* Form demo */

#include "form.h"
#include "res.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

static void i_OnFilter(void *noused, Event *e)
{
    const EvText *params = event_params(e, EvText);
    EvTextFilter *result = event_result(e, EvTextFilter);
    uint32_t i = 0, j = 0;
    while (params->text[i] != '\0')
    {
        if (params->text[i] >= '0' && params->text[i] <= '9')
        {
            result->text[j] = params->text[i];
            j += 1;
        }

        i += 1;
    }

    result->text[j] = '\0';
    result->apply = TRUE;
    unref(noused);
}

/*---------------------------------------------------------------------------*/

static void i_OnUpDown(Edit *edit, Event *e)
{
    const EvButton *params = event_params(e, EvButton);
    int32_t n = str_to_i32(edit_get_text(edit), 10, NULL);
    char_t text[64];
    n += (params->index == 0) ? 1 : -1;
    bstd_sprintf(text, sizeof(text), "%d", n);
    edit_text(edit, text);
}

/*---------------------------------------------------------------------------*/

static Layout *i_numbers(color_t colorbg)
{
    Layout *layout = layout_create(5, 1);
    Label *label = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    UpDown *updown1 = updown_create();
    UpDown *updown2 = updown_create();
    label_text(label, "Height (cm):");
    edit_text(edit1, "25");
    edit_text(edit2, "175");
    edit_align(edit1, ekRIGHT);
    edit_align(edit2, ekRIGHT);
    edit_OnFilter(edit1, listener(NULL, i_OnFilter, void));
    edit_OnFilter(edit2, listener(NULL, i_OnFilter, void));
    edit_bgcolor_focus(edit1, colorbg);
    edit_bgcolor_focus(edit2, colorbg);
    updown_OnClick(updown1, listener(edit1, i_OnUpDown, Edit));
    updown_OnClick(updown2, listener(edit2, i_OnUpDown, Edit));
    updown_tooltip(updown1, "Increase/Decrease age");
    updown_tooltip(updown2, "Increase/Decrease height");
    layout_label(layout, label, 2, 0);
    layout_edit(layout, edit1, 0, 0);
    layout_edit(layout, edit2, 3, 0);
    layout_updown(layout, updown1, 1, 0);
    layout_updown(layout, updown2, 4, 0);
    layout_hmargin(layout, 1, 10.f);
    layout_hmargin(layout, 2, 10.f);
    layout_hexpand2(layout, 0, 3, .5f);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_edits(void)
{
    color_t colorbg = gui_alt_color(color_bgr(0xFFFFe4), color_bgr(0x101010));
    Layout *layout1 = layout_create(2, 6);
    Layout *layout2 = i_numbers(colorbg);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
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
    label_text(label6, "Age:");
    label_color_over(label1, color_rgb(255, 128, 52));
    label_color_over(label2, color_rgb(70, 129, 207));
    label_color_over(label3, color_rgb(119, 188, 31));
    label_style_over(label4, ekFITALIC | ekFUNDERLINE);
    edit_text(edit1, "Amanda Callister");
    edit_text(edit2, "aQwe56nhjJk");
    edit_text(edit3, "35, Tuam Road");
    edit_text(edit4, "Galway - Ireland");
    edit_text(edit5, "+35 654 333 000");
    edit_passmode(edit2, TRUE);
    edit_bgcolor_focus(edit1, colorbg);
    edit_bgcolor_focus(edit2, colorbg);
    edit_bgcolor_focus(edit3, colorbg);
    edit_bgcolor_focus(edit4, colorbg);
    edit_bgcolor_focus(edit5, colorbg);
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 0, 1);
    layout_label(layout1, label3, 0, 2);
    layout_label(layout1, label4, 0, 3);
    layout_label(layout1, label5, 0, 4);
    layout_label(layout1, label6, 0, 5);
    layout_edit(layout1, edit1, 1, 0);
    layout_edit(layout1, edit2, 1, 1);
    layout_edit(layout1, edit3, 1, 2);
    layout_edit(layout1, edit4, 1, 3);
    layout_edit(layout1, edit5, 1, 4);
    layout_layout(layout1, layout2, 1, 5);
    layout_hmargin(layout1, 0, 5);
    layout_hexpand(layout1, 1);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    layout_vmargin(layout1, 4, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_form(void)
{
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = i_edits();
    Label *label = label_multiline();
    label_text(label, "Please fill in all the information on the form. We will use this data to send commercial mail at all hours, not caring much if it bothers you or not.");
    label_color(label, gui_alt_color(color_rgb(255, 0, 0), color_rgb(180, 180, 180)));
    label_bgcolor(label, gui_alt_color(color_rgb(216, 191, 216), color_rgb(80, 40, 40)));
    label_bgcolor_over(label, gui_alt_color(color_rgb(255, 250, 205), color_rgb(105, 100, 55)));
    label_style_over(label, ekFUNDERLINE);
    layout_layout(layout1, layout2, 0, 0);
    layout_label(layout1, label, 0, 1);
    layout_hsize(layout1, 0, 300);
    layout_vmargin(layout1, 0, 10);
    layout_margin(layout1, 10);
    return layout1;
}

/*---------------------------------------------------------------------------*/

Panel *form_basic(void)
{
    Layout *layout = i_form();
    Panel *panel = panel_create();
    panel_layout(panel, layout);
    return panel;
}

