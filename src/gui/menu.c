/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: menu.c
 *
 */

/* Menu */

#include "menu.h"
#include "menu.inl"
#include "gui.inl"
#include "menuitem.h"
#include "menuitem.inl"
#include "window.h"
#include "window.inl"
#include <draw2d/guictx.h>
#include <core/arrpt.h>
#include <core/objh.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _menu_t
{
    Object object;
    GuiCtx *context;
    bool_t has_parent;
    void *ositem;
    ArrPt(MenuItem) *items;
};

/*---------------------------------------------------------------------------*/

static Menu *i_create_menu(const GuiCtx *context, const bool_t has_parent, void **ositem, ArrPt(MenuItem) **items)
{
    Menu *menu = obj_new(Menu);
    menu->context = guictx_retain(context);
    menu->has_parent = has_parent;
    menu->ositem = ptr_dget_no_null(ositem, void);
    menu->items = ptr_dget_no_null(items, ArrPt(MenuItem));
    _gui_add_menu(menu);
    return menu;
}

/*---------------------------------------------------------------------------*/

Menu *menu_create(void)
{
    const GuiCtx *context = NULL;
    bool_t has_parent = FALSE;
    void *ositem = NULL;
    ArrPt(MenuItem) *items;
    context = guictx_get_current();
    cassert_no_null(context);
    cassert_no_nullf(context->func_menu_create);
    ositem = context->func_menu_create((enum_t)0);
    items = arrpt_create(MenuItem);
    return i_create_menu(context, has_parent, &ositem, &items);
}

/*---------------------------------------------------------------------------*/

static void i_detach_menuitems(ArrPt(MenuItem) *items, void *ositem, FPtr_gctx_set_ptr func_menu_delete_item)
{
    uint32_t i, num_items;
    MenuItem **item;
    cassert_no_nullf(func_menu_delete_item);
    num_items = arrpt_size(items, MenuItem);
    item = arrpt_all(items, MenuItem);
    for (i = 0; i < num_items; ++i, ++item)
    {
        void *renderable_menu_item;
        _menuitem_unset_parent(*item);
        renderable_menu_item = _menuitem_get_renderable(*item);
        func_menu_delete_item(ositem, renderable_menu_item);
    }
}

/*---------------------------------------------------------------------------*/

static void i_destroy(Menu **menu)
{
    cassert_no_null(menu);
    cassert_no_null(*menu);
    cassert((*menu)->has_parent == FALSE);
    cassert_no_null((*menu)->context);
    cassert_no_nullf((*menu)->context->func_menu_destroy);
    i_detach_menuitems((*menu)->items, (*menu)->ositem, (*menu)->context->func_menu_delete_item);
    arrpt_destroy(&(*menu)->items, _menuitem_destroy, MenuItem);
    (*menu)->context->func_menu_destroy(&(*menu)->ositem);
    guictx_release(&(*menu)->context);
    obj_delete(menu, Menu);
}

/*---------------------------------------------------------------------------*/

void menu_destroy(Menu **menu)
{
    cassert_no_null(menu);
    _gui_delete_menu(*menu);
    i_destroy(menu);
}

/*---------------------------------------------------------------------------*/

void _menu_destroy(Menu **menu)
{
    i_destroy(menu);
}

/*---------------------------------------------------------------------------*/

void menu_add_item(Menu *menu, MenuItem *item)
{
    uint32_t pos = UINT32_MAX;
    void *ositem = NULL;
    cassert_no_null(menu);
    cassert_no_null(menu->context);
    cassert_no_nullf(menu->context->func_menu_insert_item);
    cassert_no_null(item);
    _menuitem_set_parent(item, arrpt_size(menu->items, MenuItem));
    ositem = _menuitem_get_renderable(item);
    pos = arrpt_size(menu->items, MenuItem);
    menu->context->func_menu_insert_item(menu->ositem, pos, ositem);
    arrpt_append(menu->items, item, MenuItem);
}

/*---------------------------------------------------------------------------*/

void menu_ins_item(Menu *menu, const uint32_t pos, MenuItem *item)
{
    void *ositem = NULL;
    cassert_no_null(menu);
    cassert_no_null(menu->context);
    cassert_no_nullf(menu->context->func_menu_insert_item);
    cassert_no_null(item);
    _menuitem_set_parent(item, arrpt_size(menu->items, MenuItem));
    ositem = _menuitem_get_renderable(item);
    menu->context->func_menu_insert_item(menu->ositem, pos, ositem);
    arrpt_insert(menu->items, pos, item, MenuItem);
}

/*---------------------------------------------------------------------------*/

void menu_del_item(Menu *menu, const uint32_t pos)
{
    MenuItem *item = NULL;
    void *ositem = NULL;
    cassert_no_null(menu);
    item = arrpt_get(menu->items, pos, MenuItem);
    cassert_no_null(item);
    _menuitem_unset_parent(item);
    ositem = _menuitem_get_renderable(item);
    menu->context->func_menu_delete_item(menu->ositem, ositem);
    _menuitem_destroy(&item);
    arrpt_delete(menu->items, pos, NULL, MenuItem);
}

/*---------------------------------------------------------------------------*/

void menu_launch(Menu *menu, Window *window, const V2Df position)
{
    void *oswindow = NULL;
    cassert_no_null(menu);
    cassert_no_null(menu->context);
    cassert_no_nullf(menu->context->func_menu_launch_popup);
    oswindow = _window_ositem(window);
    menu->context->func_menu_launch_popup(menu->ositem, oswindow, position.x, position.y);
    window_update(window);
}

/*---------------------------------------------------------------------------*/

void menu_off_items(Menu *menu)
{
    cassert_no_null(menu);
    arrpt_foreach(item, menu->items, MenuItem)
        if (menuitem_get_separator(item) == FALSE)
            menuitem_state(item, ekGUI_OFF);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

uint32_t menu_count(const Menu *menu)
{
    cassert_no_null(menu);
    return arrpt_size(menu->items, MenuItem);
}

/*---------------------------------------------------------------------------*/

MenuItem *menu_get_item(Menu *menu, const uint32_t index)
{
    cassert_no_null(menu);
    return arrpt_get(menu->items, index, MenuItem);
}

/*---------------------------------------------------------------------------*/

const MenuItem *menu_get_citem(const Menu *menu, const uint32_t index)
{
    cassert_no_null(menu);
    return arrpt_get_const(menu->items, index, MenuItem);
}

/*---------------------------------------------------------------------------*/

bool_t menu_is_menubar(const Menu *menu)
{
    cassert_no_null(menu);
    cassert_no_null(menu->context);
    return menu->context->func_menu_is_menubar(menu->ositem);
}

/*---------------------------------------------------------------------------*/

void *menu_imp(const Menu *menu)
{
    cassert_no_null(menu);
    return menu->ositem;
}

/*---------------------------------------------------------------------------*/

void _menu_set_parent(Menu *menu)
{
    cassert_no_null(menu);
    cassert(menu->has_parent == FALSE);
    menu->has_parent = TRUE;
}

/*---------------------------------------------------------------------------*/

void _menu_unset_parent(Menu *menu)
{
    cassert_no_null(menu);
    cassert(menu->has_parent == TRUE);
    menu->has_parent = FALSE;
}

/*---------------------------------------------------------------------------*/

void _menu_locale(Menu *menu)
{
    cassert_no_null(menu);
    arrpt_foreach(item, menu->items, MenuItem)
        _menuitem_locale(item);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void *_menu_ositem(Menu *menu)
{
    cassert_no_null(menu);
    return menu->ositem;
}
