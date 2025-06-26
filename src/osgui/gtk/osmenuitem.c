/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmenuitem.c
 *
 */

/* Operating System native menu item */

#include "osmenu_gtk.inl"
#include "osmenuitem_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osgui_gtk.inl"
#include "../osmenuitem.h"
#include "../osgui.inl"
#include <draw2d/image.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

static const guint i_VIRTUAL_KEY[] =
    {
        UINT32_MAX, /*ekKEY_UNDEF           = 0*/
        GDK_KEY_a,  /*ekKEY_A               = 1*/
        GDK_KEY_s,  /*ekKEY_S               = 2*/
        GDK_KEY_d,  /*ekKEY_D               = 3*/
        GDK_KEY_f,  /*ekKEY_F               = 4*/
        GDK_KEY_h,  /*ekKEY_H               = 5*/
        GDK_KEY_g,  /*ekKEY_G               = 6*/
        GDK_KEY_z,  /*ekKEY_Z               = 7*/
        GDK_KEY_x,  /*ekKEY_X               = 8*/
        GDK_KEY_c,  /*ekKEY_C               = 9*/

        GDK_KEY_v,         /*ekKEY_V               = 10*/
        GDK_KEY_backslash, /*ekKEY_BACKSLASH       = 11*/
        GDK_KEY_b,         /*ekKEY_B               = 12*/
        GDK_KEY_q,         /*ekKEY_Q               = 13*/
        GDK_KEY_w,         /*ekKEY_W               = 14*/
        GDK_KEY_e,         /*ekKEY_E               = 15*/
        GDK_KEY_r,         /*ekKEY_R               = 16*/
        GDK_KEY_y,         /*ekKEY_Y               = 17*/
        GDK_KEY_t,         /*ekKEY_T               = 18*/
        GDK_KEY_1,         /*ekKEY_1               = 19*/

        GDK_KEY_2,            /*ekKEY_2               = 20*/
        GDK_KEY_3,            /*ekKEY_3               = 21*/
        GDK_KEY_4,            /*ekKEY_4               = 22*/
        GDK_KEY_6,            /*ekKEY_6               = 23*/
        GDK_KEY_5,            /*ekKEY_5               = 24*/
        GDK_KEY_9,            /*ekKEY_9               = 25*/
        GDK_KEY_7,            /*ekKEY_7               = 26*/
        GDK_KEY_8,            /*ekKEY_8               = 27*/
        GDK_KEY_0,            /*ekKEY_0               = 28*/
        GDK_KEY_bracketright, /*ekKEY_RBRACKET        = 29*/

        GDK_KEY_o,           /*ekKEY_O               = 30*/
        GDK_KEY_u,           /*ekKEY_U               = 31*/
        GDK_KEY_bracketleft, /*ekKEY_LBRACKET        = 32*/
        GDK_KEY_i,           /*ekKEY_I               = 33*/
        GDK_KEY_p,           /*ekKEY_P               = 34*/
        GDK_KEY_Return,      /*ekKEY_RETURN          = 35*/
        GDK_KEY_l,           /*ekKEY_L               = 36*/
        GDK_KEY_j,           /*ekKEY_J               = 37*/
        GDK_KEY_semicolon,   /*ekKEY_SEMICOLON       = 38*/
        GDK_KEY_k,           /*ekKEY_K               = 39*/

        GDK_KEY_apostrophe, /*ekKEY_APOSTROPHE      = 40*/
        GDK_KEY_comma,      /*ekKEY_COMMA           = 41*/
        GDK_KEY_minus,      /*ekKEY_MINUS           = 42*/
        GDK_KEY_n,          /*ekKEY_N               = 43*/
        GDK_KEY_m,          /*ekKEY_M               = 44*/
        GDK_KEY_period,     /*ekKEY_PERIOD          = 45*/
        GDK_KEY_Tab,        /*ekKEY_TAB             = 46*/
        GDK_KEY_space,      /*ekKEY_SPACE           = 47*/
        GDK_KEY_less,       /*ekKEY_MAJOR_MINOR     = 48*/
        GDK_KEY_BackSpace,  /*ekKEY_BACK            = 49*/

        GDK_KEY_Escape,      /*ekKEY_ESCAPE          = 50*/
        GDK_KEY_F17,         /*ekKEY_F17             = 51*/
        GDK_KEY_KP_Decimal,  /*ekKEY_NUMPAD_DECIMAL  = 52*/
        GDK_KEY_KP_Multiply, /*ekKEY_NUMPAD_MULT     = 53*/
        GDK_KEY_KP_Add,      /*ekKEY_NUMPAD_ADD      = 54*/
        GDK_KEY_Num_Lock,    /*ekKEY_NUMLOCK         = 55*/
        GDK_KEY_KP_Divide,   /*ekKEY_NUMPAD_DIV      = 56*/
        GDK_KEY_KP_Enter,    /*ekKEY_NUMPAD_RETURN   = 57*/
        GDK_KEY_KP_Subtract, /*ekKEY_NUMPAD_SUBTRACT = 58*/
        GDK_KEY_F18,         /*ekKEY_F18             = 59*/

        GDK_KEY_F19,      /*ekKEY_F19             = 60*/
        GDK_KEY_KP_Equal, /*ekKEY_NUMPAD_EQUAL    = 61*/
        GDK_KEY_KP_0,     /*ekKEY_NUMPAD0         = 62*/
        GDK_KEY_KP_1,     /*ekKEY_NUMPAD1         = 63*/
        GDK_KEY_KP_2,     /*ekKEY_NUMPAD2         = 64*/
        GDK_KEY_KP_3,     /*ekKEY_NUMPAD3         = 65*/
        GDK_KEY_KP_4,     /*ekKEY_NUMPAD4         = 66*/
        GDK_KEY_KP_5,     /*ekKEY_NUMPAD5         = 67*/
        GDK_KEY_KP_6,     /*ekKEY_NUMPAD6         = 68*/
        GDK_KEY_KP_7,     /*ekKEY_NUMPAD7         = 69*/

        GDK_KEY_KP_8, /*ekKEY_NUMPAD8         = 70*/
        GDK_KEY_KP_9, /*ekKEY_NUMPAD9         = 71*/
        GDK_KEY_F5,   /*ekKEY_F5              = 72*/
        GDK_KEY_F6,   /*ekKEY_F6              = 73*/
        GDK_KEY_F7,   /*ekKEY_F7              = 74*/
        GDK_KEY_F3,   /*ekKEY_F3              = 75*/
        GDK_KEY_F8,   /*ekKEY_F8              = 76*/
        GDK_KEY_F9,   /*ekKEY_F9              = 77*/
        GDK_KEY_F11,  /*ekKEY_F11             = 78*/
        GDK_KEY_F13,  /*ekKEY_F13             = 79*/

        GDK_KEY_F16,       /*ekKEY_F16             = 80*/
        GDK_KEY_F14,       /*ekKEY_F14             = 81*/
        GDK_KEY_F10,       /*ekKEY_F10             = 82*/
        GDK_KEY_F12,       /*ekKEY_F12             = 83*/
        GDK_KEY_F15,       /*ekKEY_F15             = 84*/
        GDK_KEY_Page_Up,   /*ekKEY_PAGE_UP         = 85*/
        GDK_KEY_Begin,     /*ekKEY_PAGE_BEGIN      = 86*/
        GDK_KEY_KP_Delete, /*ekKEY_SUPR            = 87*/
        GDK_KEY_F4,        /*ekKEY_F4              = 88*/
        GDK_KEY_Page_Down, /*ekKEY_PAGE_DOWN       = 89*/

        GDK_KEY_F2,    /*ekKEY_F2              = 90*/
        GDK_KEY_End,   /*ekKEY_PAGE_END        = 91*/
        GDK_KEY_F1,    /*ekKEY_F1              = 92*/
        GDK_KEY_Left,  /*ekKEY_LEFT            = 93*/
        GDK_KEY_Right, /*ekKEY_RIGHT           = 94*/
        GDK_KEY_Down,  /*ekKEY_DOWN            = 95*/
        GDK_KEY_Up     /*ekKEY_UP              = 96*/
};

