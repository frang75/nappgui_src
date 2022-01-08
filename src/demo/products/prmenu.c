/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: prmenu.c
 *
 */

/* Products Menu */

#include "prmenu.h"
#include "prctrl.h"
#include "guiall.h"
#include "res_gui.h"

/*---------------------------------------------------------------------------*/

#if defined (__APPLE__)
static Menu *i_app(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    MenuItem *item1 = menuitem_separator();
    MenuItem *item2 = menuitem_create();
    MenuItem *item3 = menuitem_separator();
    MenuItem *item4 = menuitem_create();
    menuitem_text(item0, TMEN_ABOUT);
    menuitem_text(item2, TMEN_PREFERS);
    menuitem_text(item4, TMEN_QUIT);
    menu_item(menu, item0);
    menu_item(menu, item1);
    menu_item(menu, item2);
    menu_item(menu, item3);
    menu_item(menu, item4);
    ctrl_about_item(ctrl, item0);
    ctrl_exit_item(ctrl, item4);
    return menu;
}
#endif

/*---------------------------------------------------------------------------*/

static Menu *i_file(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    MenuItem *item1 = menuitem_create();
    menuitem_text(item0, TMEN_IMPORT);
    menuitem_text(item1, TMEN_EXPORT);
    menu_item(menu, item0);
    menu_item(menu, item1);

#if !defined(__APPLE__)
    {
        MenuItem *item2 = menuitem_separator();
        MenuItem *item3 = menuitem_create();
        menuitem_text(item3, TMEN_EXIT);
        menuitem_image(item3, (const Image*)EXIT_PNG);
        menu_item(menu, item2);
        menu_item(menu, item3);
        ctrl_exit_item(ctrl, item3);
    }
#endif

    ctrl_import_item(ctrl, item0);
    ctrl_export_item(ctrl, item1);
    return menu;
}

/*---------------------------------------------------------------------------*/

static Menu *i_navigate(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    MenuItem *item1 = menuitem_create();
    MenuItem *item2 = menuitem_create();
    MenuItem *item3 = menuitem_create();
    menuitem_text(item0, TMEN_FIRST);
    menuitem_text(item1, TMEN_BACK);
    menuitem_text(item2, TMEN_NEXT);
    menuitem_text(item3, TMEN_LAST);
    menuitem_key(item0, ekKEY_F5, 0);
    menuitem_key(item1, ekKEY_F6, 0);
    menuitem_key(item2, ekKEY_F7, 0);
    menuitem_key(item3, ekKEY_F8, 0);
    menu_item(menu, item0);
    menu_item(menu, item1);
    menu_item(menu, item2);
    menu_item(menu, item3);
    ctrl_first_item(ctrl, item0);
    ctrl_back_item(ctrl, item1);
    ctrl_next_item(ctrl, item2);
    ctrl_last_item(ctrl, item3);
    return menu;
}

/*---------------------------------------------------------------------------*/

static Menu *i_view(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    unref(ctrl);
    menuitem_text(item0, TMEN_LOGIN_PANEL);
    menuitem_image(item0, (const Image*)SETTINGS16_PNG);
    menu_item(menu, item0);
    ctrl_setting_item(ctrl, item0);
    return menu;
}

/*---------------------------------------------------------------------------*/

static Menu *i_server(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    MenuItem *item1 = menuitem_create();
    menuitem_text(item0, TMEN_LOGIN);
    menuitem_text(item1, TMEN_LOGOUT);
    menu_item(menu, item0);
    menu_item(menu, item1);
    ctrl_login_item(ctrl, item0);
    ctrl_logout_item(ctrl, item1);
    return menu;
}

/*---------------------------------------------------------------------------*/

static Menu *i_language(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    MenuItem *item1 = menuitem_create();
    MenuItem *item2 = menuitem_create();
    MenuItem *item3 = menuitem_create();
    MenuItem *item4 = menuitem_create();
    MenuItem *item5 = menuitem_create();
    MenuItem *item6 = menuitem_create();
    menuitem_text(item0, ENGLISH);
    menuitem_text(item1, SPANISH);
    menuitem_text(item2, PORTUGUESE);
    menuitem_text(item3, ITALIAN);
    menuitem_text(item4, VIETNAMESE);
    menuitem_text(item5, RUSSIAN);
    menuitem_text(item6, JAPANESE);
    menuitem_image(item0, (const Image*)USA_PNG);
    menuitem_image(item1, (const Image*)SPAIN_PNG);
    menuitem_image(item2, (const Image*)PORTUGAL_PNG);
    menuitem_image(item3, (const Image*)ITALY_PNG);
    menuitem_image(item4, (const Image*)VIETNAM_PNG);
    menuitem_image(item5, (const Image*)RUSSIA_PNG);
    menuitem_image(item6, (const Image*)JAPAN_PNG);
    menu_item(menu, item0);
    menu_item(menu, item1);
    menu_item(menu, item2);
    menu_item(menu, item3);
    menu_item(menu, item4);
    menu_item(menu, item5);
    menu_item(menu, item6);
    ctrl_lang_menu(ctrl, menu);
    return menu;
}

/*---------------------------------------------------------------------------*/

#if !defined (__APPLE__)
static Menu *i_help(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item0 = menuitem_create();
    menuitem_text(item0, TMEN_ABOUT);
    menuitem_image(item0, (const Image*)ABOUT_PNG);
    menu_item(menu, item0);
    ctrl_about_item(ctrl, item0);
    return menu;
}
#endif

/*---------------------------------------------------------------------------*/

Menu *prmenu_create(Ctrl *ctrl)
{
    Menu *menu = menu_create();
    MenuItem *item1 = menuitem_create();
    MenuItem *item2 = menuitem_create();
    MenuItem *item3 = menuitem_create();
    MenuItem *item4 = menuitem_create();
    MenuItem *item5 = menuitem_create();
    Menu *submenu1 = i_file(ctrl);
    Menu *submenu2 = i_navigate(ctrl);
    Menu *submenu3 = i_view(ctrl);
    Menu *submenu4 = i_server(ctrl);
    Menu *submenu5 = i_language(ctrl);
    
 #if defined (__APPLE__)
    {
        MenuItem *item0 = menuitem_create();
        Menu *submenu0 = i_app(ctrl);
        menuitem_text(item1, "");
        menuitem_submenu(item0, &submenu0);
        menu_item(menu, item0);
    }
#endif

    menuitem_text(item1, TMEN_FILE);
    menuitem_text(item2, TMEN_NAVIGATE);
    menuitem_text(item3, TMEN_VIEW);
    menuitem_text(item4, TMEN_SERVER);
    menuitem_text(item5, LANGUAGE);
    menuitem_submenu(item1, &submenu1);
    menuitem_submenu(item2, &submenu2);
    menuitem_submenu(item3, &submenu3);
    menuitem_submenu(item4, &submenu4);
    menuitem_submenu(item5, &submenu5);
    menu_item(menu, item1);
    menu_item(menu, item2);
    menu_item(menu, item3);
    menu_item(menu, item4);
    menu_item(menu, item5);

 #if !defined (__APPLE__)
    {
        MenuItem *item6 = menuitem_create();
        Menu *submenu6 = i_help(ctrl);
        menuitem_text(item6, TMEN_HELP);
        menuitem_submenu(item6, &submenu6);
        menu_item(menu, item6);
    }
#endif
    return menu;
}
