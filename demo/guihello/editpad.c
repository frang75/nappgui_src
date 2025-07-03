/* Editbox padding */

#include "editpad.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

Panel *editpad(void)
{
    Layout *layout = layout_create(1, 3);
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    Panel *panel = panel_create();
    color_t bg = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    edit_text(edit1, "Edit with default padding");
    edit_text(edit2, "Edit with zero padding");
    edit_text(edit3, "Edit with high padding");
    edit_bgcolor(edit1, bg);
    edit_bgcolor(edit2, bg);
    edit_bgcolor(edit3, bg);
    edit_vpadding(edit2, 0);
    edit_vpadding(edit3, 30);
    layout_edit(layout, edit1, 0, 0);
    layout_edit(layout, edit2, 0, 1);
    layout_edit(layout, edit3, 0, 2);
    layout_hsize(layout, 0, 250);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    panel_layout(panel, layout);
    return panel;
}