struct _osmenuitem_t
{
    GtkWidget *widget;
    GtkWidget *check;
    GtkWidget *icon;
    GtkWidget *label;
    vkey_t key;
    uint32_t modifiers;
    GtkAccelGroup *accel;
    OSMenu *menu;
    OSMenu *sub_menu;
    Listener *OnClick;
    bool_t visible;
    bool_t show_icon;
    bool_t show_check;
    bool_t launch_event;
#if defined(__ASSERTS__)
    bool_t is_alive;
#endif
};

#if defined(__ASSERTS__)

/*---------------------------------------------------------------------------*/

static void i_OnDestroy(GtkWidget *widget, OSMenuItem *item)
{
    cassert_no_null(item);
    cassert(item->is_alive == TRUE);
    unref(widget);
    item->is_alive = FALSE;
}

#endif

/*---------------------------------------------------------------------------*/

static void i_OnClick(GtkMenuItem *widget, OSMenuItem *item)
{
    cassert_no_null(item);
    unref(widget);
    if (item->OnClick != NULL && item->launch_event == TRUE)
    {
        EvMenu params;
        params.index = UINT32_MAX;
        params.state = ekGUI_ON;
        params.text = NULL;
        listener_event(item->OnClick, ekGUI_EVENT_MENU, item, &params, NULL, OSMenuItem, EvMenu, void);
    }
}

