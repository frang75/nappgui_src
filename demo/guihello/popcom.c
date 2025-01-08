/* PopUp and Combo */

#include "popcom.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct _popupdata_t PopUpData;

struct _popupdata_t
{
    PopUp *popup;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(PopUpData **data)
{
    heap_delete(data, PopUpData);
}

/*---------------------------------------------------------------------------*/

static void i_popups(Layout *layout, PopUpData *data)
{
    Label *label1 = label_create();
    Label *label2 = label_create();
    PopUp *popup1 = popup_create();
    PopUp *popup2 = popup_create();
    label_text(label1, "Language:");
    label_text(label2, "Color:");
    popup_add_elem(popup1, "English", gui_image(UKING_PNG));
    popup_add_elem(popup1, "Español", gui_image(SPAIN_PNG));
    popup_add_elem(popup1, "Portugues", gui_image(PORTUGAL_PNG));
    popup_add_elem(popup1, "Italiana", gui_image(ITALY_PNG));
    popup_add_elem(popup1, "Tiếng Việt", gui_image(VIETNAM_PNG));
    popup_add_elem(popup1, "России", gui_image(RUSSIA_PNG));
    popup_add_elem(popup1, "日本語", gui_image(JAPAN_PNG));
    popup_add_elem(popup2, "Red", gui_image(RED_PNG));
    popup_add_elem(popup2, "Blue", gui_image(BLUE_PNG));
    popup_add_elem(popup2, "Green", gui_image(GREEN_PNG));
    popup_add_elem(popup2, "Yellow", gui_image(YELLOW_PNG));
    popup_add_elem(popup2, "Black", gui_image(BLACK_PNG));
    popup_add_elem(popup2, "White", gui_image(WHITE_PNG));
    popup_list_height(popup1, 10);
    popup_list_height(popup2, 10);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_popup(layout, popup1, 1, 0);
    layout_popup(layout, popup2, 1, 1);
    data->popup = popup1;
}

/*---------------------------------------------------------------------------*/

static void i_combos(Layout *layout)
{
    Label *label1 = label_create();
    Label *label2 = label_create();
    Combo *combo1 = combo_create();
    Combo *combo2 = combo_create();
    label_text(label1, "Search:");
    label_text(label2, "Folder:");
    combo_add_elem(combo1, "Search", NULL);
    combo_add_elem(combo1, "Disk", NULL);
    combo_add_elem(combo1, "Edit", NULL);
    combo_add_elem(combo2, "/home/fran/Desktop", NULL);
    combo_add_elem(combo2, "/usr/include", NULL);
    combo_add_elem(combo2, "/mnt/volume1", NULL);
    combo_add_elem(combo2, "/etc/html/log.txt", NULL);
    layout_label(layout, label1, 2, 0);
    layout_label(layout, label2, 2, 1);
    layout_combo(layout, combo1, 3, 0);
    layout_combo(layout, combo2, 3, 1);
}

/*---------------------------------------------------------------------------*/

static void i_OnAdd(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    popup_add_elem(data->popup, "Español", gui_image(SPAIN_PNG));
}

/*---------------------------------------------------------------------------*/

static void i_OnClear(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    popup_clear(data->popup);
}

/*---------------------------------------------------------------------------*/

Panel *popup_combo(void)
{
    PopUpData *data = heap_new0(PopUpData);
    Panel *panel = panel_create();
    Layout *layout = layout_create(4, 4);
    Button *button1 = button_push();
    Button *button2 = button_push();
    i_popups(layout, data);
    i_combos(layout);
    button_text(button1, "Add elem to PopUp");
    button_text(button2, "Clear PopUp");
    button_OnClick(button1, listener(data, i_OnAdd, PopUpData));
    button_OnClick(button2, listener(data, i_OnClear, PopUpData));
    layout_button(layout, button1, 1, 2);
    layout_button(layout, button2, 1, 3);
    layout_margin(layout, 10);
    layout_vmargin(layout, 0, 10);
    layout_vmargin(layout, 1, 10);
    layout_vmargin(layout, 2, 5);
    layout_hmargin(layout, 0, 5);
    layout_hmargin(layout, 1, 10);
    layout_hmargin(layout, 2, 5);
    layout_hsize(layout, 1, 150);
    layout_hsize(layout, 3, 150);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_data, PopUpData);
    return panel;
}
