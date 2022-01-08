/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: textviews.c
 *
 */

/* Use of textviews */

#include "textviews.h"
#include "guiall.h"
#include "res.h"

/*---------------------------------------------------------------------------*/

static void i_set_rtf(TextView *text)
{
    ResPack *pack = res_respack("");
    uint32_t size = 0;
    const byte_t *data = respack_file(pack, TEXTVIEW_RTF, &size);
    Stream *stm = stm_from_block(data, size);
    textview_rtf(text, stm);
    stm_close(&stm);
    respack_destroy(&pack);
}

/*---------------------------------------------------------------------------*/

static void i_set_hard_coding(TextView *text)
{
    textview_units(text, ekFPOINTS);
    textview_lspacing(text, 1.15f);
    textview_afspace(text, 10);
    textview_family(text, "Arial");
    textview_fsize(text, 16);
    textview_writef(text, "What is Lorem Ipsum?\n");
    textview_fsize(text, 11);
    textview_writef(text, "Lorem Ipsum ");
    textview_fstyle(text, ekFBOLD);
    textview_writef(text, "is simply");
    textview_fstyle(text, ekFNORMAL);
    textview_writef(text, " dummy text of the ");
    textview_fstyle(text, ekFITALIC);
    textview_writef(text, "printing and typesetting ");
    textview_fstyle(text, ekFNORMAL);
    textview_writef(text, "industry. ");
    textview_fsize(text, 16);
    textview_color(text, color_rgb(255, 0, 0));
    textview_writef(text, "Lorem Ipsum ");
    textview_color(text, kCOLOR_DEFAULT);
    textview_fsize(text, 11);
    textview_writef(text, "has been the ");
    textview_family(text, "Courier New");
    textview_fsize(text, 14);
    textview_writef(text, "[industry's standard] ");
    textview_family(text, "Arial");
    textview_fsize(text, 11);
    textview_fstyle(text, ekFUNDERLINE);
    textview_writef(text, "dummy text");
    textview_fstyle(text, ekFNORMAL);
    textview_writef(text, " ever ");
    textview_fstyle(text, ekFSTRIKEOUT);
    textview_writef(text, "since the 1500s");
    textview_fstyle(text, ekFNORMAL);
    textview_writef(text, ", when an ");
    textview_color(text, color_rgb(0, 176, 80));
    textview_writef(text, "unknown printer ");
    textview_color(text, kCOLOR_DEFAULT);
    textview_writef(text, "took a galley of type and scrambled it to make a type specimen book");
    textview_fstyle(text, ekFITALIC);
    textview_color(text, color_rgb(0, 77, 187));
    textview_bgcolor(text, color_rgb(192, 192, 192));
    textview_writef(text, ". It has survived not only five centuries");
    textview_fstyle(text, ekFNORMAL);
    textview_color(text, kCOLOR_DEFAULT);
    textview_bgcolor(text, kCOLOR_DEFAULT);
    textview_writef(text, ", but also the leap into electronic typesetting, remaining essentially unchanged.");
}

/*---------------------------------------------------------------------------*/

Panel *textviews(void)
{
    Layout *layout = layout_create(1, 4);
    Label *label1 = label_create();
    Label *label2 = label_create();
    TextView *text1 = textview_create();
    TextView *text2 = textview_create();
    Panel *panel = panel_create();
    label_text(label1, "From RTF data");
    label_text(label2, "Hard coding");
    textview_size(text1, s2df(450, 250));
    textview_size(text2, s2df(450, 250));
    i_set_rtf(text1);
    i_set_hard_coding(text2);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 2);
    layout_textview(layout, text1, 0, 1);
    layout_textview(layout, text2, 0, 3);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 10);
    layout_vmargin(layout, 2, 5);
    panel_layout(panel, layout);
    return panel;
}