/*---------------------------------------------------------------------------*/

OSMenuItem *osmenuitem_create(const uint32_t flags)
{
    OSMenuItem *item = heap_new0(OSMenuItem);
    if ((menu_flag_t)flags == ekMENU_ITEM)
    {
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
        item->widget = gtk_menu_item_new();
        item->label = gtk_accel_label_new("");
        item->check = gtk_label_new("✓");
        item->icon = gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_MENU);
        item->key = ENUM_MAX(vkey_t);
        item->visible = TRUE;
        item->launch_event = TRUE;
        g_object_ref_sink(item->check);
        g_object_ref_sink(item->icon);
        /*_oscontrol_widget_set_css(item->box, "box {padding-top:1px;padding-bottom:1px;}");*/
        gtk_label_set_use_underline(GTK_LABEL(item->label), TRUE);
        gtk_accel_label_set_accel_widget(GTK_ACCEL_LABEL(item->label), item->widget);
#if GTK_CHECK_VERSION(3, 16, 0)
        gtk_label_set_xalign(GTK_LABEL(item->label), 0.0);
#endif
        gtk_box_pack_end(GTK_BOX(box), item->label, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(item->widget), box);
        g_signal_connect(G_OBJECT(item->widget), "activate", G_CALLBACK(i_OnClick), (gpointer)item);
    }
    else
    {
        cassert((menu_flag_t)flags == ekMENU_SEPARATOR);
        item->widget = gtk_separator_menu_item_new();
        item->key = ENUM_MAX(vkey_t);
        item->visible = TRUE;
    }

    /*
     * Increase the reference count of object, and remove the floating reference.
     * In other words, if the object is floating, then this call "assumes ownership" of the floating reference.
     * Will be explicitly destroyed in 'osmenuitem_destroy'
    */
    g_object_ref_sink(item->widget);

    return item;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_widget(OSMenuItem *item, GtkWidget *widget)
{
    cassert_no_null(item);
    cassert_no_null(widget);
#if defined(__ASSERTS__)
    item->is_alive = TRUE;
    g_signal_connect(widget, "destroy", G_CALLBACK(i_OnDestroy), (gpointer)item);
#else
    unref(item);
#endif
    g_object_unref(widget);
    cassert(item->is_alive == FALSE);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_destroy(OSMenuItem **item)
{
    GtkWidget *box = NULL;
    cassert_no_null(item);
    cassert_no_null(*item);
    cassert((*item)->menu == NULL);
    cassert((*item)->sub_menu == NULL);
    cassert(gtk_menu_item_get_submenu(GTK_MENU_ITEM((*item)->widget)) == NULL);
    cassert((*item)->accel == NULL);
    listener_destroy(&(*item)->OnClick);

    box = gtk_bin_get_child(GTK_BIN((*item)->widget));
    if (box != NULL)
    {
        if (_oscontrol_find_child(box, (*item)->check) != UINT32_MAX)
            gtk_container_remove(GTK_CONTAINER(box), (*item)->check);

        if (_oscontrol_find_child(box, (*item)->icon) != UINT32_MAX)
            gtk_container_remove(GTK_CONTAINER(box), (*item)->icon);

        i_destroy_widget(*item, (*item)->check);
        i_destroy_widget(*item, (*item)->icon);
    }
    else
    {
        cassert((*item)->check == NULL);
        cassert((*item)->icon == NULL);
    }

    i_destroy_widget(*item, (*item)->widget);
    heap_delete(item, OSMenuItem);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_OnClick(OSMenuItem *item, Listener *listener)
{
    cassert_no_null(item);
    cassert(item->label != NULL);
    listener_update(&item->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_enabled(OSMenuItem *item, const bool_t enabled)
{
    cassert_no_null(item);
    gtk_widget_set_sensitive(item->widget, (gboolean)enabled);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_visible(OSMenuItem *item, const bool_t visible)
{
    cassert_no_null(item);
    if (item->visible != visible)
    {
        item->visible = visible;
        if (item->menu != NULL)
            _osmenu_widget_recompute(item->menu);
    }
}

/*---------------------------------------------------------------------------*/

void osmenuitem_text(OSMenuItem *item, const char_t *text)
{
    cassert_no_null(item);
    gtk_label_set_text(GTK_LABEL(item->label), cast_const(text, gchar));
    if (item->menu != NULL)
        _osmenu_widget_recompute(item->menu);
}

/*---------------------------------------------------------------------------*/

static void i_update_icons(OSMenuItem *item)
{
    GtkWidget *box = NULL;
    cassert_no_null(item);
    box = gtk_bin_get_child(GTK_BIN(item->widget));
    if (_oscontrol_find_child(box, item->icon) != UINT32_MAX)
    {
        gtk_container_remove(GTK_CONTAINER(box), item->icon);
        cassert(_oscontrol_find_child(box, item->icon) == UINT32_MAX);
    }

    if (_oscontrol_find_child(box, item->check) != UINT32_MAX)
    {
        gtk_container_remove(GTK_CONTAINER(box), item->check);
        cassert(_oscontrol_find_child(box, item->check) == UINT32_MAX);
    }

    if (item->show_icon == TRUE)
        gtk_box_pack_start(GTK_BOX(box), item->icon, FALSE, FALSE, 0);

    if (item->show_check == TRUE)
        gtk_box_pack_start(GTK_BOX(box), item->check, FALSE, FALSE, 0);

    if (item->menu != NULL)
        _osmenu_widget_recompute(item->menu);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_image(OSMenuItem *item, const Image *image)
{
    cassert_no_null(item);
    if (image != NULL)
    {
        const char_t *icon_name = _osgui_register_icon(image);
        gtk_image_set_from_icon_name(GTK_IMAGE(item->icon), icon_name, GTK_ICON_SIZE_MENU);
        if (item->show_icon == FALSE)
        {
            item->show_icon = TRUE;
            i_update_icons(item);
        }
    }
    else
    {
        if (item->show_icon == TRUE)
        {
            item->show_icon = FALSE;
            i_update_icons(item);
        }
    }
}

/*---------------------------------------------------------------------------*/

static ___INLINE GdkModifierType i_kmod(const uint32_t modifiers)
{
    GdkModifierType mod = 0;
    if (modifiers & ekMKEY_CONTROL)
        mod |= GDK_CONTROL_MASK;
    if (modifiers & ekMKEY_ALT)
        mod |= GDK_MOD1_MASK;
    if (modifiers & ekMKEY_SHIFT)
        mod |= GDK_SHIFT_MASK;
    return mod;
}

/*---------------------------------------------------------------------------*/

void osmenuitem_key(OSMenuItem *item, const vkey_t key, const uint32_t modifiers)
{
    cassert_no_null(item);
    if (item->accel != NULL)
    {
        if (item->key != ENUM_MAX(vkey_t))
        {
            gboolean ok = gtk_widget_remove_accelerator(item->widget, item->accel, i_VIRTUAL_KEY[item->key], i_kmod(item->modifiers));
            cassert_unref(ok == TRUE, ok);
        }

        if (key != ENUM_MAX(vkey_t))
            gtk_widget_add_accelerator(item->widget, "activate", item->accel, i_VIRTUAL_KEY[key], i_kmod(modifiers), GTK_ACCEL_VISIBLE);
    }

    item->key = key;
    item->modifiers = modifiers;
}

/*---------------------------------------------------------------------------*/

void osmenuitem_state(OSMenuItem *item, const gui_state_t state)
{
    cassert_no_null(item);
    switch (state)
    {
    case ekGUI_OFF:
        if (item->show_check == TRUE)
        {
            item->show_check = FALSE;
            i_update_icons(item);
        }
        break;

    case ekGUI_ON:
    case ekGUI_MIXED:
        if (item->show_check == FALSE)
        {
            item->show_check = TRUE;
            i_update_icons(item);
        }
        break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osmenuitem_submenu(OSMenuItem *item, OSMenu *menu)
{
    cassert_no_null(item);
    cassert(item->sub_menu == NULL);
    item->sub_menu = menu;
    _osmenu_attach_to_item(menu, item);
}

/*---------------------------------------------------------------------------*/

void osmenuitem_unset_submenu(OSMenuItem *item, OSMenu *menu)
{
    cassert_no_null(item);
    cassert_unref(item->sub_menu == menu, menu);
    _osmenu_detach_from_item(item->sub_menu, item);
    item->sub_menu = NULL;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osmenuitem_widget(OSMenuItem *item)
{
    cassert_no_null(item);
    return item->widget;
}

/*---------------------------------------------------------------------------*/

void _osmenuitem_append_to_menu(OSMenuItem *item, OSMenu *menu, GtkWidget *widget)
{
    cassert_no_null(item);
    cassert(item->menu == NULL);
    item->menu = menu;
    if (item->visible == TRUE && widget != NULL)
    {
        cassert(GTK_IS_MENU(widget));
        gtk_menu_shell_append(GTK_MENU_SHELL(widget), item->widget);
    }
}

/*---------------------------------------------------------------------------*/

static void i_current_menubar(GtkWidget *widget, gpointer current)
{
    cassert(GTK_IS_MENU_BAR(widget));
    if (_oscontrol_num_children(widget) == 0)
    {
        /* If the first menubar is empty, is the current */
        if (*dcast(current, GtkWidget) == NULL)
            *dcast(current, GtkWidget) = widget;
    }
    else
    {
        /* The last bar with widgets is the current */
        *dcast(current, GtkWidget) = widget;
    }
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_get_current_menubar(GtkWidget *widget)
{
    GtkWidget *last = NULL;
    cassert(GTK_IS_BOX(widget));
    gtk_container_foreach(GTK_CONTAINER(widget), i_current_menubar, (gpointer)&last);
    return last;
}

/*---------------------------------------------------------------------------*/

static void i_next_menubar(GtkWidget *widget, gpointer data)
{
    cassert(GTK_IS_MENU_BAR(widget));
    if (dcast(data, GtkWidget)[0] == widget)
    {
        cassert(dcast(data, GtkWidget)[1] == NULL);
        cassert(dcast(data, GtkWidget)[2] == NULL);
        dcast(data, GtkWidget)[2] = widget;
    }
    /* This means, we are in the next widget */
    else if (dcast(data, GtkWidget)[2] != NULL)
    {
        cassert(dcast(data, GtkWidget)[1] == NULL);
        dcast(data, GtkWidget)[1] = widget;
        dcast(data, GtkWidget)[2] = NULL;
    }
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_get_next_menubar(GtkWidget *widget, GtkWidget *current)
{
    GtkWidget *data[3];
    data[0] = current;
    data[1] = NULL; /* Next */
    data[2] = NULL; /* Flag */
    cassert(GTK_IS_BOX(widget));
    gtk_container_foreach(GTK_CONTAINER(widget), i_next_menubar, (gpointer)data);
    if (data[1] != NULL)
    {
        cassert(_oscontrol_num_children(data[1]) == 0);
    }

    return data[1];
}

/*---------------------------------------------------------------------------*/

static void i_add_item_to_new_bar(OSMenuItem *item, GtkWidget *widget)
{
    GtkWidget *menubar = gtk_menu_bar_new();
    cassert_no_null(item);
    cassert_no_null(item->widget);
    cassert(GTK_IS_BOX(widget));
    gtk_box_pack_start(GTK_BOX(widget), menubar, FALSE, FALSE, 0);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item->widget);
}

/*---------------------------------------------------------------------------*/

void _osmenuitem_append_to_menubar(OSMenuItem *item, OSMenu *menu, GtkWidget *widget, const uint32_t max_width)
{
    cassert_no_null(item);
    cassert_no_null(item->widget);
    cassert(item->menu == NULL);
    item->menu = menu;
    if (item->visible == TRUE && widget != NULL)
    {
        GtkWidget *current = NULL;
        cassert(GTK_IS_BOX(widget));
        current = i_get_current_menubar(widget);
        if (current != NULL)
        {
            GtkRequisition bsize;
            cassert(GTK_IS_MENU_BAR(current));
            gtk_menu_shell_append(GTK_MENU_SHELL(current), item->widget);
            gtk_widget_show_all(widget);
            gtk_widget_get_preferred_size(widget, &bsize, NULL);
            /* bsize == 0 --> The menubar isn't in window, is in a top screen bar */
            cassert((bsize.width > 0 && bsize.height > 0) || (bsize.width == 0 && bsize.height == 0));

            /* Remove the last item and create a new menubar */
            if ((uint32_t)bsize.width > max_width)
            {
                GtkWidget *next = NULL;
                gtk_container_remove(GTK_CONTAINER(current), item->widget);
#if defined(__ASSERTS__)
                {
                    GtkRequisition nbsize;
                    gtk_widget_get_preferred_size(current, &nbsize, NULL);
                    cassert((uint32_t)nbsize.width <= max_width);
                }
#endif

                next = i_get_next_menubar(widget, current);
                if (next != NULL)
                {
                    gtk_menu_shell_append(GTK_MENU_SHELL(next), item->widget);
                    gtk_widget_show_all(widget);
                }
                else
                {
                    i_add_item_to_new_bar(item, widget);
                }
            }
        }
        else
        {
            i_add_item_to_new_bar(item, widget);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _osmenuitem_unset_parent(OSMenuItem *item, OSMenu *menu)
{
    cassert_no_null(item);
    if (item->menu != NULL)
    {
        cassert_unref(item->menu == menu, menu);
        item->menu = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void _osmenuitem_set_accel(OSMenuItem *item, GtkAccelGroup *accel)
{
    cassert_no_null(item);
    cassert_no_null(accel);
    cassert(item->accel == NULL);
    item->accel = accel;
    if (item->key != ENUM_MAX(vkey_t))
        gtk_widget_add_accelerator(item->widget, "activate", item->accel, i_VIRTUAL_KEY[item->key], i_kmod(item->modifiers), GTK_ACCEL_VISIBLE);

    if (item->sub_menu != NULL)
        _osmenu_set_accel(item->sub_menu, accel);
}

/*---------------------------------------------------------------------------*/

void _osmenuitem_unset_accel(OSMenuItem *item, GtkAccelGroup *accel)
{
    cassert_no_null(item);
    if (item->accel != NULL)
    {
        cassert(item->accel == accel);
        if (item->key != ENUM_MAX(vkey_t))
        {
            gboolean ok = gtk_widget_remove_accelerator(item->widget, item->accel, i_VIRTUAL_KEY[item->key], i_kmod(item->modifiers));
            cassert_unref(ok == TRUE, ok);
        }

        item->accel = NULL;
    }

    if (item->sub_menu != NULL)
        _osmenu_unset_accel(item->sub_menu, accel);
}
