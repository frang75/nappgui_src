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

#include "osgui_gtk.inl"
#include "osmenu_gtk.inl"
#include "osmenuitem_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osgui.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

/*
 *  menubar --> GtkBox (vertical) with one or more GtkMenuBar
 *  menupop --> GtkMenu instance
 *  The same GtkMenuItems will be moved from/to menubar/menupop
*/
struct _osmenu_t
{
    GtkWidget *menubar;
    GtkWidget *menupop;
    bool_t is_popup;
    OSWindow *window;
    OSMenuItem *parent;
    uint32_t bar_width;
    ArrPt(OSMenuItem) *items;

#if GTK_CHECK_VERSION(3, 22, 0)
#else
    gint popup_x;
    gint popup_y;
#endif

#if defined(__ASSERTS__)
    bool_t is_alive;
#endif
};

#if defined(__ASSERTS__)

/*---------------------------------------------------------------------------*/

static void i_count_items(GtkWidget *widget, gpointer n)
{
    if (GTK_IS_MENU_ITEM(widget))
    {
        *cast(n, uint32_t) += 1;
    }
    else
    {
        cassert(GTK_IS_MENU_BAR(widget));
        gtk_container_foreach(GTK_CONTAINER(widget), i_count_items, (gpointer)&n);
    }
}

/*---------------------------------------------------------------------------*/

static uint32_t i_num_items(GtkWidget *widget)
{
    uint32_t n = 0;
    cassert(GTK_IS_MENU(widget) || GTK_IS_BOX(widget));
    gtk_container_foreach(GTK_CONTAINER(widget), i_count_items, (gpointer)&n);
    return n;
}

/*---------------------------------------------------------------------------*/

static void i_OnDestroy(GtkWidget *obj, OSMenu *menu)
{
    cassert(menu->is_alive == TRUE);
    unref(obj);
    menu->is_alive = FALSE;
}

#endif

/*---------------------------------------------------------------------------*/

