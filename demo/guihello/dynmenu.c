/* Dynamic menus */

#include "dynmenu.h"
#include "res_guihello.h"
#include <gui/guiall.h>
#include <osapp/osapp.h>

typedef struct _dynmenu_t DynMenu;

struct _dynmenu_t
{
    Menu *menu;
    Window *window;
    PopUp *popup;
    TextView *text;
    Button *hide_check;
    Button *enable_check;
    Button *state_check;
    uint32_t itemid;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_dynmenu(DynMenu **dmenu)
{
    cassert_no_null(dmenu);
    menu_destroy(&(*dmenu)->menu);
    heap_delete(dmenu, DynMenu);
}

/*---------------------------------------------------------------------------*/

static void i_OnMenuClick(DynMenu *dmenu, Event *e)
{
    MenuItem *item = event_sender(e, MenuItem);
    const char_t *text = menuitem_get_text(item);
    cassert_no_null(dmenu);
    textview_printf(dmenu->text, "Click: %s\n", text);
}

/*---------------------------------------------------------------------------*/

static void i_select_item(DynMenu *dmenu, const uint32_t index)
{
    MenuItem *item = NULL;
    bool_t visible = TRUE;
    bool_t enabled = TRUE;
    cassert_no_null(dmenu);
    item = menu_get_item(dmenu->menu, index);
    visible = menuitem_get_visible(item);
    enabled = menuitem_get_enabled(item);
    button_state(dmenu->hide_check, visible ? ekGUI_OFF : ekGUI_ON);
    button_state(dmenu->enable_check, enabled ? ekGUI_ON : ekGUI_OFF);
    button_state(dmenu->state_check, menuitem_get_state(item));
}

/*---------------------------------------------------------------------------*/

static void i_index_popup(DynMenu *dmenu)
{
    uint32_t n;
    cassert_no_null(dmenu);
    n = menu_count(dmenu->menu);
    popup_clear(dmenu->popup);
    if (n > 0)
    {
        uint32_t i;
        for (i = 0; i < n; ++i)
        {
            const MenuItem *item = menu_get_citem(dmenu->menu, i);
            popup_add_elem(dmenu->popup, menuitem_get_text(item), NULL);
        }

        i_select_item(dmenu, popup_get_selected(dmenu->popup));
    }
}

/*---------------------------------------------------------------------------*/

static MenuItem *i_menuitem(DynMenu *dmenu, const char_t *text, const Image *image)
{
    MenuItem *item = menuitem_create();
    menuitem_text(item, text);
    menuitem_image(item, image);
    menuitem_OnClick(item, listener(dmenu, i_OnMenuClick, DynMenu));
    return item;
}

/*---------------------------------------------------------------------------*/

static Menu *i_menu(DynMenu *dmenu)
{
    Menu *menu = menu_create();
    Menu *submenu = menu_create();
    MenuItem *item1 = i_menuitem(dmenu, "English", gui_image(UKING_PNG));
    MenuItem *item2 = i_menuitem(dmenu, "Español", gui_image(SPAIN_PNG));
    MenuItem *item3 = i_menuitem(dmenu, "Portugues", gui_image(PORTUGAL_PNG));
    MenuItem *item4 = i_menuitem(dmenu, "Italiana", gui_image(ITALY_PNG));
    MenuItem *item5 = i_menuitem(dmenu, "Tiếng Việt", gui_image(VIETNAM_PNG));
    MenuItem *item6 = i_menuitem(dmenu, "России", gui_image(RUSSIA_PNG));
    MenuItem *item7 = i_menuitem(dmenu, "日本語", gui_image(JAPAN_PNG));
    MenuItem *item8 = i_menuitem(dmenu, "Red", gui_image(RED_PNG));
    MenuItem *item9 = i_menuitem(dmenu, "Blue", gui_image(BLUE_PNG));
    MenuItem *item10 = i_menuitem(dmenu, "Green", gui_image(GREEN_PNG));
    MenuItem *item11 = i_menuitem(dmenu, "Yellow", gui_image(YELLOW_PNG));
    MenuItem *item12 = i_menuitem(dmenu, "Black", gui_image(BLACK_PNG));
    MenuItem *item13 = i_menuitem(dmenu, "White", gui_image(WHITE_PNG));
    MenuItem *item14 = menuitem_create();
    menu_add_item(menu, item1);
    menu_add_item(menu, item2);
    menu_add_item(menu, item3);
    menu_add_item(menu, item4);
    menu_add_item(menu, item5);
    menu_add_item(menu, item6);
    menu_add_item(menu, item7);
    menu_add_item(submenu, item8);
    menu_add_item(submenu, item9);
    menu_add_item(submenu, item10);
    menu_add_item(submenu, item11);
    menu_add_item(submenu, menuitem_separator());
    menu_add_item(submenu, item12);
    menu_add_item(submenu, item13);
    menuitem_text(item14, "Colors");
    menuitem_submenu(item14, &submenu);
    menu_add_item(menu, menuitem_separator());
    menu_add_item(menu, item14);
    return menu;
}

/*---------------------------------------------------------------------------*/

static DynMenu *i_dynmenu(Window *window)
{
    DynMenu *dmenu = heap_new0(DynMenu);
    dmenu->menu = i_menu(dmenu);
    dmenu->window = window;
    return dmenu;
}

/*---------------------------------------------------------------------------*/

static void i_OnSelectItem(DynMenu *dmenu, Event *e)
{
    const EvButton *p = event_params(e, EvButton);
    i_select_item(dmenu, p->index);
}

/*---------------------------------------------------------------------------*/

static void i_OnHideClick(DynMenu *dmenu, Event *e)
{
    cassert_no_null(dmenu);
    if (menu_count(dmenu->menu) > 0)
    {
        const EvButton *p = event_params(e, EvButton);
        uint32_t index = popup_get_selected(dmenu->popup);
        MenuItem *item = menu_get_item(dmenu->menu, index);
        bool_t visible = p->state == ekGUI_OFF;
        menuitem_visible(item, visible);
        window_update(dmenu->window);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnEnableClick(DynMenu *dmenu, Event *e)
{
    cassert_no_null(dmenu);
    if (menu_count(dmenu->menu) > 0)
    {
        const EvButton *p = event_params(e, EvButton);
        uint32_t index = popup_get_selected(dmenu->popup);
        MenuItem *item = menu_get_item(dmenu->menu, index);
        bool_t enabled = p->state == ekGUI_ON;
        menuitem_enabled(item, enabled);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnStateClick(DynMenu *dmenu, Event *e)
{
    cassert_no_null(dmenu);
    if (menu_count(dmenu->menu) > 0)
    {
        const EvButton *p = event_params(e, EvButton);
        uint32_t index = popup_get_selected(dmenu->popup);
        MenuItem *item = menu_get_item(dmenu->menu, index);
        menuitem_state(item, p->state);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnSetMenubar(DynMenu *dmenu, Event *e)
{
    cassert_no_null(dmenu);
    unref(e);

    if (menu_is_menubar(dmenu->menu) == FALSE)
    {
        /*
           In apple menubar, the first menu item is attached
           automatically to app name menu entry.
         */
#if defined(__APPLE__)
        MenuItem *item00 = menuitem_create();
        Menu *apple_app_menu = menu_create();
        MenuItem *item01 = menuitem_create();
        MenuItem *item02 = menuitem_create();
        menuitem_text(item00, "");
        menuitem_text(item01, "About...");
        menuitem_text(item02, "Settings...");
        menu_add_item(apple_app_menu, item01);
        menu_add_item(apple_app_menu, item02);
        menuitem_submenu(item00, &apple_app_menu);
        menu_ins_item(dmenu->menu, 0, item00);
        i_index_popup(dmenu);
#endif
        osapp_menubar(dmenu->menu, dmenu->window);
    }
}

/*---------------------------------------------------------------------------*/

static void i_unset_menubar(DynMenu *dmenu)
{
    cassert_no_null(dmenu);
    if (menu_is_menubar(dmenu->menu) == TRUE)
    {
        /* Remove the Apple 'appName' first menu entry */
#if defined(__APPLE__)
        menu_del_item(dmenu->menu, 0);
        i_index_popup(dmenu);
#endif
        osapp_menubar(NULL, dmenu->window);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnUnsetMenubar(DynMenu *dmenu, Event *e)
{
    unref(e);
    i_unset_menubar(dmenu);
}

/*---------------------------------------------------------------------------*/

static void i_OnPopUpMenu(DynMenu *dmenu, Event *e)
{
    V2Df pos;
    cassert_no_null(dmenu);
    unref(e);

    /* First, unlink the menu if has a menubar role */
    i_unset_menubar(dmenu);

    /* Screen position of top-left textview corner */
    {
        R2Df frame = window_control_frame(dmenu->window, guicontrol(dmenu->text));
        pos = window_client_to_screen(dmenu->window, frame.pos);
    }

    /* Launch as popup */
    menu_launch(dmenu->menu, pos);
}

/*---------------------------------------------------------------------------*/

static void i_OnInsertItem(DynMenu *dmenu, Event *e)
{
    MenuItem *item = menuitem_create();
    char_t text[64];
    uint32_t pos = 0;
    cassert_no_null(dmenu);
    unref(e);
    bstd_sprintf(text, sizeof(text), "NewItem%d", dmenu->itemid++);
    menuitem_text(item, text);
    menuitem_OnClick(item, listener(dmenu, i_OnMenuClick, DynMenu));

#if defined(__APPLE__)
    /* In macOS, we insert the new item AFTER the 'appName' menu item */
    if (menu_is_menubar(dmenu->menu) == TRUE)
        pos = 1;
#endif

    menu_ins_item(dmenu->menu, pos, item);
    i_index_popup(dmenu);
    window_update(dmenu->window);
}

/*---------------------------------------------------------------------------*/

static void i_OnRemoveItem(DynMenu *dmenu, Event *e)
{
    uint32_t pos = 0;
    cassert_no_null(dmenu);
    unref(e);

#if defined(__APPLE__)
    /* In macOS, we remove the item AFTER the 'appName' menu item */
    if (menu_is_menubar(dmenu->menu) == TRUE)
        pos = 1;
#endif

    if (menu_count(dmenu->menu) > pos)
    {
        menu_del_item(dmenu->menu, pos);
        i_index_popup(dmenu);
        window_update(dmenu->window);
    }
}

/*---------------------------------------------------------------------------*/

Panel *dynmenu_panel(Window *window)
{
    DynMenu *dmenu = i_dynmenu(window);
    Layout *layout1 = layout_create(1, 8);
    Layout *layout2 = layout_create(5, 1);
    Label *label1 = label_create();
    Label *label2 = label_create();
    Button *button1 = button_push();
    Button *button2 = button_push();
    Button *button3 = button_push();
    Button *button4 = button_push();
    Button *button5 = button_push();
    Button *button6 = button_check();
    Button *button7 = button_check();
    Button *button8 = button_check();
    PopUp *popup = popup_create();
    TextView *text = textview_create();
    Panel *panel = panel_create();
    dmenu->text = text;
    label_text(label1, "NAppGUI Dynamic Menus API");
    label_text(label2, "Item");
    button_text(button1, "Set as menubar");
    button_text(button2, "Unset menubar");
    button_text(button3, "Launch as popup");
    button_text(button4, "Insert item at 0");
    button_text(button5, "Remove item at 0");
    button_text(button6, "Hide");
    button_text(button7, "Enable");
    button_text(button8, "Check");
    button_OnClick(button1, listener(dmenu, i_OnSetMenubar, DynMenu));
    button_OnClick(button2, listener(dmenu, i_OnUnsetMenubar, DynMenu));
    button_OnClick(button3, listener(dmenu, i_OnPopUpMenu, DynMenu));
    button_OnClick(button4, listener(dmenu, i_OnInsertItem, DynMenu));
    button_OnClick(button5, listener(dmenu, i_OnRemoveItem, DynMenu));
    button_OnClick(button6, listener(dmenu, i_OnHideClick, DynMenu));
    button_OnClick(button7, listener(dmenu, i_OnEnableClick, DynMenu));
    button_OnClick(button8, listener(dmenu, i_OnStateClick, DynMenu));
    popup_OnSelect(popup, listener(dmenu, i_OnSelectItem, DynMenu));
    layout_label(layout1, label1, 0, 0);
    layout_button(layout1, button1, 0, 1);
    layout_button(layout1, button2, 0, 2);
    layout_button(layout1, button3, 0, 3);
    layout_button(layout1, button4, 0, 4);
    layout_button(layout1, button5, 0, 5);
    layout_layout(layout1, layout2, 0, 6);
    layout_textview(layout1, text, 0, 7);
    layout_label(layout2, label2, 0, 0);
    layout_popup(layout2, popup, 1, 0);
    layout_button(layout2, button6, 2, 0);
    layout_button(layout2, button7, 3, 0);
    layout_button(layout2, button8, 4, 0);
    layout_halign(layout1, 0, 6, ekLEFT);
    layout_hmargin(layout2, 0, 5);
    layout_hmargin(layout2, 1, 5);
    layout_hmargin(layout2, 2, 5);
    layout_hmargin(layout2, 3, 5);
    panel_layout(panel, layout1);
    dmenu->popup = popup;
    dmenu->hide_check = button6;
    dmenu->enable_check = button7;
    dmenu->state_check = button8;
    i_index_popup(dmenu);
    panel_data(panel, &dmenu, i_destroy_dynmenu, DynMenu);
    return panel;
}
