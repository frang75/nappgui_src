/* PopUp and Combo */

#include "popcom.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct _popupdata_t PopUpData;

struct _popupdata_t
{
    PopUp *popup;
    Combo *combo;
    TextView *view;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(PopUpData **data)
{
    heap_delete(data, PopUpData);
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpSelect(PopUpData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    textview_printf(data->view, "PopUp OnSelect %d '%s'\n", p->index, p->text);
    textview_scroll_caret(data->view);
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
    popup_OnSelect(popup1, listener(data, i_OnPopUpSelect, PopUpData));
    popup_list_height(popup1, 10);
    popup_list_height(popup2, 10);
    layout_label(layout, label1, 0, 0);
    layout_label(layout, label2, 0, 1);
    layout_popup(layout, popup1, 1, 0);
    layout_popup(layout, popup2, 1, 1);
    data->popup = popup1;
}

/*---------------------------------------------------------------------------*/

static void i_OnComboSelect(PopUpData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    textview_printf(data->view, "Combo OnSelect %d '%s'\n", p->index, p->text);
    textview_scroll_caret(data->view);
}

/*---------------------------------------------------------------------------*/

static void i_combos(Layout *layout, PopUpData *data)
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
    combo_add_elem(combo2, "/home/fran/Desktop", gui_image(FOLDER16_PNG));
    combo_add_elem(combo2, "/usr/include", gui_image(EDIT16_PNG));
    combo_add_elem(combo2, "/mnt/volume1", gui_image(DISK16_PNG));
    combo_add_elem(combo2, "/etc/html/log.txt", gui_image(FIRST16_PNG));
    combo_OnSelect(combo1, listener(data, i_OnComboSelect, PopUpData));
    combo_list_height(combo1, 10);
    combo_list_height(combo2, 3);
    layout_label(layout, label1, 2, 0);
    layout_label(layout, label2, 2, 1);
    layout_combo(layout, combo1, 3, 0);
    layout_combo(layout, combo2, 3, 1);
    data->combo = combo1;
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpAdd(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    popup_add_elem(data->popup, "Español", gui_image(SPAIN_PNG));
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpInsert(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    popup_ins_elem(data->popup, 0, "English", gui_image(UKING_PNG));
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpDelete(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    if (popup_count(data->popup) > 0)
        popup_del_elem(data->popup, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpClear(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    popup_clear(data->popup);
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpGetSel(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    textview_printf(data->view, "PopUp selected: %d\n", popup_get_selected(data->popup));
    textview_scroll_caret(data->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnComboAdd(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    combo_add_elem(data->combo, "combo add item", NULL);
}

/*---------------------------------------------------------------------------*/

static void i_OnComboInsert(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    combo_ins_elem(data->combo, 0, "combo ins item", NULL);
}

/*---------------------------------------------------------------------------*/

static void i_OnComboDelete(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    if (combo_count(data->combo) > 0)
        combo_del_elem(data->combo, 0);
}

/*---------------------------------------------------------------------------*/

static void i_OnComboClear(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    combo_clear(data->combo);
}

/*---------------------------------------------------------------------------*/

static void i_OnComboGetSel(PopUpData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    textview_printf(data->view, "Combo selected: %d\n", combo_get_selected(data->combo));
    textview_scroll_caret(data->view);
}

/*---------------------------------------------------------------------------*/

Panel *popup_combo(void)
{
    PopUpData *data = heap_new0(PopUpData);
    Panel *panel = panel_create();
    Layout *layout1 = layout_create(1, 2);
    Layout *layout2 = layout_create(4, 7);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Button *button5 = button_push();
    Button *button6 = button_push();
    Button *button7 = button_push();
    Button *button8 = button_push();
    Button *button9 = button_push();
    Button *button10 = button_push();
    TextView *view = textview_create();
    data->view = view;
    i_popups(layout2, data);
    i_combos(layout2, data);
    button_text(button1, "Add elem to PopUp");
    button_text(button2, "Insert elem to PopUp");
    button_text(button3, "Delete elem from PopUp");
    button_text(button4, "Clear PopUp");
    button_text(button5, "Get Select PopUp");
    button_text(button6, "Add elem to Combo");
    button_text(button7, "Insert elem to Combo");
    button_text(button8, "Delete elem from Combo");
    button_text(button9, "Clear Combo");
    button_text(button10, "Get Select Combo");
    button_OnClick(button1, listener(data, i_OnPopUpAdd, PopUpData));
    button_OnClick(button2, listener(data, i_OnPopUpInsert, PopUpData));
    button_OnClick(button3, listener(data, i_OnPopUpDelete, PopUpData));
    button_OnClick(button4, listener(data, i_OnPopUpClear, PopUpData));
    button_OnClick(button5, listener(data, i_OnPopUpGetSel, PopUpData));
    button_OnClick(button6, listener(data, i_OnComboAdd, PopUpData));
    button_OnClick(button7, listener(data, i_OnComboInsert, PopUpData));
    button_OnClick(button8, listener(data, i_OnComboDelete, PopUpData));
    button_OnClick(button9, listener(data, i_OnComboClear, PopUpData));
    button_OnClick(button10, listener(data, i_OnComboGetSel, PopUpData));
    layout_button(layout2, button1, 1, 2);
    layout_button(layout2, button2, 1, 3);
    layout_button(layout2, button3, 1, 4);
    layout_button(layout2, button4, 1, 5);
    layout_button(layout2, button5, 1, 6);
    layout_button(layout2, button6, 3, 2);
    layout_button(layout2, button7, 3, 3);
    layout_button(layout2, button8, 3, 4);
    layout_button(layout2, button9, 3, 5);
    layout_button(layout2, button10, 3, 6);
    layout_layout(layout1, layout2, 0, 0);
    layout_textview(layout1, view, 0, 1);
    layout_margin(layout1, 10);
    layout_vmargin(layout2, 0, 10);
    layout_vmargin(layout2, 1, 10);
    layout_vmargin(layout2, 2, 5);
    layout_vmargin(layout2, 3, 5);
    layout_vmargin(layout2, 4, 5);
    layout_vmargin(layout2, 5, 5);
    layout_vmargin(layout1, 0, 10);
    layout_hmargin(layout2, 0, 5);
    layout_hmargin(layout2, 1, 10);
    layout_hmargin(layout2, 2, 5);
    panel_layout(panel, layout1);
    panel_data(panel, &data, i_destroy_data, PopUpData);
    return panel;
}