OSMenu *osmenu_create(const enum_t flags)
{
    OSMenu *menu = heap_new0(OSMenu);
    menu->items = arrpt_create(OSMenuItem);
    menu->bar_width = UINT32_MAX;
    unref(flags);
    return menu;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_widget(OSMenu *menu, GtkWidget *widget)
{
    cassert_no_null(menu);
    cassert_no_null(widget);
    cassert(i_num_items(widget) == 0);
#if defined(__ASSERTS__)
    menu->is_alive = TRUE;
    g_signal_connect(widget, "destroy", G_CALLBACK(i_OnDestroy), (gpointer)menu);
#else
    unref(menu);
#endif
    g_object_unref(widget);
    cassert(menu->is_alive == FALSE);
}

/*---------------------------------------------------------------------------*/

void osmenu_destroy(OSMenu **menu)
{
    cassert_no_null(menu);
    cassert_no_null(*menu);
    cassert((*menu)->parent == NULL);

    if ((*menu)->menubar != NULL)
    {
        i_destroy_widget(*menu, (*menu)->menubar);
        (*menu)->menubar = NULL;
    }

    if ((*menu)->menupop != NULL)
    {
        i_destroy_widget(*menu, (*menu)->menupop);
        (*menu)->menupop = NULL;
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
    _osmenu_widget_recompute(menu);
}

/*---------------------------------------------------------------------------*/

void osmenu_delete_item(OSMenu *menu, OSMenuItem *item)
{
    uint32_t pos;
    cassert_no_null(menu);
    _osmenuitem_unset_parent(item, menu);

    if (menu->window != NULL)
    {
        GtkAccelGroup *accel = _oswindow_accel(menu->window);
        cassert_no_null(accel);
        _osmenuitem_unset_accel(item, accel);
    }

    pos = arrpt_find(menu->items, item, OSMenuItem);
    arrpt_delete(menu->items, pos, NULL, OSMenuItem);
    _osmenu_widget_recompute(menu);
}

/*---------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(3, 22, 0)
#else
static void i_popup_pos(GtkMenu *widget, gint *x, gint *y, gboolean *push_in, gpointer user_data)
{
    OSMenu *menu = cast(user_data, OSMenu);
    cassert_no_null(menu);
    cassert_unref(cast(widget, GtkWidget) == menu->menupop, widget);
    cassert_no_null(x);
    cassert_no_null(y);
    cassert_no_null(push_in);
    *x = menu->popup_x;
    *y = menu->popup_y;
    *push_in = TRUE;
}
#endif

/*---------------------------------------------------------------------------*/

static void i_get_item(GtkWidget *widget, gpointer items)
{
    if (GTK_IS_MENU_ITEM(widget))
    {
        g_array_append_val((GArray *)items, widget);
    }
    else
    {
        cassert(GTK_IS_MENU_BAR(widget));
        gtk_container_foreach(GTK_CONTAINER(widget), i_get_item, items);
    }
}

/*---------------------------------------------------------------------------*/

static void i_get_items(GtkWidget *widget, GArray *items)
{
    cassert(GTK_IS_MENU(widget) || GTK_IS_BOX(widget));
    gtk_container_foreach(GTK_CONTAINER(widget), i_get_item, (gpointer)items);
}

/*---------------------------------------------------------------------------*/

static void i_remove_all_items(OSMenu *menu)
{
    GArray *items = NULL;

    cassert_no_null(menu);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_unset_parent(item, menu);
    arrpt_end()

    if (menu->menupop != NULL)
    {
        cassert(items == NULL);
        items = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));
        i_get_items(menu->menupop, items);
    }

    if (menu->menubar != NULL)
    {
        guint n = 0;
        if (items == NULL)
            items = g_array_new(FALSE, FALSE, sizeof(GtkWidget *));
        else
            n = items->len;

        i_get_items(menu->menubar, items);
        /* Its not possible to have items in both menus */
        cassert_unref(n == 0 || items->len == n, n);
    }

    /* Remove the items from their parent, without destroy the items */
    if (items != NULL)
    {
        guint i = 0;
        for (i = 0; i < items->len; ++i)
        {
            GtkWidget *item = cast(items->data, GtkWidget *)[i];
            GtkWidget *parent = NULL;
            cassert(GTK_IS_MENU_ITEM(item));
            parent = gtk_widget_get_parent(item);
            cassert(parent != NULL && GTK_IS_MENU_SHELL(parent));
            gtk_container_remove(GTK_CONTAINER(parent), item);
            cassert(cast(item, GObject)->ref_count > 0);
        }

        g_array_free(items, TRUE);
        items = NULL;
    }

#if defined(__ASSERTS__)
    if (menu->menubar != NULL)
    {
        cassert(i_num_items(menu->menubar) == 0);
    }

    if (menu->menupop != NULL)
    {
        cassert(i_num_items(menu->menupop) == 0);
    }
#endif
}

/*---------------------------------------------------------------------------*/

