/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: splits.c
 *
 */

/* Use of splitviews */

#include "splits.h"
#include "guiall.h"

static const char_t *i_LOREM = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";

/*---------------------------------------------------------------------------*/

static void i_OnDraw(View *view, Event *e)
{
    const EvDraw *p = event_params(e, EvDraw);
    real32_t p0 = p->width / 6;
    real32_t p1 = p->height / 6;
    real32_t p2 = p->width / 3;
    real32_t p3 = p->height / 3;
    unref(view);
    draw_fill_color(p->ctx, kCOLOR_RED);
    draw_rect(p->ctx, ekFILL, 0, 0, p->width, p->height);
    draw_fill_color(p->ctx, kCOLOR_GREEN);
    draw_rect(p->ctx, ekFILL, p0, p1, p->width - 2 * p0, p->height - 2 * p1);
    draw_fill_color(p->ctx, kCOLOR_BLUE);
    draw_rect(p->ctx, ekFILL, p2, p3, p->width - 2 * p2, p->height - 2 * p3);
}

/*---------------------------------------------------------------------------*/

static Panel *i_left_panel(void)
{
    uint32_t i, n = 32;
    Panel *panel = panel_scroll(FALSE, TRUE);
    Layout *layout = layout_create(2, n);
    real32_t rmargin = panel_scroll_width(panel);

    for (i = 0; i < n; ++i)
    {
        char_t text[64];
        Label *label = label_create();
        Edit *edit = edit_create();
        bstd_sprintf(text, sizeof(text), "Value %02d", i);
        label_text(label, text);
        bstd_sprintf(text, sizeof(text), "Edit here value %02d", i);
        edit_text(edit, text);
        layout_label(layout, label, 0, i);
        layout_edit(layout, edit, 1, i);
    }

    for (i = 0; i < n - 1; ++i)
        layout_vmargin(layout, i, 3);

    layout_hmargin(layout, 0, 5);
    layout_margin4(layout, 0, rmargin + 5, 0, 0);
    layout_hexpand(layout, 1);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

Panel *split_panel(void)
{
    Panel *panel1 = panel_create();
    Panel *panel2 = i_left_panel();
    Layout *layout = layout_create(1, 1);
    SplitView *split1 = splitview_vertical();
    SplitView *split2 = splitview_horizontal();
    TextView *text = textview_create();
    View *view = view_create();
    textview_writef(text, i_LOREM);
    view_OnDraw(view, listener(view, i_OnDraw, View));
    splitview_pos(split1, .25f);
    splitview_size(split1, s2df(800, 480));
    splitview_size(split2, s2df(640, 480));
    splitview_view(split2, view);
    splitview_text(split2, text);
    splitview_panel(split1, panel2);
    splitview_split(split1, split2);
    layout_splitview(layout, split1, 0, 0);
    panel_layout(panel1, layout);
    return panel1;
}
