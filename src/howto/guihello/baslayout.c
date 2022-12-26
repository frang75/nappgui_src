/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: baslayout.c
 *
 */

/* Basic Layouts */

#include "baslayout.h"
#include "guiall.h"

/*---------------------------------------------------------------------------*/

Panel *basic_layout(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(2, 5);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Label *label3 = label_create();
    Label *label4 = label_create();
    Label *label5 = label_create();
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
    edit_text(edit1, "Amanda Callister");
    edit_text(edit2, "aQwe56nhjJk");
    edit_text(edit3, "35, Tuam Road");
    edit_text(edit4, "Galway - Ireland");
    edit_text(edit5, "+35 654 333 000");
    edit_passmode(edit2, TRUE);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_label(layout, label3, 0, 2);
    layout_label(layout, label4, 0, 3);
    layout_label(layout, label5, 0, 4);
    layout_edit(layout, edit1, 1, 0);
    layout_edit(layout, edit2, 1, 1);
    layout_edit(layout, edit3, 1, 2);
    layout_edit(layout, edit4, 1, 3);
    layout_edit(layout, edit5, 1, 4);
    panel_layout(panel, layout);
    return panel;
}
