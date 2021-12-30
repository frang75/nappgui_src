/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
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
#include "arrpt.h"
#include "cassert.h"
#include "event.h"
#include "heap.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _oswindow_t
{
    OSControl control;
    OSMenu *menu;
    GMainLoop *runloop;
    uint32_t modal_return;
    GtkAccelGroup *accel;
    OSPanel *main_panel;
    gint signal_delete;
    gint signal_config;
    gint signal_key;
    Listener *OnMoved;
    Listener *OnResize;
    Listener *OnClose;
    ArrPt(OSControl) *tabstops;
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

/*---------------------------------------------------------------------------*/

static gboolean i_OnClose(GtkWidget *widget, GdkEvent *event, OSWindow *window)
{
    bool_t closed = TRUE;
    cassert_no_null(window);
    cassert_unref(window->control.widget == widget, widget);

    if (window->OnClose != NULL)
    {
        EvWinClose params;
        params.origin = ekCLBUTTON;
        listener_event(window->OnClose, ekEVWNDCLOSE, window, &params, &closed, OSWindow, EvWinClose, bool_t);
    }

    if (closed == TRUE)
        gtk_widget_hide(widget);

    return TRUE;
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
            listener_event(window->OnResize, ekEVWNDSIZING, window, &params, &result, OSWindow, EvSize, EvSize);
            listener_event(window->OnResize, ekEVWNDSIZE, window, &result, NULL, OSWindow, EvSize, void);

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
    case ekGUI_COMPONENT_LABEL:
    case ekGUI_COMPONENT_PROGRESS:
        return NULL;

    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_CUSTOMVIEW:
        return control->widget;

    case ekGUI_COMPONENT_EDITBOX:
        return _osedit_focus((OSEdit*)control);

    case ekGUI_COMPONENT_BUTTON:
        return _osbutton_focus((OSButton*)control);

    case ekGUI_COMPONENT_POPUP:
        return _ospopup_focus((OSPopUp*)control);

    case ekGUI_COMPONENT_COMBOBOX:
        return _oscombo_focus((OSCombo*)control);

    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
    case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
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

static void i_set_tabstop(GtkWindow *window, const OSControl **tabstop, const uint32_t size, const uint32_t index, const bool_t reverse)
{
    register uint32_t idx = index, i;
    cassert(index < size);
    for (i = 0 ; i < size; ++i)
    {
        register const OSControl *control = tabstop[idx];
        register GtkWidget *widget = i_focus_widget(control);
        if (widget && gtk_widget_is_sensitive(widget) && gtk_widget_get_visible(widget))
        {
            if (control->type == ekGUI_COMPONENT_EDITBOX && _osedit_autoselect((OSEdit*)control) == FALSE)
            {
                if (GTK_IS_ENTRY(widget) == TRUE)
                {
                #if GTK_CHECK_VERSION(3, 16, 0)
                    gtk_entry_grab_focus_without_selecting(GTK_ENTRY(widget));
                #else
                    gtk_widget_grab_focus(widget);
                #endif
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

static void i_set_next_tabstop(GtkWindow *window, const ArrPt(OSControl) *tabstops, GtkWidget *widget)
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
        i_set_tabstop(window, tabstop, size, tabindex, FALSE);
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_previous_tabstop(GtkWindow *window, const ArrPt(OSControl) *tabstops, GtkWidget *widget)
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
        i_set_tabstop(window, tabstop, size, tabindex, TRUE);
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSWindow *window)
{
    switch (event->keyval) {
    case GDK_KEY_Tab:
    {
        GtkWidget *tabstop = gtk_window_get_focus(GTK_WINDOW(widget));
        i_set_next_tabstop(GTK_WINDOW(widget), window->tabstops, tabstop);
        return TRUE;
    }

    /* https://mail.gnome.org/archives/gtk-list/1999-August/msg00127.html */
    case GDK_KEY_ISO_Left_Tab:
    {
        GtkWidget *tabstop = gtk_window_get_focus(GTK_WINDOW(widget));
        i_set_previous_tabstop(GTK_WINDOW(widget), window->tabstops, tabstop);
        return TRUE;
    }

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
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSWindow *oswindow_create(const window_flag_t flags)
{
    OSWindow *window = heap_new0(OSWindow);
    GtkWidget *widget = gtk_application_window_new(i_GTK_APP);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(widget), box);
    _oscontrol_init((OSControl*)window, ekGUI_COMPONENT_WINDOW, widget, widget, FALSE);
	window->destroy_main_view = TRUE;
	window->tabstops = arrpt_create(OSControl);
    window->is_resizable = (flags & ekWNRES) == ekWNRES ? TRUE : FALSE;
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
//	cassert_no_null(window);
//    cassert(window->state != i_ekSTATE_MANAGED);
//    listener_remove_optional(&window->OnMoved);
//    window->OnMoved = ptr_assign_struct_no_null(listener, Listener);
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
//    HWND parent = NULL;
	cassert_no_null(window);
	unref(window);
    unref(below_window);
    cassert(FALSE);
//    cassert(window->state != i_ekSTATE_MANAGED);

//    if (below_window != NULL)
//        parent = below_window->control.hwnd;
//
//    SetWindowLongPtr(window->control.hwnd, GWLP_HWNDPARENT, (LONG)parent);
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

void oswindow_taborder(OSWindow *window, OSControl *control)
{
    cassert_no_null(window);
    if (control != NULL)
    {
        cassert(control->type != ekGUI_COMPONENT_PANEL);
        if (control->type == ekGUI_COMPONENT_CUSTOMVIEW)
            g_object_set(G_OBJECT(control->widget), "can-focus", TRUE, NULL);
        arrpt_append(window->tabstops, control, OSControl);
    }
    else
    {
        arrpt_foreach(tabstop, window->tabstops, OSControl)
            if (tabstop->type == ekGUI_COMPONENT_CUSTOMVIEW)
                g_object_set(G_OBJECT(tabstop->widget), "can-focus", FALSE, NULL);
        arrpt_end();

        arrpt_clear(window->tabstops, NULL, OSControl);
    }
}

/*---------------------------------------------------------------------------*/

void oswindow_focus(OSWindow *window, OSControl *control)
{
    unref(window);
    cassert_no_null(control);
    gtk_widget_grab_focus(control->widget);
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
    // OJO!!!!!!!
//    void
//    gtk_window_set_transient_for (GtkWindow *window,
//                                  GtkWindow *parent);



    //HWND prevParent = 0;
    unref(parent_window);
	unref(child_window);
    cassert(FALSE);
    /*prevParent = SetParent(child_window->control.hwnd, parent_window->control.hwnd);
    unreferenced(prevParent);
    prevParent = GetParent(child_window->control.hwnd);*/
    //SetWindowLong(child_window->control.hwnd, GWL_STYLE, child_window->dwStyle | WS_CHILD);
    //SetWindowLong(child_window->control.hwnd, GWL_EXSTYLE, child_window->dwExStyle);
    //oswindow_set_z_order(child_window, parent_window);
}

/*---------------------------------------------------------------------------*/

void oswindow_detach_window(OSWindow *parent_window, OSWindow *child_window)
{
    //HWND prevParent = 0;
    unref(parent_window);
    unref(child_window);
    cassert(FALSE);
   /* cassert_no_null(parent_window);
	cassert_no_null(child_window);
    prevParent = SetParent(child_window->control.hwnd, GetDesktopWindow());
    cassert(prevParent == parent_window->control.hwnd);*/
    /*SetWindowLong(child_window->control.hwnd, GWL_STYLE, child_window->dwStyle);
    SetWindowLong(child_window->control.hwnd, GWL_EXSTYLE, child_window->dwExStyle);*/
}

/*---------------------------------------------------------------------------*/

void oswindow_launch(OSWindow *window, OSWindow *parent_window)
{
    cassert_no_null(window);
    unref(parent_window);
    window->resize_event = FALSE;
    gtk_widget_show(window->control.widget);
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
    gtk_window_set_modal(GTK_WINDOW(window->control.widget), TRUE);
    window->resize_event = FALSE;
    window->runloop = g_main_loop_new(NULL, FALSE);

    if (parent_window != NULL)
        gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), GTK_WINDOW(parent_window->control.widget));

    gtk_widget_show(window->control.widget);
    g_main_loop_run(window->runloop);
    g_main_loop_unref(window->runloop);
    gtk_window_set_transient_for(GTK_WINDOW(window->control.widget), NULL);
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
//    gtk_widget_set_can_default(((OSControl*)button)->widget, TRUE);
//    gtk_window_set_default(GTK_WINDOW(window->control.widget), ((OSControl*)button)->widget);
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

void oswindow_property(OSWindow *window, const guiprop_t property, const void *value)
{
    cassert_no_null(window);
    unref(value);
    switch(property){
    case ekGUI_PROPERTY_RESIZE:
        break;
    case ekGUI_PROPERTY_CHILDREN:
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
