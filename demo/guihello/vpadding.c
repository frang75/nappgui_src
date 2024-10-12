/* Internal vertical padding */

#include "vpadding.h"
#include "res_guihello.h"
#include <gui/guiall.h>

/*---------------------------------------------------------------------------*/

Panel *vpadding(void)
{
    Font *font = font_system(20, 0);
    Layout *layout = layout_create(1, 6);
    Edit *edit1 = edit_create();
    Edit *edit2 = edit_create();
    Edit *edit3 = edit_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Panel *panel = panel_create();
    color_t bg = gui_alt_color(color_rgb(192, 255, 255), color_rgb(48, 112, 112));
    edit_text(edit1, "Edit with default padding");
    edit_text(edit2, "Edit with zero padding");
    edit_text(edit3, "Edit with high padding");
    edit_bgcolor(edit1, bg);
    edit_bgcolor(edit2, bg);
    edit_bgcolor(edit3, bg);
    button_text(button1, "Button with default padding");
    button_text(button2, "Button with zero padding");
    button_text(button3, "Button with high padding");
    button_font(button2, font);
    button_image(button1, gui_image(FOLDER24_PNG));
    button_image(button2, gui_image(FOLDER24_PNG));
    button_image(button3, gui_image(FOLDER24_PNG));
    edit_vpadding(edit2, 0);
    edit_vpadding(edit3, 20);
    button_vpadding(button2, 0);
    button_vpadding(button3, 20);
    layout_edit(layout, edit1, 0, 0);
    layout_edit(layout, edit2, 0, 1);
    layout_edit(layout, edit3, 0, 2);
    layout_button(layout, button1, 0, 3);
    layout_button(layout, button2, 0, 4);
    layout_button(layout, button3, 0, 5);
    layout_hsize(layout, 0, 250);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 5);
    layout_vmargin(layout, 4, 5);
    panel_layout(panel, layout);
    font_destroy(&font);
    return panel;
}
