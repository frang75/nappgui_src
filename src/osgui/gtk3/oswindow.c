/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oswindow.c
 *
 */

/* Operating System native window */

#include "oswindow.h"
#include "oswindow.inl"
#include "osgui_gtk.inl"
#include "osbutton.inl"
#include "oscombo.inl"
#include "oscontrol.inl"
#include "osedit.inl"
#include "osmenu.inl"
#include "ospanel.inl"
#include "ospopup.inl"
#include "osview.inl"
#include "arrpt.h"
#include "arrst.h"
#include "cassert.h"
#include "event.h"
#include "heap.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef struct _hotkey_t HotKey;

struct _hotkey_t
{
    vkey_t key;
    uint32_t modifiers;
    Listener *listener;
};

DeclSt(HotKey);

struct _oswindow_t
{
    OSControl control;
    OSMenu *menu;
    GMainLoop *runloop;
    uint32_t modal_return;
    uint32_t flags;
    GtkAccelGroup *accel;
    OSPanel *main_panel;
    gint signal_delete;
    gint signal_config;
    gint signal_key;
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
    ArrSt(HotKey) *hotkeys;
    ArrPt(OSControl) *tabstops;
    GtkWidget *ctabstop;
    OSButton *defbutton;
    bool_t destroy_main_view;
    bool_t is_resizable;
    bool_t resize_event;
    gint current_x;
    gint current_y;
    gint current_width;
    gint current_height;
    gint minimun_width;
    gint minimun_height;
};

/*---------------------------------------------------------------------------*/

static GtkApplication* i_GTK_APP = NULL;
static GdkPixbuf *i_APP_ICON = NULL;
static bool_t i_APP_TERMINATE = FALSE;
static int i_SCROLL_OFFSET = 10;

/*---------------------------------------------------------------------------*/

