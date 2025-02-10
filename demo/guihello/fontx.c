/* Font x-scale */

#include "fontx.h"
#include <gui/guiall.h>

typedef struct _fontx_t FontX;

struct _fontx_t
{
    Font *font_1_0;
    Font *font_1_5;
    Font *font_0_5;
    Font *fontm_1_0;
    Font *fontm_1_5;
    Font *fontm_0_5;
    Label *mline;
    Layout *layout;
    color_t c1;
    color_t c2;
};

const char_t *i_TEXT1 = "Font system regular";
const char_t *i_TEXT2 = "Font monospace regular";
const char_t *i_TEXT3 = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";

/*---------------------------------------------------------------------------*/

static void i_destroy_fontx(FontX **fontx)
{
    cassert_no_null(fontx);
    cassert_no_null(*fontx);
    font_destroy(&(*fontx)->font_1_0);
    font_destroy(&(*fontx)->font_1_5);
    font_destroy(&(*fontx)->font_0_5);
    font_destroy(&(*fontx)->fontm_1_0);
    font_destroy(&(*fontx)->fontm_1_5);
    font_destroy(&(*fontx)->fontm_0_5);
    heap_delete(fontx, FontX);
}

/*---------------------------------------------------------------------------*/

static void i_OnMoved(FontX *fontx, Event *e)
{
    const EvSlider *p = event_params(e, EvSlider);
    Font *font = font_system(font_regular_size(), 0);
    Font *font_sx = font_with_xscale(font, p->pos + .5f);
    label_font(fontx->mline, font_sx);
    layout_update(fontx->layout);
    font_destroy(&font_sx);
    font_destroy(&font);
}

/*---------------------------------------------------------------------------*/

static Layout *i_label_layout(FontX *fontx)
{
    Layout *layout1 = layout_create(2, 1);
    Layout *layout2 = layout_create(1, 7);
    Slider *slider = slider_create();
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    Label *label7 = label_create();
    cassert_no_null(fontx);
    slider_value(slider, .5f);
    slider_OnMoved(slider, listener(fontx, i_OnMoved, FontX));
    label_multiline(label7, TRUE);
    label_text(label1, i_TEXT1);
    label_text(label2, i_TEXT1);
    label_text(label3, i_TEXT1);
    label_text(label4, i_TEXT2);
    label_text(label5, i_TEXT2);
    label_text(label6, i_TEXT2);
    label_text(label7, i_TEXT3);
    label_font(label1, fontx->font_1_0);
    label_font(label2, fontx->font_1_5);
    label_font(label3, fontx->font_0_5);
    label_font(label4, fontx->fontm_1_0);
    label_font(label5, fontx->fontm_1_5);
    label_font(label6, fontx->fontm_0_5);
    label_font(label7, fontx->font_1_0);
    label_bgcolor(label1, fontx->c1);
    label_bgcolor(label2, fontx->c1);
    label_bgcolor(label3, fontx->c1);
    label_bgcolor(label4, fontx->c1);
    label_bgcolor(label5, fontx->c1);
    label_bgcolor(label6, fontx->c1);
    label_bgcolor(label7, fontx->c1);
    layout_slider(layout2, slider, 0, 0);
    layout_label(layout2, label1, 0, 1);
    layout_label(layout2, label2, 0, 2);
    layout_label(layout2, label3, 0, 3);
    layout_label(layout2, label4, 0, 4);
    layout_label(layout2, label5, 0, 5);
    layout_label(layout2, label6, 0, 6);
    layout_vmargin(layout2, 0, 5);
    layout_valign(layout1, 0, 0, ekTOP);
    layout_valign(layout1, 1, 0, ekTOP);
    layout_hsize(layout1, 1, 300);
    layout_layout(layout1, layout2, 0, 0);
    layout_label(layout1, label7, 1, 0);
    fontx->mline = label7;
    fontx->layout = layout1;
    return layout1;
}

/*---------------------------------------------------------------------------*/

