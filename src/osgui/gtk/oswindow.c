/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.c
 *
 */

/* Operating System native window */

#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osbutton_gtk.inl"
#include "oscombo_gtk.inl"
#include "osedit_gtk.inl"
#include "osglobals_gtk.inl"
#include "osmenu_gtk.inl"
#include "ospanel_gtk.inl"
#include "oswindow_gtk.inl"
#include "../oswindow.h"
#include "../oswindow.inl"
#include "../oscontrol.inl"
#include "../ostabstop.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/event.h>
#include <core/heap.h>
#include <osbs/bthread.h>
#include <sewer/cassert.h>

#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/gdkwayland.h>
#endif

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _oswindow_t
{
    OSControl control;
    OSMenu *menu;
    GMainLoop *runloop;
    uint32_t modal_return;
    uint32_t flags;
    gui_role_t role;
    GtkAccelGroup *accel;
    OSPanel *main_panel;
    gulong signal_delete;
    gulong signal_config;
    gulong signal_keypre;
    gulong signal_keyrel;
    gulong signal_state;
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
    OSTabStop tabstop;
    ArrSt(OSHotKey) *hotkeys;
    bool_t destroy_main_view;
    bool_t is_resizable;
    gint configure_event;
    gint current_x;
    gint current_y;
    gint current_width;
    gint current_height;
    gint minimun_width;
    gint minimun_height;
    GtkWidget *content_box;
    GtkWidget *popover;
};

/*---------------------------------------------------------------------------*/

static GtkApplication *i_GTK_APP = NULL;
static GdkPixbuf *i_APP_ICON = NULL;
static bool_t i_APP_TERMINATE = FALSE;
static bool_t i_close(OSWindow *, const gui_close_t);
static gboolean i_OnKeyPress(GtkWidget *, GdkEventKey *, OSWindow *);

/*---------------------------------------------------------------------------*/

/* Wayland support for overlay windows, via GtkPopover */
#if GTK_CHECK_VERSION(3, 12, 0)