static gboolean i_OnClose(GtkWidget *widget, GdkEvent *event, OSWindow *window)
{
    bool_t closed = TRUE;
    cassert_no_null(window);
    cassert_unref(window->control.widget == widget, widget);
    unref(event);

    if (window->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = ekGUI_CLOSE_BUTTON;
        listener_event(window->OnClose, ekGUI_EVENT_WND_CLOSE, window, &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    if (closed == TRUE)
        gtk_widget_hide(widget);

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_close(OSWindow *window, const gui_close_t close_origin)
{
    bool_t closed = TRUE;
    cassert_no_null(window);

    if (window->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = close_origin;
        listener_event(window->OnClose, ekGUI_EVENT_WND_CLOSE, window, &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    if (closed == TRUE)
        gtk_widget_hide(window->control.widget);

    return closed;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnResize(GtkWidget *widget, GdkEventConfigure *event, OSWindow *window)
{
    if (i_APP_TERMINATE == TRUE)
        return FALSE;

    cassert_no_null(window);
    cassert_unref(window->control.widget == widget, widget);
    if (window->resize_event == FALSE)
    {
        window->resize_event = TRUE;
        return FALSE;
    }

    if (window->is_resizable == FALSE)
        return FALSE;

    if (window->current_width > 0 && (window->current_width != event->width || window->current_height != event->height))
    {
        if (window->OnResize != NULL)
        {
            EvSize params;
            EvSize result;
            params.width = (real32_t)event->width;
            params.height = (real32_t)event->height;
            listener_event(window->OnResize, ekGUI_EVENT_WND_SIZING, window, &params, &result, OSWindow, EvSize, EvSize);
            listener_event(window->OnResize, ekGUI_EVENT_WND_SIZE, window, &result, NULL, OSWindow, EvSize, void);

            if ((gint)result.width > event->width)
            {
                GdkGeometry hints;
                window->minimun_width = (gint)result.width;
                hints.min_width = window->minimun_width;
                hints.min_height = window->minimun_height;
                gtk_window_set_geometry_hints(GTK_WINDOW(window->control.widget), window->control.widget, &hints, (GdkWindowHints)GDK_HINT_MIN_SIZE);
            }

            if ((gint)result.height > event->height)
            {
                GdkGeometry hints;
                window->minimun_height = (gint)result.height;
                hints.min_width = window->minimun_width;
                hints.min_height = window->minimun_height;
                gtk_window_set_geometry_hints(GTK_WINDOW(window->control.widget), window->control.widget, &hints, (GdkWindowHints)GDK_HINT_MIN_SIZE);
            }

            window->current_width = (gint)result.width;
            window->current_height = (gint)result.height;
        }
        else
        {
            window->current_width = event->width;
            window->current_height = event->height;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static __INLINE GtkWidget* i_focus_widget(const OSControl *control)
{
    cassert_no_null(control);
    switch (control->type) {
    case ekGUI_TYPE_LABEL:
    case ekGUI_TYPE_PROGRESS:
        return NULL;

    case ekGUI_TYPE_SLIDER:
    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_UPDOWN:
        return control->widget;

    case ekGUI_TYPE_CUSTOMVIEW:
        return _osview_focus((OSView*)control);

    case ekGUI_TYPE_EDITBOX:
        return _osedit_focus((OSEdit*)control);

    case ekGUI_TYPE_BUTTON:
        return _osbutton_focus((OSButton*)control);

    case ekGUI_TYPE_POPUP:
        return _ospopup_focus((OSPopUp*)control);

    case ekGUI_TYPE_COMBOBOX:
        return _oscombo_focus((OSCombo*)control);

    case ekGUI_TYPE_TABLEVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
    cassert_default();
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static __INLINE uint32_t i_search_tabstop(const OSControl **tabstop, const uint32_t size, GtkWidget *widget)
{
    register uint32_t i;
    if (widget == NULL)
        return UINT32_MAX;

    for (i = 0; i < size; ++i)
        if (i_focus_widget(tabstop[i]) == widget)
            return i;

    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

static bool_t i_widget_grab_focus(GtkWidget *widget, const OSControl *control, GtkWidget **ctabstop)
{
    cassert_no_null(control);
    cassert_no_null(ctabstop);
    if (widget && gtk_widget_is_sensitive(widget) && gtk_widget_get_visible(widget))
    {
        *ctabstop = widget;
        if (control->type == ekGUI_TYPE_EDITBOX && _osedit_autoselect((const OSEdit*)control) == FALSE)
        {
            if (GTK_IS_ENTRY(widget) == TRUE)
            {
            /*#if GTK_CHECK_VERSION(3, 16, 0)
                gtk_widget_grab_focus(widget);
                gtk_entry_grab_focus_without_selecting(GTK_ENTRY(widget));
            #else*/
                gtk_widget_grab_focus(widget);
                gtk_editable_select_region(GTK_EDITABLE(widget), -1, -1);
            /*#endif*/
            }
            else
            {
                gtk_widget_grab_focus(widget);
            }
        }
        else
        {
            gtk_widget_grab_focus(widget);
        }

        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_control_rect(const OSControl *control, RectI *rect)
{
    real32_t x, y, w, h;
    cassert_no_null(rect);
    _oscontrol_get_origin(control, &x, &y);
    _oscontrol_get_size(control, &w, &h);
    rect->left = (int)x;
    rect->top = (int)y;
    rect->right = (int)(x + w);
    rect->bottom = (int)(y + h);
}

/*---------------------------------------------------------------------------*/

static void i_set_tabstop(const OSControl **tabstop, const uint32_t size, const uint32_t index, const bool_t reverse, GtkWidget **ctabstop)
{
    register uint32_t idx = index, i;
    cassert(index < size);
    for (i = 0 ; i < size; ++i)
    {
        register const OSControl *control = tabstop[idx];
        register GtkWidget *widget = i_focus_widget(control);

        if (i_widget_grab_focus(widget, control, ctabstop) == TRUE)
        {
            GtkWidget *parent = gtk_widget_get_parent(control->widget);
	        /*const gchar *ptype = G_OBJECT_TYPE_NAME(parent);*/
            OSControl *pcontrol = (OSControl*)g_object_get_data(G_OBJECT(parent), "OSControl");
            cassert_no_null(pcontrol);
            if (pcontrol->type == ekGUI_TYPE_PANEL)
            {
                RectI prect, crect;
                int scroll_x = INT_MAX, scroll_y = INT_MAX;

                i_control_rect(control, &crect);
                _ospanel_scroll_frame((OSPanel*)pcontrol, &prect);

                if (prect.left > crect.left)
                    scroll_x = (crect.left - i_SCROLL_OFFSET);
                else if (prect.right < crect.right)
                    scroll_x = (crect.right + i_SCROLL_OFFSET) - (prect.right - prect.left);

                if (prect.top > crect.top)
                    scroll_y = (crect.top - i_SCROLL_OFFSET);
                else if (prect.bottom < crect.bottom)
                    scroll_y = (crect.bottom + i_SCROLL_OFFSET) - (prect.bottom - prect.top);

                if (scroll_x != INT_MAX || scroll_y != INT_MAX)
                    _ospanel_scroll((OSPanel*)pcontrol, scroll_x, scroll_y);
            }

            return;
        }

        if (reverse == TRUE)
        {
            if (idx == 0)
                idx = size - 1;
            else
                idx -= 1;
        }
        else
        {
            if (idx == size - 1)
                idx = 0;
            else
                idx += 1;
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_next_tabstop(const ArrPt(OSControl) *tabstops, GtkWidget *widget, GtkWidget **ctabstop)
{
    register uint32_t size = arrpt_size(tabstops, OSControl);
    if (size > 0)
    {
        register const OSControl **tabstop = arrpt_all_const(tabstops, OSControl);
        register uint32_t tabindex = i_search_tabstop(tabstop, size, widget);
        if (tabindex == UINT32_MAX)
            tabindex = 0;
        if (tabindex == size - 1)
            tabindex = 0;
        else
            tabindex += 1;
        i_set_tabstop(tabstop, size, tabindex, FALSE, ctabstop);
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_previous_tabstop(const ArrPt(OSControl) *tabstops, GtkWidget *widget, GtkWidget **ctabstop)
{
    register uint32_t size = arrpt_size(tabstops, OSControl);
    if (size > 0)
    {
        register const OSControl **tabstop = arrpt_all_const(tabstops, OSControl);
        register uint32_t tabindex = i_search_tabstop(tabstop, size, widget);
        if (tabindex == UINT32_MAX)
            tabindex = 0;
        if (tabindex == 0)
            tabindex = size - 1;
        else
            tabindex -= 1;
        i_set_tabstop(tabstop, size, tabindex, TRUE, ctabstop);
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_ctabstop(const ArrPt(OSControl) *tabstops, GtkWidget **ctabstop)
{
    register uint32_t size = arrpt_size(tabstops, OSControl);
    cassert_no_null(ctabstop);
    if (size > 0)
    {
        register const OSControl **tabstop = arrpt_all_const(tabstops, OSControl);
        register uint32_t tabindex = 0;

        if (*ctabstop == NULL)
            *ctabstop = tabstop[0]->widget;

        tabindex = i_search_tabstop(tabstop, size, *ctabstop);
        if (tabindex == UINT32_MAX)
            tabindex = 0;

        i_set_tabstop(tabstop, size, tabindex, FALSE, ctabstop);
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSWindow *window)
{
    guint key = 0;
    cassert_no_null(event);
    key = event->keyval;
    switch (key) {
    case GDK_KEY_Tab:
    {
        GtkWidget *tabstop = gtk_window_get_focus(GTK_WINDOW(widget));
        i_set_next_tabstop(window->tabstops, tabstop, &window->ctabstop);
        return TRUE;
    }

    /* https://mail.gnome.org/archives/gtk-list/1999-August/msg00127.html */
    case GDK_KEY_ISO_Left_Tab:
    {
        GtkWidget *tabstop = gtk_window_get_focus(GTK_WINDOW(widget));
        i_set_previous_tabstop(window->tabstops, tabstop, &window->ctabstop);
        return TRUE;
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
        if (window->defbutton != NULL)
        {
            GtkWidget *focus = gtk_window_get_focus(GTK_WINDOW(widget));
            GtkWidget *bfocus = _osbutton_focus(window->defbutton);
            gtk_window_set_focus(GTK_WINDOW(widget), bfocus);
            _osbutton_command(window->defbutton);
            gtk_window_set_focus(GTK_WINDOW(widget), focus);
        }

        if (window->flags & ekWINDOW_RETURN)
        {
            i_close(window, ekGUI_CLOSE_INTRO);
            return TRUE;
        }
        break;

    default:
        break;
    }

    /* Check hotkeys */
    if (window->hotkeys != NULL)
    {
        uint32_t modifiers = 0;

        if (event->state & GDK_SHIFT_MASK)
            modifiers |= ekMKEY_SHIFT;

        if (event->state & GDK_CONTROL_MASK)
            modifiers |= ekMKEY_CONTROL;

        if (event->state & GDK_MOD1_MASK)
            modifiers |= ekMKEY_ALT;

        arrst_foreach(hotkey, window->hotkeys, HotKey)
            if (key == kVIRTUAL_KEY[hotkey->key] && modifiers == hotkey->modifiers)
            {
                if (hotkey->listener != NULL)
                {
                    EvKey params;
                    params.key = hotkey->key;
                    params.modifiers = hotkey->modifiers;
                    listener_event(hotkey->listener, ekGUI_EVENT_KEYDOWN, window, &params, NULL, OSWindow, EvKey, void);
                    return TRUE;
                }
            }
        arrst_end()
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_create(const uint32_t flags)
{
    OSWindow *window = heap_new0(OSWindow);
    GtkWidget *widget = gtk_application_window_new(i_GTK_APP);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(widget), box);
    _oscontrol_init((OSControl*)window, ekGUI_TYPE_WINDOW, widget, widget, FALSE);
    window->flags = flags;
	window->destroy_main_view = TRUE;
	window->tabstops = arrpt_create(OSControl);
    window->is_resizable = (flags & ekWINDOW_RESIZE) == ekWINDOW_RESIZE ? TRUE : FALSE;
	window->resize_event = TRUE;
    gtk_window_set_resizable(GTK_WINDOW(window->control.widget), (gboolean)window->is_resizable);
    window->signal_delete = g_signal_connect(G_OBJECT(widget), "delete-event", G_CALLBACK(i_OnClose), (gpointer)window);
    window->signal_config = g_signal_connect(G_OBJECT(widget), "configure-event", G_CALLBACK(i_OnResize), (gpointer)window);
    window->signal_key = g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK (i_OnKeyPress), (gpointer)window);

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

#if defined (__ASSERTS__)
/*---------------------------------------------------------------------------*/

static void i_count(GtkWidget *widget, gpointer data)
{
    uint32_t *n = (uint32_t*)data;
    unref(widget);
    *n += 1;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_num_children(GtkContainer *container)
{
    uint32_t n = 0;
    gtk_container_foreach(container, i_count, (gpointer)&n);
    return n;
}

#endif

/*---------------------------------------------------------------------------*/

static void i_remove_hotkey(HotKey *hotkey)
{
    listener_destroy(&hotkey->listener);
}

/*---------------------------------------------------------------------------*/

void oswindow_destroy(OSWindow **window)
{
    cassert_no_null(window);
    cassert_no_null(*window);
    cassert((*window)->menu == NULL);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_delete);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_config);
    g_signal_handler_disconnect(G_OBJECT((*window)->control.widget), (*window)->signal_key);
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
    arrpt_destroy(&(*window)->tabstops, NULL, OSControl);
    arrst_destopt(&(*window)->hotkeys, i_remove_hotkey, HotKey);
    cassert(i_num_children(GTK_CONTAINER((*window)->control.widget)) == 1);
    cassert(i_num_children(GTK_CONTAINER(gtk_bin_get_child(GTK_BIN((*window)->control.widget)))) == 0);
    g_object_unref((*window)->control.widget);
    heap_delete(window, OSWindow);
}

/*---------------------------------------------------------------------------*/

void oswindow_OnMoved(OSWindow *window, Listener *listener)
{
    unref(window);
    unref(listener);
    cassert(FALSE);
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
    gtk_window_set_title(GTK_WINDOW(window->control.widget), (const gchar*)text);
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
    if (window->hotkeys == NULL && listener != NULL)
        window->hotkeys = arrst_create(HotKey);

    /* Update the hotkey (if exists) */
    arrst_foreach(hotkey, window->hotkeys, HotKey)
        if (hotkey->key == key && hotkey->modifiers == modifiers)
        {
            listener_update(&hotkey->listener, listener);
            return;
        }
    arrst_end();

    /* Adds a new hotkey */
    if (listener != NULL)
    {
        HotKey *hotkey = arrst_new(window->hotkeys, HotKey);
        hotkey->key = key;
        hotkey->modifiers = modifiers;
        hotkey->listener = listener;
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    if (control != NULL)
    {
        cassert(control->type != ekGUI_TYPE_PANEL);
        if (control->type == ekGUI_TYPE_CUSTOMVIEW)
            g_object_set(G_OBJECT(_osview_focus((OSView*)control)), "can-focus", TRUE, NULL);

        arrpt_append(window->tabstops, control, OSControl);
    }
    else
    {
        arrpt_foreach(tabstop, window->tabstops, OSControl)
            if (tabstop->type == ekGUI_TYPE_CUSTOMVIEW)
                g_object_set(G_OBJECT(_osview_focus((OSView*)tabstop)), "can-focus", FALSE, NULL);
        arrpt_end();

        arrpt_clear(window->tabstops, NULL, OSControl);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_focus(OSWindow *window, OSControl *control)
{
    register GtkWidget *widget = i_focus_widget(control);
    cassert_no_null(widget);
    cassert_no_null(window);
    arrpt_foreach(tabstop, window->tabstops, OSControl)
        if (i_focus_widget(tabstop) == widget)
        {
            i_widget_grab_focus(widget, control, &window->ctabstop);
            break;
        }
    arrpt_end();
}

/*---------------------------------------------------------------------------*/

void oswindow_attach_panel(OSWindow *window, OSPanel *panel)
{
    GtkWidget *box;
    cassert_no_null(window);
    cassert(window->main_panel == NULL);
    box = gtk_bin_get_child(GTK_BIN(window->control.widget));
    gtk_box_pack_end(GTK_BOX(box), ((OSControl*)panel)->widget, TRUE, TRUE, 0);
    window->main_panel = panel;
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_panel(OSWindow *window, OSPanel *panel)
{
    GtkWidget *box;
    cassert_no_null(window);
    cassert(window->main_panel == panel);
    box = gtk_bin_get_child(GTK_BIN(window->control.widget));
    gtk_container_remove(GTK_CONTAINER(box), ((OSControl*)panel)->widget);
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

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    unref(parent_window);
    window->resize_event = FALSE;
    gtk_widget_show(window->control.widget);
    i_set_ctabstop(window->tabstops, &window->ctabstop);
}

/*---------------------------------------------------------------------------*/

void oswindow_hide(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    unref(parent_window);
    gtk_widget_hide(window->control.widget);
}

/*---------------------------------------------------------------------------*/

uint32_t oswindow_launch_modal(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    cassert(window->runloop == NULL);
    i_set_ctabstop(window->tabstops, &window->ctabstop);
    gtk_window_set_modal(GTK_WINDOW(window->control.widget), TRUE);
    window->resize_event = FALSE;
    window->runloop = g_main_loop_new(NULL, FALSE);

    if (parent_window != NULL)
        gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), GTK_WINDOW(parent_window->control.widget));

    gtk_widget_show(window->control.widget);
    g_main_loop_run(window->runloop);
    g_main_loop_unref(window->runloop);
    gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), NULL);

    if (parent_window != NULL)
        i_set_ctabstop(parent_window->tabstops, &parent_window->ctabstop);

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
    gtk_widget_hide(window->control.widget);
    g_main_loop_quit(window->runloop);
    gtk_window_set_modal(GTK_WINDOW(window->control.widget), FALSE);
}

/*---------------------------------------------------------------------------*/

void oswindow_get_origin(const OSWindow *window, real32_t *x, real32_t *y)
{
    gint wx, wy;
    cassert_no_null(window);
    cassert_no_null(x);
    cassert_no_null(y);
    gtk_window_get_position(GTK_WINDOW(window->control.widget), &wx, &wy);
    *x = (real32_t)wx;
    *y = (real32_t)wy;
}

/*---------------------------------------------------------------------------*/

void oswindow_origin(OSWindow *window, const real32_t x, const real32_t y)
{
    cassert_no_null(window);
    window->resize_event = FALSE;
    gtk_window_move(GTK_WINDOW(window->control.widget), (gint)x, (gint) y);
}

/*---------------------------------------------------------------------------*/

void oswindow_get_size(const OSWindow *window, real32_t *width, real32_t *height)
{
    gint w, h;
    cassert_no_null(window);
    cassert_no_null(width);
    cassert_no_null(height);
    gtk_window_get_size(GTK_WINDOW(window->control.widget), &w, &h);
    *width = (real32_t)w;
    *height = (real32_t)h;
}

/*---------------------------------------------------------------------------*/

static void i_update_menu_size(OSWindow *window)
{
    if (window->menu != NULL)
    {
        GtkRequisition ws, ms;
        GtkWidget *wmenu = _osmenu_widget(window->menu);
        gtk_widget_get_preferred_size(window->control.widget, &ws, NULL);
        gtk_widget_get_preferred_size(wmenu, &ms, NULL);
        if (window->is_resizable == TRUE)
        {
            gtk_widget_set_size_request(window->control.widget, -1, -1);
            gtk_window_resize(GTK_WINDOW(window->control.widget), MAX(ws.width, ms.width), ws.height + ms.height);
        }
        else
        {
            gtk_widget_set_size_request(window->control.widget, MAX(ws.width, ms.width), ws.height + ms.height);
        }
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_size(OSWindow *window, const real32_t width, const real32_t height)
{
    cassert_no_null(window);
    if (window->is_resizable == TRUE)
    {
        window->resize_event = FALSE;
        gtk_window_resize(GTK_WINDOW(window->control.widget), (gint)width, (gint)height);
        window->current_width = (gint)width;
        window->current_height = (gint)height;
        window->minimun_width = -1;
        window->minimun_height = -1;
    }
    else
    {
        gtk_widget_set_size_request(window->control.widget, (gint)width, (gint)height);
    }

    i_update_menu_size(window);
}

/*---------------------------------------------------------------------------*/

void oswindow_set_default_pushbutton(OSWindow *window, OSButton *button)
{
    cassert_no_null(window);
    window->defbutton = button;
}

/*---------------------------------------------------------------------------*/

void oswindow_set_cursor(OSWindow *window, Cursor *cursor)
{
    GdkWindow *gdkwindow = NULL;
    cassert_no_null(window);
    gdkwindow = gtk_widget_get_window(window->control.widget);
    gdk_window_set_cursor(gdkwindow, (GdkCursor*)cursor);
}

/*---------------------------------------------------------------------------*/

void oswindow_property(OSWindow *window, const gui_prop_t property, const void *value)
{
    cassert_no_null(window);
    unref(value);
    switch(property){
    case ekGUI_PROP_RESIZE:
        break;
    case ekGUI_PROP_CHILDREN:
        window->destroy_main_view = FALSE;
        break;
    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_menubar(OSWindow *window, OSMenu *menu)
{
    GtkWidget *box, *wmenu;
    cassert_no_null(window);
    cassert(window->menu == NULL);
    box = gtk_bin_get_child(GTK_BIN(window->control.widget));
    wmenu = _osmenu_widget(menu);
    gtk_box_pack_start(GTK_BOX(box), wmenu, FALSE, FALSE, 0);
    gtk_widget_show_all(wmenu);
    window->menu = menu;
    if (window->accel == NULL)
    {
        window->accel = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(window->control.widget), window->accel);
    }

    _osmenu_set_accel(window->menu, window->accel);
    i_update_menu_size(window);
}

/*---------------------------------------------------------------------------*/

void _oswindow_unset_menubar(OSWindow *window, OSMenu *menu)
{
    GtkWidget *box, *wmenu;
    cassert_no_null(window);
    cassert(window->menu == menu);
    cassert(window->accel != NULL);
    box = gtk_bin_get_child(GTK_BIN(window->control.widget));
    wmenu = _osmenu_widget(menu);
    _osmenu_unset_accel(menu, window->accel);
    g_object_ref(wmenu);
    gtk_container_remove(GTK_CONTAINER(box), wmenu);
    window->menu = NULL;
    i_update_menu_size(window);
}

/*---------------------------------------------------------------------------*/

void _oswindow_gtk_app(GtkApplication *app, GdkPixbuf *icon)
{
    cassert(i_GTK_APP == NULL);
    cassert(i_APP_ICON == NULL);
    i_GTK_APP = app;
    i_APP_ICON = icon;
}

/*---------------------------------------------------------------------------*/

void _oswindow_set_app_terminate(void)
{
    cassert(i_APP_TERMINATE == FALSE);
    i_APP_TERMINATE = TRUE;
}

/*---------------------------------------------------------------------------*/

void _oswindow_unset_focus(OSWindow *window)
{
    GtkWidget *focus_widget = NULL;
    OSControl **tabstop = NULL;
    uint32_t n = 0;
    uint32_t index = UINT32_MAX;
    cassert_no_null(window);
    focus_widget = gtk_window_get_focus(GTK_WINDOW(window->control.widget));
    tabstop = arrpt_all(window->tabstops, OSControl);
    n = arrpt_size(window->tabstops, OSControl);
    index = i_search_tabstop((const OSControl**)tabstop, n, focus_widget);
    if (index != UINT32_MAX)
        _oscontrol_unset_focus(tabstop[index]);
}