static Layout *i_button_layout(FontX *fontx)
{
    Layout *layout = layout_create(1, 6);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Button *button5 = button_push();
    Button *button6 = button_push();
    cassert_no_null(fontx);
    button_text(button1, i_TEXT1);
    button_text(button2, i_TEXT1);
    button_text(button3, i_TEXT1);
    button_text(button4, i_TEXT2);
    button_text(button5, i_TEXT2);
    button_text(button6, i_TEXT2);
    button_font(button1, fontx->font_1_0);
    button_font(button2, fontx->font_1_5);
    button_font(button3, fontx->font_0_5);
    button_font(button4, fontx->fontm_1_0);
    button_font(button5, fontx->fontm_1_5);
    button_font(button6, fontx->fontm_0_5);
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_button(layout, button5, 0, 4);
    layout_button(layout, button6, 0, 5);
    layout_halign(layout, 0, 0, ekLEFT);
    layout_halign(layout, 0, 1, ekLEFT);
    layout_halign(layout, 0, 2, ekLEFT);
    layout_halign(layout, 0, 3, ekLEFT);
    layout_halign(layout, 0, 4, ekLEFT);
    layout_halign(layout, 0, 5, ekLEFT);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Layout *i_edit_layout(FontX *fontx)
{
    Layout *layout = layout_create(1, 6);
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    Edit *edit4 = edit_create();
    Edit *edit5 = edit_create();
    Edit *edit6 = edit_create();
    cassert_no_null(fontx);
    edit_text(edit1, i_TEXT1);
    edit_text(edit2, i_TEXT1);
    edit_text(edit3, i_TEXT1);
    edit_text(edit4, i_TEXT2);
    edit_text(edit5, i_TEXT2);
    edit_text(edit6, i_TEXT2);
    edit_font(edit1, fontx->font_1_0);
    edit_font(edit2, fontx->font_1_5);
    edit_font(edit3, fontx->font_0_5);
    edit_font(edit4, fontx->fontm_1_0);
    edit_font(edit5, fontx->fontm_1_5);
    edit_font(edit6, fontx->fontm_0_5);
    layout_edit(layout, edit1, 0, 0);
    layout_edit(layout, edit2, 0, 1);
    layout_edit(layout, edit3, 0, 2);
    layout_edit(layout, edit4, 0, 3);
    layout_edit(layout, edit5, 0, 4);
    layout_edit(layout, edit6, 0, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static ListBox *i_listbox(const Font *font, const char_t *text)
{
    ListBox *listbox = listbox_create();
    listbox_font(listbox, font);
    listbox_add_elem(listbox, text, NULL);
    listbox_size(listbox, s2df(100, 50));
    return listbox;
}

/*---------------------------------------------------------------------------*/

static Layout *i_list_layout(FontX *fontx)
{
    Layout *layout = layout_create(6, 1);
    ListBox *list1 = i_listbox(fontx->font_1_0, i_TEXT1);
    ListBox *list2 = i_listbox(fontx->font_1_5, i_TEXT1);
    ListBox *list3 = i_listbox(fontx->font_0_5, i_TEXT1);
    ListBox *list4 = i_listbox(fontx->fontm_1_0, i_TEXT2);
    ListBox *list5 = i_listbox(fontx->fontm_1_5, i_TEXT2);
    ListBox *list6 = i_listbox(fontx->fontm_0_5, i_TEXT2);
    layout_listbox(layout, list1, 0, 0);
    layout_listbox(layout, list2, 1, 0);
    layout_listbox(layout, list3, 2, 0);
    layout_listbox(layout, list4, 3, 0);
    layout_listbox(layout, list5, 4, 0);
    layout_listbox(layout, list6, 5, 0);
    return layout;
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(FontX *fontx, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    real32_t w1, w2, w3, w4, w5, w6;
    real32_t h1, h2, h3, h4, h5, h6;
    cassert_no_null(fontx);
    draw_clear(p->ctx, fontx->c2);
    draw_text_color(p->ctx, gui_label_color());
    draw_font(p->ctx, fontx->font_1_0);
    draw_text_extents(p->ctx, i_TEXT1, -1, &w1, &h1);
    draw_text(p->ctx, i_TEXT1, 0, 0);
    draw_font(p->ctx, fontx->font_1_5);
    draw_text_extents(p->ctx, i_TEXT1, -1, &w2, &h2);
    draw_text(p->ctx, i_TEXT1, 0, h1);
    draw_font(p->ctx, fontx->font_0_5);
    draw_text_extents(p->ctx, i_TEXT1, -1, &w3, &h3);
    draw_text(p->ctx, i_TEXT1, 0, h1 + h2);
    draw_font(p->ctx, fontx->fontm_1_0);
    draw_text_extents(p->ctx, i_TEXT2, -1, &w4, &h4);
    draw_text(p->ctx, i_TEXT2, 0, h1 + h2 + h3);
    draw_font(p->ctx, fontx->fontm_1_5);
    draw_text_extents(p->ctx, i_TEXT2, -1, &w5, &h5);
    draw_text(p->ctx, i_TEXT2, 0, h1 + h2 + h3 + h4);
    draw_font(p->ctx, fontx->fontm_0_5);
    draw_text_extents(p->ctx, i_TEXT2, -1, &w6, &h6);
    draw_text(p->ctx, i_TEXT2, 0, h1 + h2 + h3 + h4 + h5);
    draw_line_color(p->ctx, kCOLOR_RED);
    draw_rect(p->ctx, ekSTROKE, 0, 0, w1, h1);
    draw_rect(p->ctx, ekSTROKE, 0, h1, w2, h2);
    draw_rect(p->ctx, ekSTROKE, 0, h1 + h2, w3, h3);
    draw_rect(p->ctx, ekSTROKE, 0, h1 + h2 + h3, w4, h4);
    draw_rect(p->ctx, ekSTROKE, 0, h1 + h2 + h3 + h4, w5, h5);
    draw_rect(p->ctx, ekSTROKE, 0, h1 + h2 + h3 + h4 + h5, w6, h6);
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(FontX *fontx)
{
    Layout *layout1 = layout_create(1, 5);
    Layout *layout2 = i_label_layout(fontx);
    Layout *layout3 = i_button_layout(fontx);
    Layout *layout4 = i_edit_layout(fontx);
    Layout *layout5 = i_list_layout(fontx);
    View *view = view_create();
    view_OnDraw(view, listener(fontx, i_OnDraw, FontX));
    view_size(view, s2df(200, 120));
    layout_layout(layout1, layout2, 0, 0);
    layout_view(layout1, view, 0, 1);
    layout_layout(layout1, layout3, 0, 2);
    layout_layout(layout1, layout4, 0, 3);
    layout_layout(layout1, layout5, 0, 4);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 5);
    layout_vmargin(layout1, 2, 5);
    layout_vmargin(layout1, 3, 5);
    return layout1;
}

/*---------------------------------------------------------------------------*/

static FontX *i_fontx(void)
{
    FontX *fontx = heap_new(FontX);
    fontx->font_1_0 = font_system(font_regular_size(), 0);
    fontx->font_1_5 = font_with_xscale(fontx->font_1_0, 1.5f);
    fontx->font_0_5 = font_with_xscale(fontx->font_1_0, 0.5f);
    fontx->fontm_1_0 = font_monospace(font_regular_size(), 0);
    fontx->fontm_1_5 = font_with_xscale(fontx->fontm_1_0, 1.5f);
    fontx->fontm_0_5 = font_with_xscale(fontx->fontm_1_0, 0.5f);
    fontx->c1 = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    fontx->c2 = gui_alt_color(color_rgb(255, 192, 255), color_rgb(128, 48, 112));
    return fontx;
}

/*---------------------------------------------------------------------------*/

Panel *font_x_scale(void)
{
    FontX *fontx = i_fontx();
    Layout *layout = i_layout(fontx);
    Panel *panel = panel_create();
    panel_data(panel, &fontx, i_destroy_fontx, FontX);
    panel_layout(panel, layout);
    return panel;
}
