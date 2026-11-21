/* DateView */

#include "dateviews.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

static void i_OnSelect(Label *label, Event *e)
{
    const EvDate *p = event_params(e, EvDate);
    String *text = date_format(&p->date, "%Y-%m-%d");
    label_text(label, tc(text));
    str_destroy(&text);
}

/*---------------------------------------------------------------------------*/

Panel *dateviews(void)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(3, 3);
    Layout *layout2 = layout_create(1, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    DateView *dateview1 = dateview_create();
    DateView *dateview2 = dateview_create();
    DateView *dateview3 = dateview_create();
    label_text(label1, "Natural size");
    label_text(label2, "Expanded");
    label_text(label3, "Reduced");
    label_text(label4, "No selection");
    dateview_OnSelect(dateview1, listener(label4, i_OnSelect, Label));
    dateview_OnSelect(dateview2, listener(label4, i_OnSelect, Label));
    dateview_OnSelect(dateview3, listener(label4, i_OnSelect, Label));
    layout_label(layout1, label1, 0, 0);
    layout_label(layout1, label2, 1, 0);
    layout_label(layout1, label3, 2, 0);
    layout_dateview(layout1, dateview1, 0, 1);
    layout_dateview(layout1, dateview2, 1, 1);
    layout_layout(layout1, layout2, 2, 1);
    layout_label(layout1, label4, 0, 2);
    layout_dateview(layout2, dateview3, 0, 0);
    layout_hsize(layout1, 1, 300);
    layout_vsize(layout1, 1, 400);
    layout_hsize(layout2, 0, 150);
    layout_vsize(layout2, 0, 120);
    layout_valign(layout1, 0, 1, ekTOP);
    layout_valign(layout1, 2, 1, ekTOP);
    layout_hmargin(layout1, 0, 10);
    layout_hmargin(layout1, 1, 10);
    layout_vmargin(layout1, 0, 5);
    layout_vmargin(layout1, 1, 10);
    panel_layout(panel, layout1);
    return panel;
}
