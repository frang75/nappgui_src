/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenu.c
 *
 */

/* Operating System native menu */

#include "osmenu_win.inl"
#include "osgui_win.inl"
#include "osmenuitem_win.inl"
#include "oswindow_win.inl"
#include "../osmenu.h"
#include <core/arrpt.h>
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

/*
    https://blogs.msdn.microsoft.com/oldnewthing/20031230-00/?p=41273

    CreateMenu creates a horizontal menu bar, suitable for attaching to a top-level window. This is the sort of menu that says "File, Edit", and so on.
    CreatePopupMenu creates a vertical popup menu, suitable for use as a submenu of another menu (either a horizontal menu bar or another popup menu) or as the root of a context menu.
    If you get the two confused, you can get strange menu behavior. Windows on rare occasions detects that you confused the two and converts as appropriate, but I wouldn't count on Windows successfully reading your mind.

    There is no way to take a menu and ASK IT WHETHER IT IS HORIZONTAL OR VERTICAL. You just have to know.

    Answers to other questions about menus:
        When a window is destroyed, its menu is also destroyed. When a menu is destroyed, the entire menu tree is destroyed. (All its submenus are destroyed, all the submenu's submenus, etc.) And when you destroy a menu, it had better not be the submenu of some other menu. That other menu would have an invalid menu as a submenu!
        If you remove a submenu from its parent, then you become responsible for destroying it, since it no longer gets destroyed automatically when the parent is destroyed.
        It is legal for a menu to be a submenu of multiple parent menus. Be extra careful when you do this, however, because if one of the parents is destroyed, it will destroy the submenu with it, leaving the other parent with an invalid submenu.
        And finally: The menu nesting limit is currently 25 on Windows XP. That may change in the future, of course. (As with window nesting, Windows 95 let you go ahead and nest menus all you wanted. In fact, you could go really evil and create an infinite loop of menus. You crashed pretty quickly thereafter, of course...)
*/

/*
 *  hmenu == NULL --> Win32 Menu not created
 *  hmenu != NULL && parent != NULL --> hmenu CreatePopupMenu() (submenu)
 *  hmenu != NULL && is_popup --> hmenu CreatePopupMenu() (popupmenu)
 *  hmenu != NULL && !is_popup --> hmenu CreateMenu() (menubar)
 */
struct _osmenu_t
{
    HMENU hmenu;
    bool_t is_popup;
    OSWindow *window;
    OSMenuItem *parent;
    ArrPt(OSMenuItem) *items;
};

/*---------------------------------------------------------------------------*/

OSMenu *osmenu_create(const uint32_t flags)
{
    OSMenu *menu = heap_new0(OSMenu);
    menu->items = arrpt_create(OSMenuItem);
    unref(flags);
    return menu;
}

/*---------------------------------------------------------------------------*/

void osmenu_destroy(OSMenu **menu)
{
    cassert_no_null(menu);
    cassert_no_null(*menu);
    cassert((*menu)->parent == NULL);

    if ((*menu)->hmenu != NULL)
    {
        BOOL ok = FALSE;
        cassert(GetMenuItemCount((*menu)->hmenu) == 0);
        ok = EndMenu();
        cassert(ok != 0);
        ok = DestroyMenu((*menu)->hmenu);
        cassert_unref(ok != 0, ok);
    }

    cassert(arrpt_size((*menu)->items, OSMenuItem) == 0);
    arrpt_destroy(&(*menu)->items, NULL, OSMenuItem);
    heap_delete(menu, OSMenu);
}

/*---------------------------------------------------------------------------*/

void osmenu_insert_item(OSMenu *menu, const uint32_t pos, OSMenuItem *item)
{
    cassert_no_null(menu);
    arrpt_insert(menu->items, pos, item, OSMenuItem);
    _osmenu_hmenu_recompute(menu);
}

/*---------------------------------------------------------------------------*/

void osmenu_delete_item(OSMenu *menu, OSMenuItem *item)
{
    uint32_t pos = UINT32_MAX;
    cassert_no_null(menu);
    _osmenuitem_unset_parent(item, menu);
    pos = arrpt_find(menu->items, item, OSMenuItem);
    arrpt_delete(menu->items, pos, NULL, OSMenuItem);
    _osmenu_hmenu_recompute(menu);
}

/*---------------------------------------------------------------------------*/

static void i_remove_all_items(OSMenu *menu)
{
    cassert_no_null(menu);
    cassert_no_null(menu->hmenu);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_unset_parent(item, menu);
    arrpt_end()

    {
        int i, n = GetMenuItemCount(menu->hmenu);
        for (i = 0; i < n; ++i)
        {
            BOOL ok = RemoveMenu(menu->hmenu, 0, MF_BYPOSITION);
            cassert_unref(ok == TRUE, ok);
        }

        cassert(GetMenuItemCount(menu->hmenu) == 0);
    }
}

