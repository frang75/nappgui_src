/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: scrollpanel.c
 *
 */

/* Panel with scroll */
    
#include "scrollpanel.h"
#include "guiall.h"

static const uint32_t i_ROWS = 100;

/*---------------------------------------------------------------------------*/

Panel *scrollpanel(void)
{
    Panel *panel = panel_scroll(FALSE, TRUE);
    Layout *layout = layout_create(3, i_ROWS);
    real32_t margin = panel_scroll_width(panel);
    uint32_t i = 0;
    panel_size(panel, s2df(-1, 400));
    for (i = 0; i < i_ROWS; ++i)
    {
        char_t text[128];
        Label *label = label_create();
        Edit *edit = edit_create();
        Button *button = button_push();
        bstd_sprintf(text, sizeof(text), "User %d", i + 1);
        label_text(label, text);
        bstd_sprintf(text, sizeof(text), "Name of User %d", i + 1);
        edit_text(edit, text);
        bstd_sprintf(text, sizeof(text), "Edit %d", i + 1);
        button_text(button, text);
        layout_label(layout, label, 0, i);
        layout_edit(layout, edit, 1, i);
        layout_button(layout, button, 2, i);
    }

    for (i = 0; i < i_ROWS - 1; ++i)
        layout_vmargin(layout, i, 5);

    layout_hmargin(layout, 0, 10);
    layout_hmargin(layout, 1, 10);
    layout_hsize(layout, 1, 150);
    layout_margin4(layout, 0, margin, 0, 0);
    panel_layout(panel, layout);
    return panel;
}
