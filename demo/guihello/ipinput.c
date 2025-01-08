/* IP input */

#include "ipinput.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

static void i_OnEditFilter(Window *window, Event *e)
{
    const EvText *p = event_params(e, EvText);
    EvTextFilter *filter = event_result(e, EvTextFilter);
    uint32_t i, j = 0, n = str_len_c(p->text);

    /* We only accept numbers in IP controls */
    for (i = 0; i < n; ++i)
    {
        if (p->text[i] >= '0' && p->text[i] <= '9')
            filter->text[j++] = p->text[i];
    }

    if (j > 3)
        j = 3;

    filter->text[j] = '\0';
    filter->apply = TRUE;

    /* We wrote the third character --> Jump to next control */
    if (j == 3)
        window_next_tabstop(window);
}

/*---------------------------------------------------------------------------*/

Panel *ip_input(Window *window)
{
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(7, 1);
    Layout *layout2 = layout_create(1, 3);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    Edit *edit4 = edit_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    label_text(label1, ".");
    label_text(label2, ".");
    label_text(label3, ".");
    button_text(button1, "Connect");
    button_text(button2, "Exit");
    edit_OnFilter(edit1, listener(window, i_OnEditFilter, Window));
    edit_OnFilter(edit2, listener(window, i_OnEditFilter, Window));
    edit_OnFilter(edit3, listener(window, i_OnEditFilter, Window));
    edit_OnFilter(edit4, listener(window, i_OnEditFilter, Window));
    layout_label(layout1, label1, 1, 0);
    layout_label(layout1, label2, 3, 0);
    layout_label(layout1, label3, 5, 0);
    layout_edit(layout1, edit1, 0, 0);
    layout_edit(layout1, edit2, 2, 0);
    layout_edit(layout1, edit3, 4, 0);
    layout_edit(layout1, edit4, 6, 0);
    layout_layout(layout2, layout1, 0, 0);
    layout_button(layout2, button1, 0, 1);
    layout_button(layout2, button2, 0, 2);
    layout_vmargin(layout2, 0, 5.f);
    layout_vmargin(layout2, 1, 5.f);
    layout_hsize(layout2, 0, 200.f);
    panel_layout(panel, layout2);
    return panel;
}
