/* Font units */

#include "fontx.h"
#include <gui/guiall.h>

typedef struct _fontunits_t FontUnits;

struct _fontunits_t
{
    Font *font_px;
    Font *font_pt;
    Font *font_cell;
};

static const char_t *i_TEXT = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.";
static const real32_t i_WIDTH = 300;

/*---------------------------------------------------------------------------*/

static void i_destroy_font_units(FontUnits **funits)
{
    cassert_no_null(funits);
    cassert_no_null(*funits);
    font_destroy(&(*funits)->font_px);
    font_destroy(&(*funits)->font_pt);
    font_destroy(&(*funits)->font_cell);
    heap_delete(funits, FontUnits);
}

/*---------------------------------------------------------------------------*/

static void i_draw(const Font *font, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    draw_font(p->ctx, font);
    draw_text_width(p->ctx, i_WIDTH);
    draw_text(p->ctx, i_TEXT, 0, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrawPx(FontUnits *funits, Event *e)
{
    cassert_no_null(funits);
    i_draw(funits->font_px, e);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrawPt(FontUnits *funits, Event *e)
{
    cassert_no_null(funits);
    i_draw(funits->font_pt, e);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrawCell(FontUnits *funits, Event *e)
{
    cassert_no_null(funits);
    i_draw(funits->font_cell, e);
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(FontUnits *funits)
{
    color_t c1 = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    Layout *layout = layout_create(3, 4);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
    Label *label6 = label_create();
    TextView *text1 = textview_create();
    TextView *text2 = textview_create();
    TextView *text3 = textview_create();
    View *view1 = view_create();
    View *view2 = view_create();
    View *view3 = view_create();
    label_text(label1, "Font 20 px");
    label_text(label2, "Font 20 pt");
    label_text(label3, "Font 20 height");
    label_multiline(label4, TRUE);
    label_multiline(label5, TRUE);
    label_multiline(label6, TRUE);
    label_text(label4, i_TEXT);
    label_text(label5, i_TEXT);
    label_text(label6, i_TEXT);
    label_bgcolor(label6, c1);
    label_font(label4, funits->font_px);
    label_font(label5, funits->font_pt);
    label_font(label6, funits->font_cell);
    label_min_width(label4, i_WIDTH);
    label_min_width(label5, i_WIDTH);
    label_min_width(label6, i_WIDTH);
    textview_family(text1, font_family(funits->font_px));
    textview_family(text2, font_family(funits->font_pt));
    textview_family(text3, font_family(funits->font_cell));
    textview_fsize(text1, 20);
    textview_fsize(text2, 20);
    textview_fsize(text3, 20);
    textview_units(text1, ekFPIXELS);
    textview_units(text2, ekFPOINTS);
    textview_units(text3, ekFCELL);
    textview_writef(text1, i_TEXT);
    textview_writef(text2, i_TEXT);
    textview_writef(text3, i_TEXT);
    textview_size(text1, s2df(i_WIDTH, 120));
    textview_size(text2, s2df(i_WIDTH, 120));
    textview_size(text3, s2df(i_WIDTH, 120));
    view_OnDraw(view1, listener(funits, i_OnDrawPx, FontUnits));
    view_OnDraw(view2, listener(funits, i_OnDrawPt, FontUnits));
    view_OnDraw(view3, listener(funits, i_OnDrawCell, FontUnits));
    view_size(view1, s2df(i_WIDTH, 200));
    view_size(view2, s2df(i_WIDTH, 200));
    view_size(view3, s2df(i_WIDTH, 200));
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 1, 0);
    layout_label(layout, label3, 2, 0);
    layout_label(layout, label4, 0, 1);
    layout_label(layout, label5, 1, 1);
    layout_label(layout, label6, 2, 1);
    layout_textview(layout, text1, 0, 2);
    layout_textview(layout, text2, 1, 2);
    layout_textview(layout, text3, 2, 2);
    layout_view(layout, view1, 0, 3);
    layout_view(layout, view2, 1, 3);
    layout_view(layout, view3, 2, 3);
    layout_valign(layout, 0, 1, ekTOP);
    layout_valign(layout, 1, 1, ekTOP);
    layout_valign(layout, 2, 1, ekTOP);
    return layout;
}

/*---------------------------------------------------------------------------*/

static FontUnits *i_font_units(void)
{
    FontUnits *funits = heap_new(FontUnits);
    funits->font_px = font_system(20, ekFPIXELS);
    funits->font_pt = font_system(20, ekFPOINTS);
    funits->font_cell = font_system(20, ekFCELL);
    return funits;
}

/*---------------------------------------------------------------------------*/

Panel *font_units_demo(void)
{
    FontUnits *funits = i_font_units();
    Layout *layout = i_layout(funits);
    Panel *panel = panel_create();
    panel_data(panel, &funits, i_destroy_font_units, FontUnits);
    panel_layout(panel, layout);
    return panel;
}
