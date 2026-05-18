/* Tabcontrols */

#include "tabctrls.h"
#include "res_guihello.h"
#include <gui/guiall.h>

typedef struct _tabsdata_t TabsData;

struct _tabsdata_t
{
    Tabs *tabs_top;
    Tabs *tabs_bottom;
    Tabs *tabs_left;
    Tabs *tabs_right;
    TextView *view;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(TabsData **data)
{
    heap_delete(data, TabsData);
}

/*---------------------------------------------------------------------------*/

static void i_OnTabSelect(TabsData *data, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    Tabs *sender = event_sender(e, Tabs);
    const char_t *name = "";
    cassert_no_null(data);
    if (sender == data->tabs_top)
        name = "Top";
    else if (sender == data->tabs_bottom)
        name = "Bottom";
    else if (sender == data->tabs_left)
        name = "Left";
    else if (sender == data->tabs_right)
        name = "Right";
    textview_printf(data->view, "%s Tabs OnSelect %d '%s'\n", name, p->index, p->text);
    textview_scroll_caret(data->view);
}

/*---------------------------------------------------------------------------*/

static void i_OnTabAdd(TabsData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    tabs_add_elem(data->tabs_top, "Español", gui_image(SPAIN_PNG));
    tabs_add_elem(data->tabs_bottom, "Español", gui_image(SPAIN_PNG));
    tabs_add_elem(data->tabs_left, "Español", gui_image(SPAIN_PNG));
    tabs_add_elem(data->tabs_right, "Español", gui_image(SPAIN_PNG));
}

/*---------------------------------------------------------------------------*/

static void i_OnTabInsert(TabsData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    tabs_ins_elem(data->tabs_top, 0, "English", gui_image(UKING_PNG));
    tabs_ins_elem(data->tabs_bottom, 0, "English", gui_image(UKING_PNG));
    tabs_ins_elem(data->tabs_left, 0, "English", gui_image(UKING_PNG));
    tabs_ins_elem(data->tabs_right, 0, "English", gui_image(UKING_PNG));
}

/*---------------------------------------------------------------------------*/

static void i_OnTabDelete(TabsData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    if (tabs_count(data->tabs_top) > 0)
    {
        tabs_del_elem(data->tabs_top, 0);
        tabs_del_elem(data->tabs_bottom, 0);
        tabs_del_elem(data->tabs_left, 0);
        tabs_del_elem(data->tabs_right, 0);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnTabClear(TabsData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    tabs_clear(data->tabs_top);
    tabs_clear(data->tabs_bottom);
    tabs_clear(data->tabs_left);
    tabs_clear(data->tabs_right);
}

/*---------------------------------------------------------------------------*/

static void i_OnTabGetSel(TabsData *data, Event *e)
{
    cassert_no_null(data);
    unref(e);
    textview_printf(data->view, "Top Tabs selected: %d\n", tabs_get_selected(data->tabs_top));
    textview_printf(data->view, "Bottom Tabs selected: %d\n", tabs_get_selected(data->tabs_bottom));
    textview_printf(data->view, "Left Tabs selected: %d\n", tabs_get_selected(data->tabs_left));
    textview_printf(data->view, "Right Tabs selected: %d\n", tabs_get_selected(data->tabs_right));
    textview_scroll_caret(data->view);
}

/*---------------------------------------------------------------------------*/

static Layout *i_buttons_layout(TabsData *data)
{
    Layout *layout = layout_create(1, 5);
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Button *button5 = button_push();
    button_text(button1, "Add tab to the end");
    button_text(button2, "Insert tab at begining");
    button_text(button3, "Delete first tab");
    button_text(button4, "Clear tabs");
    button_text(button5, "Get selected tab");
    button_OnClick(button1, listener(data, i_OnTabAdd, TabsData));
    button_OnClick(button2, listener(data, i_OnTabInsert, TabsData));
    button_OnClick(button3, listener(data, i_OnTabDelete, TabsData));
    button_OnClick(button4, listener(data, i_OnTabClear, TabsData));
    button_OnClick(button5, listener(data, i_OnTabGetSel, TabsData));
    layout_button(layout, button1, 0, 0);
    layout_button(layout, button2, 0, 1);
    layout_button(layout, button3, 0, 2);
    layout_button(layout, button4, 0, 3);
    layout_button(layout, button5, 0, 4);
    layout_vmargin(layout, 0, 5);
    layout_vmargin(layout, 1, 5);
    layout_vmargin(layout, 2, 5);
    layout_vmargin(layout, 3, 5);
    return layout;
}

/*---------------------------------------------------------------------------*/

static Tabs *i_tabs_create(const gui_pos_t pos)
{
    Tabs *tabs = tabs_create(pos);
    tabs_add_elem(tabs, "English", gui_image(UKING_PNG));
    tabs_add_elem(tabs, "Español", gui_image(SPAIN_PNG));
    tabs_add_elem(tabs, "Portugues", gui_image(PORTUGAL_PNG));
    tabs_add_elem(tabs, "Italiana", gui_image(ITALY_PNG));
    tabs_add_elem(tabs, "Tiếng Việt", gui_image(VIETNAM_PNG));
    tabs_add_elem(tabs, "России", gui_image(RUSSIA_PNG));
    tabs_add_elem(tabs, "日本語", gui_image(JAPAN_PNG));
    tabs_tooltip(tabs, "TabControl Tooltip");
    return tabs;
}

/*---------------------------------------------------------------------------*/

static Layout *i_layout(TabsData *data)
{
    Layout *layout1 = layout_create(1, 4);
    Layout *layout2 = layout_create(3, 1);
    Layout *layout3 = i_buttons_layout(data);
    Tabs *tabs1 = i_tabs_create(ekGUI_POS_TOP);
    Tabs *tabs2 = i_tabs_create(ekGUI_POS_BOTTOM);
    Tabs *tabs3 = i_tabs_create(ekGUI_POS_LEFT);
    Tabs *tabs4 = i_tabs_create(ekGUI_POS_RIGHT);
    TextView *view = textview_create();
    cassert_no_null(data);
    layout_tabs(layout2, tabs3, 0, 0);
    layout_layout(layout2, layout3, 1, 0);
    layout_tabs(layout2, tabs4, 2, 0);
    layout_tabs(layout1, tabs1, 0, 0);
    layout_layout(layout1, layout2, 0, 1);
    layout_tabs(layout1, tabs2, 0, 2);
    layout_textview(layout1, view, 0, 3);
    tabs_OnSelect(tabs1, listener(data, i_OnTabSelect, TabsData));
    tabs_OnSelect(tabs2, listener(data, i_OnTabSelect, TabsData));
    tabs_OnSelect(tabs3, listener(data, i_OnTabSelect, TabsData));
    tabs_OnSelect(tabs4, listener(data, i_OnTabSelect, TabsData));
    data->tabs_top = tabs1;
    data->tabs_bottom = tabs2;
    data->tabs_left = tabs3;
    data->tabs_right = tabs4;
    data->view = view;
    return layout1;
}

/*---------------------------------------------------------------------------*/

Panel *tabctrls(void)
{
    TabsData *data = heap_new0(TabsData);
    Panel *panel = panel_create();
    Layout *layout = i_layout(data);
    panel_layout(panel, layout);
    panel_data(panel, &data, i_destroy_data, TabsData);
    return panel;
}