static void i_add_all_items_to_popup(OSMenu *menu)
{
    cassert_no_null(menu);
    cassert(menu->menupop == NULL || i_num_items(menu->menupop) == 0);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_append_to_menu(item, menu, menu->menupop);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

static void i_add_all_items_to_menubar(OSMenu *menu)
{
    cassert_no_null(menu);
    cassert(menu->menubar == NULL || i_num_items(menu->menubar) == 0);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_append_to_menubar(item, menu, menu->menubar, menu->bar_width);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void osmenu_launch(OSMenu *menu, OSWindow *window, const real32_t x, const real32_t y)
{
    cassert_no_null(menu);
    cassert(menu->window == NULL);
    unref(window);

    if (menu->is_popup == FALSE || menu->menupop == NULL)
    {
        if (menu->menupop == NULL)
        {
            menu->menupop = gtk_menu_new();
            g_object_ref_sink(menu->menupop);
        }

        if (menu->is_popup == FALSE)
        {
            i_remove_all_items(menu);
            i_add_all_items_to_popup(menu);
            menu->is_popup = TRUE;
        }
    }

    gtk_widget_show_all(menu->menupop);

#if GTK_CHECK_VERSION(3, 22, 0)
    {
        GdkDisplay *display = gdk_display_get_default();
        GdkWindow *gdkwindow = gdk_display_get_default_group(display);
        GdkRectangle rect;
        rect.x = (int)x;
        rect.y = (int)y;
        rect.width = 100;
        rect.height = 100;
        gtk_menu_popup_at_rect(GTK_MENU(menu->menupop), gdkwindow, &rect, GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST, NULL);
    }
#else
    menu->popup_x = (gint)x;
    menu->popup_y = (gint)y;
    gtk_menu_popup(GTK_MENU(menu->menupop), NULL, NULL, i_popup_pos, menu, 0, 0);
#endif
}

/*---------------------------------------------------------------------------*/

void osmenu_hide(OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->menupop != NULL)
        gtk_widget_hide(menu->menupop);
}

/*---------------------------------------------------------------------------*/

bool_t osmenu_is_menubar(const OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->window != NULL)
    {
        cassert_no_null(menu->menubar);
        cassert(menu->is_popup == FALSE);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osmenu_menubar(OSMenu *menu, OSWindow *window, const uint32_t max_width)
{
    cassert_no_null(menu);
    cassert(max_width > 0 && max_width != UINT32_MAX);
    cassert(menu->window == NULL || menu->window == window);
    menu->window = window;

    /* The menubar is updated to incomming max_width */
    if (menu->menubar != NULL && menu->is_popup == FALSE && menu->bar_width == max_width)
    {
        return menu->menubar;
    }
    /* The menubar has to be recomputed */
    else
    {
        i_remove_all_items(menu);

        /* No menubar created */
        if (menu->menubar == NULL)
        {
            menu->menubar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
            g_object_ref_sink(menu->menubar);
        }

        menu->is_popup = FALSE;
        menu->bar_width = max_width;
        i_add_all_items_to_menubar(menu);
    }

    return menu->menubar;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osmenu_menubar_unlink(OSMenu *menu, OSWindow *window)
{
    cassert_no_null(menu);
    cassert_unref(menu->window == window, window);
    cassert(menu->is_popup == FALSE);
    cassert(menu->menubar != NULL);
    menu->window = NULL;
    return menu->menubar;
}

/*---------------------------------------------------------------------------*/

uint32_t _osmenu_menubar_height(const OSMenu *menu)
{
    cassert_no_null(menu);
    if (menu->menubar != NULL)
    {
        GtkRequisition bsize;
        gtk_widget_get_preferred_size(menu->menubar, &bsize, NULL);
        return (uint32_t)bsize.height;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*/

void _osmenu_widget_recompute(OSMenu *menu)
{
    cassert_no_null(menu);
    i_remove_all_items(menu);
    if (menu->is_popup == TRUE && menu->menupop != NULL)
        i_add_all_items_to_popup(menu);
    else if (menu->is_popup == FALSE && menu->menubar != NULL)
        i_add_all_items_to_menubar(menu);
}

/*---------------------------------------------------------------------------*/

void _osmenu_attach_to_item(OSMenu *menu, OSMenuItem *item)
{
    GtkWidget *widget = NULL;
    cassert_no_null(menu);
    cassert(menu->parent == NULL);
    cassert(menu->bar_width == UINT32_MAX);
    menu->parent = item;

    /* A submenu is always a popup menu */
    if (menu->menupop == NULL)
    {
        menu->menupop = gtk_menu_new();
        g_object_ref_sink(menu->menupop);
        i_add_all_items_to_popup(menu);
    }

    widget = _osmenuitem_widget(item);
    cassert(GTK_IS_MENU_ITEM(widget));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), menu->menupop);
}

/*---------------------------------------------------------------------------*/

void _osmenu_detach_from_item(OSMenu *menu, OSMenuItem *item)
{
    GtkWidget *widget = NULL;
    cassert_no_null(menu);
    cassert_unref(menu->parent == item, item);
    cassert(menu->bar_width == UINT32_MAX);
    menu->parent = NULL;
    widget = _osmenuitem_widget(item);
    cassert(GTK_IS_MENU_ITEM(widget));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget), NULL);
}

/*---------------------------------------------------------------------------*/

void _osmenu_set_accel(OSMenu *menu, GtkAccelGroup *accel)
{
    cassert_no_null(menu);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_set_accel(item, accel);
    arrpt_end()
}

/*---------------------------------------------------------------------------*/

void _osmenu_unset_accel(OSMenu *menu, GtkAccelGroup *accel)
{
    cassert_no_null(menu);
    arrpt_foreach(item, menu->items, OSMenuItem)
        _osmenuitem_unset_accel(item, accel);
    arrpt_end()
}