/*---------------------------------------------------------------------------*/

static void i_add_all_items(OSMenu *menu)
{
    cassert_no_null(menu);
    cassert_no_null(menu->hmenu);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_append_to_hmenu(item, menu);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void osmenu_launch(OSMenu *menu, OSWindow *window, const real32_t x, const real32_t y)
{
    cassert_no_null(menu);
    cassert(menu->window == NULL);

    /* The hmenu exists as menubar, we have to destroy it */
    if (menu->hmenu != NULL && menu->is_popup == FALSE)
    {
        BOOL ok = FALSE;
        i_remove_all_items(menu);
        ok = DestroyMenu(menu->hmenu);
        cassert_unref(ok != 0, ok);
        menu->hmenu = NULL;
    }

    /* No hmenu created */
    if (menu->hmenu == NULL)
    {
        menu->hmenu = CreatePopupMenu();
        i_add_all_items(menu);
        menu->is_popup = TRUE;
    }

    {
        HWND hwnd = NULL;
        BOOL ok = FALSE;
        UINT flags = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_NOANIMATION;
        hwnd = _oswindow_set_current_popup_menu(window, menu->hmenu);
        ok = TrackPopupMenu(menu->hmenu, flags, (int)x, (int)y, PARAM(nReserved, 0), hwnd, NULL);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

void osmenu_hide(OSMenu *menu)
{
    BOOL ok = FALSE;
    cassert_no_null(menu);
    unref(menu);
    ok = EndMenu();
    cassert_unref(ok != 0, ok);
}

/*---------------------------------------------------------------------------*/

bool_t osmenu_is_menubar(const OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->window != NULL)
    {
        cassert_no_null(menu->hmenu);
        cassert(menu->is_popup == FALSE);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

HMENU _osmenu_hmenu(OSMenu *menu)
{
    cassert_no_null(menu);
    return menu->hmenu;
}

/*---------------------------------------------------------------------------*/

HMENU _osmenu_menubar(OSMenu *menu, OSWindow *window)
{
    cassert_no_null(menu);
    cassert(menu->window == NULL);

    /* The hmenu exists as popup, we have to destroy it */
    if (menu->hmenu != NULL && menu->is_popup == TRUE)
    {
        BOOL ok = FALSE;
        i_remove_all_items(menu);
        ok = DestroyMenu(menu->hmenu);
        cassert_unref(ok != 0, ok);
        menu->hmenu = NULL;
        menu->is_popup = FALSE;
    }

    /* No hmenu created */
    if (menu->hmenu == NULL)
    {
        menu->hmenu = CreateMenu();
        i_add_all_items(menu);
        menu->is_popup = FALSE;
    }

    menu->window = window;
    return menu->hmenu;
}

/*---------------------------------------------------------------------------*/

HMENU _osmenu_menubar_unlink(OSMenu *menu, OSWindow *window)
{
    cassert_no_null(menu);
    cassert_unref(menu->window == window, window);
    cassert(menu->is_popup == FALSE);
    menu->window = NULL;
    return menu->hmenu;
}

/*---------------------------------------------------------------------------*/

void _osmenu_hmenu_recompute(OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->hmenu != NULL)
    {
        i_remove_all_items(menu);
        i_add_all_items(menu);
        if (menu->window != NULL)
        {
            HWND hwnd = cast(menu->window, OSControl)->hwnd;
            BOOL ok = DrawMenuBar(hwnd);
            cassert_unref(ok != 0, ok);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _osmenu_hmenu_redraw(OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->hmenu != NULL && menu->window != NULL)
    {
        HWND hwnd = cast(menu->window, OSControl)->hwnd;
        BOOL ok = DrawMenuBar(hwnd);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

void _osmenu_attach_to_item(OSMenu *menu, OSMenuItem *item)
{
    cassert_no_null(menu);
    cassert(menu->parent == NULL);
    menu->parent = item;
    /* A Submenu is always a PopUp menu */
    if (menu->hmenu == NULL)
    {
        menu->hmenu = CreatePopupMenu();
        menu->is_popup = TRUE;
        i_add_all_items(menu);
    }
}

/*---------------------------------------------------------------------------*/

void _osmenu_detach_from_item(OSMenu *menu, OSMenuItem *item)
{
    cassert_no_null(menu);
    cassert_unref(menu->parent == item, item);
    menu->parent = NULL;
}