static void i_apply_flat_popover_css(GtkWidget *popover)
{
    static const gchar *css = "popover.background { border-radius: 0 }";
    GtkCssProvider *provider = gtk_css_provider_new();
    GtkStyleContext *context = gtk_widget_get_style_context(popover);
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

/*---------------------------------------------------------------------------*/

static void i_on_popover_closed(GtkWidget *popover, OSWindow *window)
{
    unref(popover);
    if (window->role == ekGUI_ROLE_OVERLAY)
    {
        if (i_close(window, ekGUI_CLOSE_DEACT) == TRUE)
            window->role = ENUM_MAX(gui_role_t);
    }
}

/*---------------------------------------------------------------------------*/

static void i_show_overlay(OSWindow *window, OSWindow *parent_window, const gint origin_x, const gint origin_y, const align_t halign, const align_t valign)
{
    GtkRequisition natural;
    GdkRectangle rect;
    GtkPositionType position;

    if (window->popover == NULL)
    {
        window->popover = gtk_popover_new(parent_window->content_box);
        g_object_set_data(G_OBJECT(window->popover), "nappgui-overlay-window", window);
        i_apply_flat_popover_css(window->popover);
#if GTK_CHECK_VERSION(3, 20, 0)
        gtk_popover_set_constrain_to(GTK_POPOVER(window->popover), GTK_POPOVER_CONSTRAINT_NONE);
#endif

        g_object_ref(window->content_box);
        gtk_container_remove(GTK_CONTAINER(window->control.widget), window->content_box);
        gtk_container_add(GTK_CONTAINER(window->popover), window->content_box);
        g_object_unref(window->content_box);

        g_signal_connect(window->popover, "closed", G_CALLBACK(i_on_popover_closed), (gpointer)window);
        g_signal_connect(window->popover, "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)window);
    }
    else
    {
        gtk_popover_set_relative_to(GTK_POPOVER(window->popover), parent_window->content_box);
    }

    gtk_widget_show(window->content_box);
    gtk_widget_show(window->popover);

    gtk_widget_get_preferred_size(window->popover, NULL, &natural);

    if (halign != ekCENTER)
    {
        position = (halign == ekLEFT) ? GTK_POS_RIGHT : GTK_POS_LEFT;
        rect.x = origin_x;
        rect.y = origin_y;
    }
    else
    {
        if (valign == ekTOP)
        {
            position = GTK_POS_BOTTOM;
            rect.y = origin_y;
        }
        else if (valign == ekBOTTOM)
        {
            position = GTK_POS_TOP;
            rect.y = origin_y;
        }
        else
        {
            position = GTK_POS_BOTTOM;
            rect.y = origin_y - natural.height / 2;
        }

        rect.x = origin_x;
    }

    rect.width = 0;
    rect.height = 0;
    gtk_popover_set_position(GTK_POPOVER(window->popover), position);
    gtk_popover_set_pointing_to(GTK_POPOVER(window->popover), &rect);
}

/*---------------------------------------------------------------------------*/

static void i_unembed_overlay(OSWindow *window)
{
    if (window->popover != NULL)
    {
        GtkWidget *popover = window->popover;
        window->popover = NULL;
        g_signal_handlers_disconnect_by_func(popover, (gpointer)(intptr_t)i_on_popover_closed, window);
        g_signal_handlers_disconnect_by_func(popover, (gpointer)(intptr_t)i_OnKeyPress, window);
        g_object_ref(window->content_box);
        gtk_container_remove(GTK_CONTAINER(popover), window->content_box);
        gtk_container_add(GTK_CONTAINER(window->control.widget), window->content_box);
        g_object_unref(window->content_box);
        gtk_widget_destroy(popover);
    }
}

/*---------------------------------------------------------------------------*/

#endif /* GTK_CHECK_VERSION(3, 12, 0) */

/*---------------------------------------------------------------------------*/

static bool_t i_close(OSWindow *window, const gui_close_t close_origin)
{
    bool_t closed = TRUE;
    cassert_no_null(window);

    /* Checks if the current control allows the window to be closed */
    if (close_origin == ekGUI_CLOSE_INTRO)
        closed = _ostabstop_can_close_window(&window->tabstop);

    /* Notify the user and check if allows the window to be closed */
    if (closed == TRUE && window->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = close_origin;
        listener_event(window->OnClose, ekGUI_EVENT_WND_CLOSE, window, &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    if (closed == TRUE)
    {
        if (window->popover != NULL)
        {
#if GTK_CHECK_VERSION(3, 12, 0)
            gtk_widget_hide(window->content_box);
            g_signal_handlers_block_by_func(window->popover, (gpointer)(intptr_t)i_on_popover_closed, window);
            gtk_widget_hide(window->popover);
            g_signal_handlers_unblock_by_func(window->popover, (gpointer)(intptr_t)i_on_popover_closed, window);
#endif
        }
        else
        {
            gtk_widget_hide(window->control.widget);
        }
    }

    return closed;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnClose(GtkWidget *widget, GdkEvent *event, OSWindow *window)
{
    cassert_no_null(window);
    cassert_unref(window->control.widget == widget, widget);
    unref(event);
    i_close(window, ekGUI_CLOSE_BUTTON);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_menubar_required_width(const OSWindow *window)
{
    cassert_no_null(window);
    /*
     * This small margin creates a menu bar slightly narrower than the width of
     * the window, allowing for shrinking the size during resizing operations.
     */
    if (window->is_resizable == TRUE)
        return (uint32_t)(window->current_width - 5);
    else
        return (uint32_t)window->current_width;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_menubar_current_height(const OSWindow *window)
{
    cassert_no_null(window);
    if (window->menu != NULL)
        return _osmenu_menubar_height(window->menu);
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnConfigure(GtkWidget *widget, GdkEventConfigure *event, OSWindow *window)
{
    if (i_APP_TERMINATE == TRUE)
        return FALSE;

    cassert_no_null(window);
    cassert_unref(window->control.widget == widget, widget);
    cassert_no_null(event);

    /*
     * Avoid to launch events from explict GTK move/resize functions.
     * We want launch event ONLY in user actions: Move title bar, drag borders, etc
     */
    if (window->configure_event == 0)
    {
        if (window->current_x != event->x || window->current_y != event->y)
        {
            /* When window is moved dragging the titlebar */
            if (window->OnMoved != NULL)
            {
                gint x, y;
                EvPos p;
                gtk_window_get_position(GTK_WINDOW(window->control.widget), &x, &y);
                p.x = (real32_t)x;
                p.y = (real32_t)y;
                listener_event(window->OnMoved, ekGUI_EVENT_WND_MOVED, window, &p, NULL, OSWindow, EvPos, void);
            }

            /* When window is moved dragging the titlebar, the focus is lost */
            _ostabstop_restore(&window->tabstop);
        }

        if (window->current_width != event->width || window->current_height != event->height)
        {
            if (window->is_resizable == TRUE && window->OnResize != NULL)
            {
                EvSize params;
                EvSize result;
                uint32_t mheight = i_menubar_current_height(window);
                params.width = (real32_t)event->width;
                params.height = (real32_t)(event->height - (gint)mheight);
                listener_event(window->OnResize, ekGUI_EVENT_WND_SIZING, window, &params, &result, OSWindow, EvSize, EvSize);
                listener_event(window->OnResize, ekGUI_EVENT_WND_SIZE, window, &result, NULL, OSWindow, EvSize, void);

                if (result.width > params.width)
                {
                    GdkGeometry hints;
                    window->minimun_width = (gint)result.width;
                    hints.min_width = window->minimun_width;
                    hints.min_height = window->minimun_height;
                    gtk_window_set_geometry_hints(GTK_WINDOW(window->control.widget), window->control.widget, &hints, (GdkWindowHints)GDK_HINT_MIN_SIZE);
                }

                if (result.height > params.height)
                {
                    GdkGeometry hints;
                    window->minimun_height = (gint)result.height + (gint)mheight;
                    hints.min_width = window->minimun_width;
                    hints.min_height = window->minimun_height;
                    gtk_window_set_geometry_hints(GTK_WINDOW(window->control.widget), window->control.widget, &hints, (GdkWindowHints)GDK_HINT_MIN_SIZE);
                }

                window->current_width = (gint)result.width;
                window->current_height = (gint)result.height + (gint)mheight;

                if (window->menu != NULL)
                    _osmenu_menubar(window->menu, window, i_menubar_required_width(window));
            }
            else
            {
                window->current_width = event->width;
                window->current_height = event->height;
            }
        }
    }

    window->current_x = event->x;
    window->current_y = event->y;

    if (window->configure_event > 0)
        window->configure_event -= 1;

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSWindow *window)
{
    guint key = 0;
    cassert_no_null(window);
    cassert_no_null(event);
    key = event->keyval;

    switch (key)
    {
    case GDK_KEY_Tab:
        if (_ostabstop_capture_tab(&window->tabstop) == FALSE)
        {
            _ostabstop_next(&window->tabstop, TRUE);
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    /* https://mail.gnome.org/archives/gtk-list/1999-August/msg00127.html */
    case GDK_KEY_ISO_Left_Tab:
        if (_ostabstop_capture_tab(&window->tabstop) == FALSE)
        {
            _ostabstop_prev(&window->tabstop, TRUE);
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case GDK_KEY_Escape:
        if (window->flags & ekWINDOW_ESC)
        {
            i_close(window, ekGUI_CLOSE_ESC);
            return TRUE;
        }
        break;

    case GDK_KEY_Return:
    case GDK_KEY_KP_Enter:
        if (_ostabstop_capture_return(&window->tabstop) == FALSE)
        {
            if (window->tabstop.defbutton != NULL)
            {
                /* 'widget' is a real GtkWindow for every window except one currently shown via a GtkPopover */
                if (GTK_IS_WINDOW(widget) == TRUE)
                {
                    GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(widget));
                    GtkWidget *bfocus = _osbutton_focus_widget(window->tabstop.defbutton);
                    if (gtk_widget_get_can_focus(bfocus) == TRUE)
                        gtk_window_set_focus(GTK_WINDOW(widget), bfocus);
                    _osbutton_command(window->tabstop.defbutton);
                    _osglobals_restore_focus(widget, focus);
                }
                else
                {
                    GtkWidget *bfocus = _osbutton_focus_widget(window->tabstop.defbutton);
                    if (gtk_widget_get_can_focus(bfocus) == TRUE)
                        gtk_widget_grab_focus(bfocus);
                    _osbutton_command(window->tabstop.defbutton);
                }
            }

            if (window->flags & ekWINDOW_RETURN)
            {
                i_close(window, ekGUI_CLOSE_INTRO);
                return TRUE;
            }
        }
        else
        {
            return FALSE;
        }
        break;

    default:
        break;
    }

    /* Check hotkeys */
    if (window->hotkeys != NULL)
    {
        vkey_t key = _osgui_vkey(event->keyval);
        uint32_t modifiers = _osgui_modifiers(event->state);
        if (_oswindow_hotkey_process(window, window->hotkeys, key, modifiers) == TRUE)
            return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyRelease(GtkWidget *widget, GdkEventKey *event, OSWindow *window)
{
    guint key = 0;
    cassert_no_null(event);
    unref(widget);
    unref(window);
    key = event->keyval;
    if (key == GDK_KEY_Alt_L || key == GDK_KEY_Alt_R)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

static ___INLINE GtkWidget *i_gtk_window(const uint32_t flags)
{
    GtkWidget *window = NULL;
    if (flags & ekWINDOW_OFFSCREEN)
    {
        window = gtk_offscreen_window_new();
    }
    else
    {
        window = gtk_application_window_new(i_GTK_APP);
        gtk_window_set_decorated(GTK_WINDOW(window), (flags & ekWINDOW_TITLE) ? TRUE : FALSE);
    }

    return window;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnWindowState(GtkWindow *widget, GdkEventWindowState *event, OSWindow *window)
{
    cassert_no_null(event);
    cassert_no_null(widget);
    if (event->new_window_state & GDK_WINDOW_STATE_FOCUSED)
        _ostabstop_restore(&window->tabstop);

    /* Maximized */
    if ((event->changed_mask & GDK_WINDOW_STATE_MAXIMIZED) && (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED))
    {
        /* Force the resize events */
        window->configure_event = 0;
    }

    /* Minimized */
    if ((event->changed_mask & GDK_WINDOW_STATE_ICONIFIED) && (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED))
    {
        if (window->OnMoved != NULL)
        {
            EvPos p;
            p.x = 0;
            p.y = 0;
            listener_event(window->OnMoved, ekGUI_EVENT_WND_MOVED, window, &p, NULL, OSWindow, EvPos, void);
        }

        if (window->OnResize != NULL)
        {
            EvSize p;
            p.width = 0;
            p.height = 0;
            listener_event(window->OnResize, ekGUI_EVENT_WND_SIZE, window, &p, NULL, OSWindow, EvSize, void);
        }
    }

    /* Restored from minimized */
    if ((event->changed_mask & GDK_WINDOW_STATE_ICONIFIED) && !(event->new_window_state & GDK_WINDOW_STATE_ICONIFIED))
    {
        /* Force new configure events (moved, resized) */
        gint x, y;
        window->current_x = -1;
        window->current_y = -1;
        window->current_width = -1;
        window->current_height = -1;
        window->configure_event = 0;
        gtk_window_get_position(GTK_WINDOW(window->control.widget), &x, &y);
        gtk_window_move(GTK_WINDOW(window->control.widget), x, y);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_create(const uint32_t flags)
{
    OSWindow *window = heap_new0(OSWindow);
    GtkWidget *widget = i_gtk_window(flags);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(widget), box);
    _oscontrol_init(cast(window, OSControl), ekGUI_TYPE_WINDOW, widget, widget, FALSE);
    window->content_box = box;
    window->popover = NULL;
    window->flags = flags;
    window->role = ENUM_MAX(gui_role_t);
    window->destroy_main_view = TRUE;
    window->is_resizable = (flags & ekWINDOW_RESIZE) ? TRUE : FALSE;
    window->configure_event = 0;
    window->current_x = -1;
    window->current_y = -1;
    window->current_width = -1;
    window->current_height = -1;
    _ostabstop_init(&window->tabstop, window);
    gtk_window_set_resizable(GTK_WINDOW(window->control.widget), (gboolean)window->is_resizable);

    if ((flags & ekWINDOW_MIN) == 0)
    {
        /* TODO: Research a secure way to disable the minimize button */
        /*
        const char *css = "window headerbar minimize titlebutton { background-image: none; }";
        GtkCssProvider *css_provider = gtk_css_provider_new();
        GtkStyleContext *style_context = gtk_widget_get_style_context(window->control.widget);
        gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
        gtk_style_context_add_provider(style_context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        const char* gdk_x11_screen_get_window_manager_name (GdkScreen *screen);
        // gtk_window_set_type_hint(GTK_WINDOW(window->control.widget), GDK_WINDOW_TYPE_HINT_MENU);
        {
            GdkDisplay *display = gdk_display_get_default();
            GdkScreen *screen = gdk_display_get_default_screen(display);
            //GdkWindow *window = gdk_screen_get_root_window(screen);
            const gchar *windowManagerName = gdk_x11_screen_get_window_manager_name(screen);
            g_print("Window Manager: %s\n", windowManagerName);
        } */
    }

    window->signal_delete = g_signal_connect(G_OBJECT(widget), "delete-event", G_CALLBACK(i_OnClose), (gpointer)window);
    window->signal_config = g_signal_connect(G_OBJECT(widget), "configure-event", G_CALLBACK(i_OnConfigure), (gpointer)window);
    window->signal_keypre = g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)window);
    window->signal_keyrel = g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(i_OnKeyRelease), (gpointer)window);
    window->signal_state = g_signal_connect(G_OBJECT(widget), "window-state-event", G_CALLBACK(i_OnWindowState), (gpointer)window);

    if (i_APP_ICON != NULL)
    {
        gtk_window_set_icon(GTK_WINDOW(window->control.widget), i_APP_ICON);
    }

    return window;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_managed(void *native_ptr)
{
    cassert(FALSE);
    unref(native_ptr);
    return NULL;
}

/*---------------------------------------------------------------------------*/

#if defined(__ASSERTS__)

static void i_count(GtkWidget *widget, gpointer data)
{
    uint32_t *n = cast(data, uint32_t);
    unref(widget);
    *n += 1;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_num_children(GtkWidget *widget)
{
    uint32_t n = 0;
    cassert(GTK_IS_CONTAINER(widget));
    gtk_container_foreach(GTK_CONTAINER(widget), i_count, (gpointer)&n);
    return n;
}

#endif

/*---------------------------------------------------------------------------*/

void oswindow_destroy(OSWindow **window)
{
    cassert_no_null(window);
    cassert_no_null(*window);
    cassert((*window)->menu == NULL);

#if GTK_CHECK_VERSION(3, 12, 0)
    i_unembed_overlay(*window);
#endif

    gtk_widget_hide((*window)->control.widget);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_delete);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_config);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_keypre);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_keyrel);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_state);

    if ((*window)->destroy_main_view == TRUE && (*window)->main_panel != NULL)
    {
        OSPanel *panel = (*window)->main_panel;
        oswindow_detach_panel(*window, panel);
        _ospanel_destroy(&panel);
    }

    cassert((*window)->main_panel == NULL);
    if ((*window)->accel != NULL)
    {
        gtk_window_remove_accel_group(GTK_WINDOW((*window)->control.widget), (*window)->accel);
        g_object_unref((*window)->accel);
    }

    listener_destroy(&(*window)->OnMoved);
    listener_destroy(&(*window)->OnResize);
    listener_destroy(&(*window)->OnClose);
    _oswindow_hotkey_destroy(&(*window)->hotkeys);
    _ostabstop_remove(&(*window)->tabstop);
    cassert(i_num_children((*window)->control.widget) == 1);
    cassert(i_num_children((*window)->content_box) == 0);
    g_object_unref((*window)->control.widget);
    heap_delete(window, OSWindow);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnMoved(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    listener_update(&window->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnResize(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    listener_update(&window->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnClose(OSWindow *window, Listener *listener)
{
    cassert_no_null(window);
    listener_update(&window->OnClose, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_title(OSWindow *window, const char_t *text)
{
    cassert_no_null(window);
    gtk_window_set_title(GTK_WINDOW(window->control.widget), cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

void oswindow_edited(OSWindow *window, const bool_t is_edited)
{
    unref(window);
    unref(is_edited);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_movable(OSWindow *window, const bool_t is_movable)
{
    unref(window);
    unref(is_movable);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_z_order(OSWindow *window, OSWindow *below_window)
{
    cassert_no_null(window);
    unref(window);
    unref(below_window);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_alpha(OSWindow *window, const real32_t alpha)
{
    unref(window);
    unref(alpha);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_enable_mouse_events(OSWindow *window, const bool_t enabled)
{
    unref(window);
    unref(enabled);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_hotkey(OSWindow *window, const vkey_t key, const uint32_t modifiers, Listener *listener)
{
    cassert_no_null(window);
    _oswindow_hotkey_set(&window->hotkeys, key, modifiers, listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    _ostabstop_list_add(&window->tabstop, control);
    if (control == NULL)
    {
        /* The window main panel has changed. We ensure that default button is still valid */
        window->tabstop.defbutton = _oswindow_apply_default_button(window, window->tabstop.defbutton);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_tabcycle(OSWindow *window, const bool_t cycle)
{
    cassert_no_null(window);
    window->tabstop.cycle = cycle;
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_tabstop(OSWindow *window, const bool_t next)
{
    cassert_no_null(window);
    if (next == TRUE)
        return _ostabstop_next(&window->tabstop, FALSE);
    else
        return _ostabstop_prev(&window->tabstop, FALSE);
}

/*---------------------------------------------------------------------------*/

gui_focus_t oswindow_focus(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    cassert_no_null(control);
    return _ostabstop_move(&window->tabstop, control);
}

/*---------------------------------------------------------------------------*/

OSControl *oswindow_get_focus(const OSWindow *window)
{
    cassert_no_null(window);
    return window->tabstop.current;
}

/*---------------------------------------------------------------------------*/

gui_tab_t oswindow_info_focus(const OSWindow *window, void **next_ctrl)
{
    cassert_no_null(window);
    return _ostabstop_info_focus(&window->tabstop, next_ctrl);
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_panel(OSWindow *window, OSPanel *panel)
{
    cassert_no_null(window);
    cassert(window->main_panel == NULL);
    gtk_box_pack_end(GTK_BOX(window->content_box), cast(panel, OSControl)->widget, TRUE, TRUE, 0);
    window->main_panel = panel;
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_panel(OSWindow *window, OSPanel *panel)
{
    cassert_no_null(window);
    cassert(window->main_panel == panel);
    gtk_container_remove(GTK_CONTAINER(window->content_box), cast(panel, OSControl)->widget);
    window->main_panel = NULL;
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_window(OSWindow *parent_window, OSWindow *child_window)
{
    unref(parent_window);
    unref(child_window);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window)
{
    unref(parent_window);
    unref(child_window);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_wayland(void)
{
#ifdef GDK_WINDOWING_WAYLAND
    GdkDisplay *display = gdk_display_get_default();
    return (bool_t)GDK_IS_WAYLAND_DISPLAY(display);
#else
    return FALSE;
#endif
}

/*---------------------------------------------------------------------------*/

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    /* window_overlay() now launches through oswindow_launch_overlay() instead -- this
       entry point is only ever reached from window_show(), always with no parent. */
    cassert(parent_window == NULL);
    unref(parent_window);
    window->configure_event += 1;
    window->role = ekGUI_ROLE_MAIN;
#if GTK_CHECK_VERSION(3, 12, 0)
    i_unembed_overlay(window);
#endif
    gtk_widget_show(window->control.widget);
    _ostabstop_restore(&window->tabstop);
}

/*---------------------------------------------------------------------------*/

void oswindow_launch_overlay(OSWindow *window, OSWindow *parent_window, const real32_t x, const real32_t y, const align_t halign, const align_t valign)
{
    cassert_no_null(window);
    cassert_no_null(parent_window);
    window->configure_event += 1;
    window->role = ekGUI_ROLE_OVERLAY;

    if (i_is_wayland() == TRUE)
    {
#if GTK_CHECK_VERSION(3, 12, 0)
        i_show_overlay(window, parent_window, (gint)x, (gint)y, halign, valign);
#else
        cassert_msg(FALSE, "window_overlay() on Wayland requires GTK >= 3.12 (GtkPopover)");
#endif
    }
    else
    {
        GtkRequisition natural;
        GdkWindow *parent_gdk_window;
        gint parent_x = 0;
        gint parent_y = 0;
        gint local_x = 0;
        gint local_y = 0;

        parent_gdk_window = gtk_widget_get_window(parent_window->control.widget);
        gdk_window_get_origin(parent_gdk_window, &parent_x, &parent_y);
        gtk_widget_get_preferred_size(window->control.widget, NULL, &natural);

        switch (halign)
        {
        case ekLEFT:
            local_x = (gint)x;
            break;
        case ekRIGHT:
            local_x = (gint)x - natural.width;
            break;
        case ekCENTER:
            local_x = (gint)x - natural.width / 2;
            break;
        case ekJUSTIFY:
        default:
            cassert_default(halign);
        }

        switch (valign)
        {
        case ekTOP:
            local_y = (gint)y;
            break;
        case ekBOTTOM:
            local_y = (gint)y - natural.height;
            break;
        case ekCENTER:
            local_y = (gint)y - natural.height / 2;
            break;
        case ekJUSTIFY:
        default:
            cassert_default(valign);
        }

        gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), GTK_WINDOW(parent_window->control.widget));
        gtk_window_move(GTK_WINDOW(window->control.widget), parent_x + local_x, parent_y + local_y);
        gtk_widget_show(window->control.widget);
    }

    _ostabstop_restore(&window->tabstop);
}

/*---------------------------------------------------------------------------*/

void oswindow_hide(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    unref(parent_window);
    window->role = ENUM_MAX(gui_role_t);

    if (window->popover != NULL)
    {
        gtk_widget_hide(window->popover);
    }
    else
    {
        gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), NULL);
        gtk_widget_hide(window->control.widget);
    }
}

/*---------------------------------------------------------------------------*/

uint32_t oswindow_launch_modal(OSWindow *window, OSWindow *parent_window)
{
    bool_t wayland_fake_modal = FALSE;
    cassert_no_null(window);
    cassert(window->runloop == NULL);
    /* In case this same window was previously launched with the overlay role: a modal
       must be a genuine, independent toplevel with its own real keyboard focus, never
       content embedded in another window's GtkPopover. */
#if GTK_CHECK_VERSION(3, 12, 0)
    i_unembed_overlay(window);
#endif
    _ostabstop_restore(&window->tabstop);

    /*
     * gtk_window_set_modal(TRUE) together with gtk_window_set_transient_for() makes
     * Mutter treat the pair as an "attached dialog" under Wayland: dragging the modal
     * drags the parent along.
     */
    wayland_fake_modal = (bool_t)(i_is_wayland() == TRUE && parent_window != NULL);
    if (wayland_fake_modal == TRUE)
        gtk_widget_set_sensitive(parent_window->content_box, FALSE);
    else
        gtk_window_set_modal(GTK_WINDOW(window->control.widget), TRUE);

    window->configure_event += 1;
    window->role = ekGUI_ROLE_MODAL;
    window->runloop = g_main_loop_new(NULL, FALSE);

    if (parent_window != NULL)
        gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), GTK_WINDOW(parent_window->control.widget));

    gtk_widget_show(window->control.widget);
    g_main_loop_run(window->runloop);
    g_main_loop_unref(window->runloop);
    gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), NULL);

    if (wayland_fake_modal == TRUE)
        gtk_widget_set_sensitive(parent_window->content_box, TRUE);

    if (parent_window != NULL)
        _ostabstop_restore(&parent_window->tabstop);

    window->runloop = NULL;
    return window->modal_return;
}

/*---------------------------------------------------------------------------*/

void oswindow_stop_modal(OSWindow *window, const uint32_t return_value)
{
    cassert_no_null(window);
    cassert_no_null(window->runloop);
    cassert(g_main_loop_is_running(window->runloop) == TRUE);
    window->modal_return = return_value;
    window->role = ENUM_MAX(gui_role_t);
    if (!(window->flags & ekWINDOW_MODAL_NOHIDE))
        gtk_widget_hide(window->control.widget);
    g_main_loop_quit(window->runloop);
    gtk_window_set_modal(GTK_WINDOW(window->control.widget), FALSE);
}

/*---------------------------------------------------------------------------*/

bool_t oswindow_get_maximize(const OSWindow *window)
{
    cassert_no_null(window);
#if GTK_CHECK_VERSION(3, 12, 0)
    return (bool_t)gtk_window_is_maximized(GTK_WINDOW(window->control.widget));
#else
    {
        GdkWindow *gdk_window = gtk_widget_get_window(window->control.widget);
        if (gdk_window != NULL)
        {
            GdkWindowState state = gdk_window_get_state(gdk_window);
            return (bool_t)((state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
        }
        else
        {
            return FALSE;
        }
    }
#endif
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnMaximize(OSWindow *window)
{
    cassert_no_null(window);
    bthread_sleep(10);
    gtk_window_maximize(GTK_WINDOW(window->control.widget));
    return FALSE;
}

/*---------------------------------------------------------------------------*/

void oswindow_maximize(OSWindow *window)
{
    cassert_no_null(window);
    if (window->is_resizable == TRUE)
        g_idle_add((GSourceFunc)i_OnMaximize, window);
}

/*---------------------------------------------------------------------------*/

bool_t oswindow_get_minimize(const OSWindow *window)
{
    cassert_no_null(window);
    return (bool_t)!gtk_window_is_active(GTK_WINDOW(window->control.widget));
}

/*---------------------------------------------------------------------------*/

void oswindow_minimize(OSWindow *window)
{
    cassert_no_null(window);
    gtk_window_iconify(GTK_WINDOW(window->control.widget));
}

/*---------------------------------------------------------------------------*/

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y)
{
    cassert_no_null(window);
    cassert_no_null(x);
    cassert_no_null(y);
    if (oswindow_get_minimize(window) == TRUE)
    {
        *x = 0;
        *y = 0;
    }
    else if (i_is_wayland() == TRUE)
    {
        if (*x == REAL32_MAX && *y == REAL32_MAX)
        {
            *x = 0;
            *y = 0;
        }
    }
    else
    {
        if (*x == REAL32_MAX && *y == REAL32_MAX)
        {
            gint wx, wy;
            gtk_window_get_position(GTK_WINDOW(window->control.widget), &wx, &wy);
            *x = (real32_t)wx;
            *y = (real32_t)wy;
        }
        else
        {
            *x = (real32_t)window->current_x + *x;
            *y = (real32_t)window->current_y + *y;
        }
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y)
{
    cassert_no_null(window);
    window->configure_event += 1;
    gtk_window_move(GTK_WINDOW(window->control.widget), (gint)x, (gint)y);
}

/*---------------------------------------------------------------------------*/

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height)
{
    GdkWindow *gdk_window = NULL;
    cassert_no_null(window);
    cassert_no_null(width);
    cassert_no_null(height);
    gdk_window = gtk_widget_get_window(window->control.widget);
    if (gdk_window != NULL)
    {
        GdkRectangle rect;
        gdk_window_get_frame_extents(gdk_window, &rect);
        *width = (real32_t)rect.width;
        *height = (real32_t)rect.height;
    }
    else
    {
        gint w, h;
        gtk_window_get_size(GTK_WINDOW(window->control.widget), &w, &h);
        *width = (real32_t)w;
        *height = (real32_t)h;
    }
}

/*---------------------------------------------------------------------------*/

static void i_window_decoration_delta(OSWindow *window, GtkWidget *box, gint *dwidth, gint *dheight)
{
    cassert_no_null(window);
    cassert_no_null(dwidth);
    cassert_no_null(dheight);
    *dwidth = 0;
    *dheight = 0;
    if (gtk_widget_get_realized(window->control.widget) == TRUE)
    {
        *dwidth = gtk_widget_get_allocated_width(window->control.widget) - gtk_widget_get_allocated_width(box);
        *dheight = gtk_widget_get_allocated_height(window->control.widget) - gtk_widget_get_allocated_height(box);
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnBoxFirstMap(GtkWidget *box, GdkEvent *event, gpointer data)
{
    unref(event);
    unref(data);
    /* The size request below is only meant to give the window its initial size; drop
       it right after the first map so the user can still shrink/grow the window freely
       afterwards (a resizable window must not keep a permanent minimum size). */
    gtk_widget_set_size_request(box, -1, -1);
    g_signal_handlers_disconnect_by_func(box, (gpointer)(intptr_t)i_OnBoxFirstMap, NULL);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_update_menu_size(OSWindow *window)
{
    GtkRequisition msize;
    GtkWidget *box = NULL;
    gint width, height;
    cassert_no_null(window);
    box = window->content_box;

    if (window->menu != NULL)
    {
        GtkWidget *wmenu = _osmenu_menubar(window->menu, window, i_menubar_required_width(window));
        gtk_widget_get_preferred_size(wmenu, &msize, NULL);
    }
    else
    {
        msize.width = 0;
        msize.height = 0;
    }

    width = window->current_width;
    height = window->current_height + msize.height;

    if (window->is_resizable == TRUE)
    {
        window->configure_event += 1;
        gtk_widget_set_size_request(window->control.widget, -1, -1);

        if (gtk_widget_get_realized(window->control.widget) == TRUE)
        {
            /* Window already mapped: an explicit gtk_window_resize() targets the whole
               surface. On X11 the window manager draws the decoration outside of it, so
               the delta is 0. On Wayland GTK draws its own decoration (CSD) inside the
               same surface, so it must be added on top of 'width x height' or the content
               box ends up squeezed into less space than requested. */
            gint dwidth, dheight;
            i_window_decoration_delta(window, box, &dwidth, &dheight);
            gtk_window_resize(GTK_WINDOW(window->control.widget), width + dwidth, height + dheight);
        }
        else
        {
            /* Not shown yet: hint the *content* box instead of the toplevel, and let GTK
               derive the toplevel's natural size from it. Any decoration GTK adds on top
               (Wayland CSD) is then additive instead of being carved out of 'width x
               height', which is what made the initial size come out too small/clipped. */
            gtk_widget_set_size_request(box, width, height);
            g_signal_handlers_disconnect_by_func(box, (gpointer)(intptr_t)i_OnBoxFirstMap, NULL);
            g_signal_connect(box, "map-event", G_CALLBACK(i_OnBoxFirstMap), NULL);
        }
    }
    else
    {
        /* Non-resizable window: GTK keeps a non-resizable toplevel sized to fit its
           child's requisition, on every backend, so hinting the content box is enough. */
        gtk_widget_set_size_request(window->control.widget, -1, -1);
        gtk_widget_set_size_request(box, width, height);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_client_size(OSWindow *window, const real32_t width, const real32_t height)
{
    cassert_no_null(window);
    window->current_width = (gint)width;
    window->current_height = (gint)height;

    if (window->is_resizable == TRUE)
    {
        window->minimun_width = -1;
        window->minimun_height = -1;
    }

    i_update_menu_size(window);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button)
{
    cassert_no_null(window);
    window->tabstop.defbutton = _oswindow_apply_default_button(window, button);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_cursor(OSWindow *window, Cursor *cursor)
{
    GdkWindow *gdkwindow = NULL;
    cassert_no_null(window);
    gdkwindow = gtk_widget_get_window(window->control.widget);
    if (gdkwindow != NULL)
        gdk_window_set_cursor(gdkwindow, cast(cursor, GdkCursor));
}

/*---------------------------------------------------------------------------*/

void oswindow_property(OSWindow *window, const gui_prop_t property, const void *value)
{
    cassert_no_null(window);
    unref(value);
    switch (property)
    {
    case ekGUI_PROP_CHILDREN:
        window->destroy_main_view = FALSE;
        break;
    default:
        cassert_default(property);
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_widget_set_focus(OSWindow *window, OSWidget *widget)
{
    OSControl *control = NULL;
    cassert_no_null(widget);
    unref(window);
    control = cast(g_object_get_data(G_OBJECT(widget), "OSControl"), OSControl);
    cassert_no_null(control);
    if (control->type == ekGUI_TYPE_EDITBOX)
    {
        if (_osedit_autosel(cast(control, OSEdit)) == FALSE)
        {
#if GTK_CHECK_VERSION(3, 16, 0)
            /* osedit can be multiline text-view */
            if (GTK_IS_ENTRY(widget))
            {
                gtk_entry_grab_focus_without_selecting((GTK_ENTRY(widget)));
                return;
            }
#endif
        }
    }
    else if (control->type == ekGUI_TYPE_COMBOBOX)
    {
        if (_oscombo_autosel(cast(control, OSCombo)) == FALSE)
        {
#if GTK_CHECK_VERSION(3, 16, 0)
            /* osedit can be multiline text-view */
            if (GTK_IS_ENTRY(widget))
            {
                gtk_entry_grab_focus_without_selecting((GTK_ENTRY(widget)));
                return;
            }
#endif
        }
    }

    gtk_widget_grab_focus(GTK_WIDGET(widget));
}

/*---------------------------------------------------------------------------*/

static void i_get_controls(GtkWidget *widget, gpointer data)
{
    OSControl *control = cast(g_object_get_data(G_OBJECT(widget), "OSControl"), OSControl);
    if (control != NULL)
    {
        ArrPt(OSControl) *controls = cast(data, ArrPt(OSControl));
        if (arrpt_find(controls, control, OSControl) == UINT32_MAX)
            arrpt_append(controls, control, OSControl);
    }

    if (GTK_IS_CONTAINER(widget) == TRUE)
        gtk_container_foreach(GTK_CONTAINER(widget), i_get_controls, data);
}

/*---------------------------------------------------------------------------*/

void _oswindow_find_all_controls(OSWindow *window, ArrPt(OSControl) *controls)
{
    cassert_no_null(window);
    cassert(arrpt_size(controls, OSControl) == 0);
    gtk_container_foreach(GTK_CONTAINER(window->control.widget), i_get_controls, (gpointer)controls);
}

/*---------------------------------------------------------------------------*/

const ArrPt(OSControl) *_oswindow_get_all_controls(const OSWindow *window)
{
    cassert_no_null(window);
    return window->tabstop.controls;
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app(void *app, void *icon)
{
    cassert(i_GTK_APP == NULL);
    cassert(i_APP_ICON == NULL);
    i_GTK_APP = cast(app, GtkApplication);
    i_APP_ICON = cast(icon, GdkPixbuf);
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app_terminate(void)
{
    cassert(i_APP_TERMINATE == FALSE);
    i_APP_TERMINATE = TRUE;
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_menubar(OSWindow *window, OSMenu *menu)
{
    GtkWidget *box = window->content_box;
    GtkWidget *wmenu = NULL;
    cassert_no_null(window);
    cassert(window->menu == NULL);
    cassert(i_num_children(box) == 1);
    wmenu = _osmenu_menubar(menu, window, i_menubar_required_width(window));
    gtk_box_pack_start(GTK_BOX(box), wmenu, FALSE, FALSE, 0);
    cassert(i_num_children(box) == 2);
    gtk_widget_show_all(wmenu);
    window->menu = menu;

    if (window->accel == NULL)
    {
        window->accel = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(window->control.widget), window->accel);
    }

    _osmenu_set_accel(window->menu, window->accel);
    i_update_menu_size(window);
    cassert(i_num_children(box) == 2);
}

/*---------------------------------------------------------------------------*/

void _oswindow_unset_menubar(OSWindow *window, OSMenu *menu)
{
    cassert_no_null(window);
    cassert(window->menu == menu);
    if (menu != NULL)
    {
        GtkWidget *box = window->content_box;
        GtkWidget *wmenu = NULL;
        cassert(window->accel != NULL);
        cassert(i_num_children(box) == 2);
        wmenu = _osmenu_menubar_unlink(menu, window);
        gtk_widget_hide(wmenu);
        _osmenu_unset_accel(menu, window->accel);
        gtk_container_remove(GTK_CONTAINER(box), wmenu);
        cassert(i_num_children(box) == 1);
        window->menu = NULL;
        i_update_menu_size(window);
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_unset_focus(OSWindow *window)
{
    cassert_no_null(window);
    /* This event can be received during window destroy */
    if (window->tabstop.tablist != NULL)
    {
        if (window->role == ekGUI_ROLE_OVERLAY)
        {
            if (i_close(window, ekGUI_CLOSE_DEACT) == TRUE)
                window->role = ENUM_MAX(gui_role_t);
        }
    }
}

/*---------------------------------------------------------------------------*/

GtkAccelGroup *_oswindow_accel(const OSWindow *window)
{
    cassert_no_null(window);
    return window->accel;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_oswindow_content_box(const OSWindow *window)
{
    cassert_no_null(window);
    return window->content_box;
}

/*---------------------------------------------------------------------------*/

static ___INLINE OSWindow *i_root(GtkWidget *widget)
{
    GtkWidget *iter = NULL;
    cassert_no_null(widget);

    for (iter = widget; iter != NULL; iter = gtk_widget_get_parent(iter))
    {
        if (GTK_IS_WINDOW(iter) == TRUE)
        {
            OSControl *control = cast(g_object_get_data(G_OBJECT(iter), "OSControl"), OSControl);
            cassert_no_null(control);
            cassert(control->type == ekGUI_TYPE_WINDOW);
            return cast(control, OSWindow);
        }

#if GTK_CHECK_VERSION(3, 12, 0)
        if (GTK_IS_POPOVER(iter) == TRUE)
        {
            OSWindow *window = cast(g_object_get_data(G_OBJECT(iter), "nappgui-overlay-window"), OSWindow);
            cassert_no_null(window);
            return window;
        }
#endif
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

bool_t _oswindow_mouse_down(OSControl *control)
{
    OSWindow *window = NULL;
    cassert_no_null(control);
    window = i_root(control->widget);
    if (window != NULL)
        return _ostabstop_mouse_down(&window->tabstop, control);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

void _oswindow_release_transient_focus(OSControl *control)
{
    OSWindow *window = NULL;
    cassert_no_null(control);
    window = i_root(control->widget);
    if (window != NULL)
        _ostabstop_release_transient(&window->tabstop, control);
}

/*---------------------------------------------------------------------------*/

void _oswindow_cursor_from_child(GtkWidget *widget, GdkCursor *cursor)
{
    OSWindow *window = i_root(widget);
    if (window != NULL)
    {
        GtkWidget *root_widget = window->control.widget;
        GdkWindow *gdkwindow = NULL;
        /* While shown via a GtkPopover (see i_show_overlay()), window->control.widget
           is a hidden, contentless shell -- the popover itself is the real, visible
           surface the cursor must be set on. */
        if (window->popover != NULL)
            root_widget = window->popover;
        gdkwindow = gtk_widget_get_window(root_widget);
        if (gdkwindow != NULL)
            gdk_window_set_cursor(gdkwindow, cursor);
    }
}
