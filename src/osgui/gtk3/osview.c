/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.c
 *
 */

/* Operating System native custom view */

#include "osview.h"
#include "osview.inl"
#include "osgui.inl"
#include "osglobals.inl"
#include "dctx.inl"
#include "dctx_gtk.inl"
#include "oscontrol.inl"
#include "oslistener.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "event.h"
#include "heap.h"
#include "strings.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osview_t
{
    OSControl control;
    DCtx *ctx;
    uint32_t flags;
    GtkWidget *area;
    GtkAdjustment *hadjust;
    GtkAdjustment *vadjust;
    real32_t area_width;
    real32_t area_height;
    real32_t clip_width;
    real32_t clip_height;
    ViewListeners listeners;
    Listener *OnFocus;
    Listener *OnNotify;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnConfig(GtkWidget *widget, GdkEventConfigure *event, OSView *view)
{
    //cassert(widget == view->control.widget);
    uint32_t w = (uint32_t)gtk_widget_get_allocated_width(widget);
    uint32_t h = (uint32_t)gtk_widget_get_allocated_height(widget);

    if ((view->flags & ekVOPENGL) == 0)
    {
        if (view->ctx != NULL)
        {
            if (view->area != NULL)
            {
                if (w != view->area_width || h != view->area_height)
                    dctx_update_view(view->ctx, (void*)widget);
            }
            else
            {
                if (w != view->clip_width || h != view->clip_height)
                    dctx_update_view(view->ctx, (void*)widget);
            }
        }
//        else
//        {
//            view->ctx = dctx_create_view((void*)widget);
//        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_area_expand(EvDraw *params, const real32_t area_width, const real32_t area_height)
{
    static const real32_t i_EXPAND_MARGIN = 200;

    params->x -= i_EXPAND_MARGIN;
    params->y -= i_EXPAND_MARGIN;
    params->width += 2 * i_EXPAND_MARGIN;
    params->height += 2 * i_EXPAND_MARGIN;

    if (params->x < 0)
        params->x = 0;

    if (params->y < 0)
        params->y = 0;

    if (params->x + params->width > area_width)
        params->width = area_width - params->x;

    if (params->y + params->height > area_height)
        params->height = area_height - params->y;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSView *view)
{
    EvDraw params;

    if (str_equ_c(gtk_widget_get_name(widget), "NAppGUICairoCtx") == TRUE)
    {
        if (view->area != NULL)
        {
            params.x = (real32_t)gtk_adjustment_get_value(view->hadjust);
            params.y = (real32_t)gtk_adjustment_get_value(view->vadjust);
            params.width = view->clip_width;
            params.height = view->clip_height;
            i_area_expand(&params, view->area_width, view->area_height);
        }
        else
        {
            params.x = 0;
            params.y = 0;
            params.width = (real32_t)view->clip_width;
            params.height = (real32_t)view->clip_height;
        }

        if (view->ctx == NULL)
            view->ctx = dctx_create(NULL);

        params.ctx = view->ctx;

        void *ctx[7];
        double scroll_x = (double)params.x;
        double scroll_y = (double)params.y;
        double total_width = (double)view->area_width;
        double total_height = (double)view->area_height;
        double clip_width = (double)params.width;
        double clip_height = (double)params.height;
        ctx[0] = cr;
        ctx[1] = &scroll_x;
        ctx[2] = &scroll_y;
        ctx[3] = &total_width;
        ctx[4] = &total_height;
        ctx[5] = &clip_width;
        ctx[6] = &clip_height;
        dctx_set_gcontext(view->ctx, (void*)ctx, (uint32_t)view->area_width, (uint32_t)view->area_height, 0, 0, 0, TRUE);
        _oslistener_redraw((OSControl*)view, &params, &view->listeners);
        dctx_unset_gcontext(view->ctx);
    }
    else
    {
       // cassert(view->ctx == NULL);
        params.x = 0;
        params.y = 0;
        params.width = (real32_t)view->area_width;
        params.height = (real32_t)view->area_height;
        cassert(view->area_width == view->clip_width);
        cassert(view->area_height == view->clip_height);
        _oslistener_redraw((OSControl*)view, &params, &view->listeners);
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(3, 16, 0)

static gboolean i_OnRender(GtkGLArea *widget, GdkGLContext *glctx, OSView *view)
{
    EvDraw params;
    cassert(GTK_WIDGET(widget) == view->control.widget);
    unref(glctx);
    params.ctx = NULL;
    params.x = 0;
    params.y = 0;
    params.width = (real32_t)gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    params.height = (real32_t)gtk_widget_get_allocated_height(GTK_WIDGET(widget));
    _oslistener_redraw((OSControl*)view, &params, &view->listeners);
    return TRUE;
}

#endif

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    _oslistener_mouse_enter((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSView *view)
{
    _oslistener_mouse_moved((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnExit(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    _oslistener_mouse_exit((OSControl*)view, event, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSView *view)
{
    gboolean can_focus = FALSE;

    cassert(widget == view->control.widget);
    g_object_get(G_OBJECT(widget), "can-focus", &can_focus, NULL);
    if (can_focus == TRUE)
        gtk_widget_grab_focus(widget);

    _oslistener_mouse_down((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnRelease(GtkWidget *widget, GdkEventButton *event, OSView *view)
{
    _oslistener_mouse_up((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnWheel(GtkWidget *widget, GdkEventScroll *event, OSView *view)
{
    _oslistener_scroll_whell((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    // TAB Alt-TAB Navigation
    if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab)
        return FALSE;

    return (gboolean)_oslistener_key_down((OSControl*)view, event, &view->listeners);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyRelease(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    // TAB Alt-TAB Navigation
    if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab)
        return FALSE;

    return (gboolean)_oslistener_key_up((OSControl*)view, event, &view->listeners);
}

///*---------------------------------------------------------------------------*/
//
//static void i_OnScrollMoved(GtkAdjustment *adjustment, OSView *view)
//{
//    cassert(view->area != NULL);
//    unref(adjustment);
//    gtk_widget_queue_draw(view->area);
//}

/*---------------------------------------------------------------------------*/

OSView *osview_create(const uint32_t flags)
{
    OSView *view = heap_new0(OSView);
    GtkWidget *widget = NULL;
    if ((view->flags & ekVOPENGL) == 0)
    {
        widget = gtk_drawing_area_new();
        gtk_widget_set_name(widget, "NAppGUICairoCtx");
        g_signal_connect(widget, "configure-event", G_CALLBACK(i_OnConfig), (gpointer)view);
        g_signal_connect(widget, "draw", G_CALLBACK(i_OnDraw), (gpointer)view);
    }
    else
    {
    #if GTK_CHECK_VERSION(3, 16, 0)
        widget = gtk_gl_area_new();
        g_signal_connect(widget, "render", G_CALLBACK(i_OnRender), (gpointer)view);
    #else
        cassert(FALSE);
    #endif
    }

    view->flags = flags;
    view->listeners.button = ENUM_MAX(mouse_t);
    view->listeners.is_enabled = TRUE;

    if (flags & ekHSCROLL || flags & ekVSCROLL)
    {
        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        GtkWidget *scrolled = NULL;

    #if GTK_CHECK_VERSION(3, 8, 0)
        scrolled = scroll;
    #else
        scrolled = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(scrolled), scroll);
        gtk_widget_show(scroll);
    #endif

        view->area = widget;
        gtk_widget_show(view->area);
        _oscontrol_init(&view->control, ekGUI_COMPONENT_CUSTOMVIEW, scrolled, scrolled, FALSE);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    #if GTK_CHECK_VERSION(3, 8, 0)
        gtk_container_add(GTK_CONTAINER(scroll), view->area);
    #else
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), view->area);
    #endif

        view->hadjust = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(scroll));
        view->vadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scroll));

        if (flags & ekBORDER)
            _oscontrol_widget_border(scroll, "scrolledwindow", osglobals_border_color(), NULL);
    }
    else
    {
        _oscontrol_init(&view->control, ekGUI_COMPONENT_CUSTOMVIEW, widget, widget, TRUE);

        if (flags & ekBORDER)
            _oscontrol_widget_border(view->control.widget, "drawing_area", osglobals_border_color(), NULL);
    }

    //gtk_widget_set_events(view->control.widget, GDK_POINTER_MOTION_MASK);


    return view;
}

/*---------------------------------------------------------------------------*/

void osview_destroy(OSView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    _oslistener_remove(&(*view)->listeners);
    listener_destroy(&(*view)->OnFocus);
    listener_destroy(&(*view)->OnNotify);

    if ((*view)->ctx != NULL)
        dctx_destroy(&(*view)->ctx);

    if ((*view)->area != NULL)
    {
        // The object is unref when removed
        //g_object_ref((*view)->area);
        gtk_container_remove(GTK_CONTAINER((*view)->control.widget), (*view)->area);
    }

    _oscontrol_destroy(*(OSControl**)view);
    heap_delete(view, OSView);
}

/*---------------------------------------------------------------------------*/

void *osview_native(OSView *view)
{
    cassert_no_null(view);
    return view->control.widget;
}

/*---------------------------------------------------------------------------*/

void osview_OnDraw(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDraw, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnEnter(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnEnter, listener);
    _oslistener_signal(view->control.widget, listener != NULL, &view->listeners.enter_signal, GDK_ENTER_NOTIFY_MASK, "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnExit(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnExit, listener);
    _oslistener_signal(view->control.widget, listener != NULL, &view->listeners.leave_signal, GDK_LEAVE_NOTIFY_MASK, "leave-notify-event", G_CALLBACK(i_OnExit), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnMoved(OSView *view, Listener *listener)
{
    bool_t add_signal;
    cassert_no_null(view);
    listener_update(&view->listeners.OnMoved, listener);
    add_signal = listener != NULL || view->listeners.OnDrag;
    _oslistener_signal(view->control.widget, add_signal, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDown, listener);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnUp, listener);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnClick(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnClick, listener);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnDown != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, listener != NULL || view->listeners.OnUp != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnDrag(OSView *view, Listener *listener)
{
    bool_t add_signal;
    cassert_no_null(view);
    listener_update(&view->listeners.OnDrag, listener);
    add_signal = listener != NULL || view->listeners.OnMoved;
    _oslistener_signal(view->control.widget, add_signal, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnWheel(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnWheel, listener);
    _oslistener_signal(view->control.widget, listener != NULL, &view->listeners.wheel_signal, GDK_SCROLL_MASK, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyDown, listener);
    _oslistener_signal(view->control.widget, listener != NULL, &view->listeners.keypressed_signal, 0, "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyUp, listener);
    _oslistener_signal(view->control.widget, listener != NULL, &view->listeners.keyrelease_signal, 0, "key-release-event", G_CALLBACK(i_OnKeyRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnFocus(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnNotify(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnNotify, listener);
}

/*---------------------------------------------------------------------------*/

#if defined __ASSERTS__

/*---------------------------------------------------------------------------*/

static bool_t i_IS_SCROLLED_WINDOW(GtkWidget *widget)
{
#if GTK_CHECK_VERSION(3, 8, 0)
    return GTK_IS_SCROLLED_WINDOW(widget);
#else

    if (GTK_IS_EVENT_BOX(widget) == TRUE)
    {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
        return GTK_IS_SCROLLED_WINDOW(child);
    }

    return FALSE;
#endif
}

/*---------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------------*/

void osview_scroll(OSView *view, const real32_t x, const real32_t y)
{
    cassert_no_null(view);
    cassert(view->area != NULL);
    cassert(i_IS_SCROLLED_WINDOW(view->control.widget) == TRUE);
    cassert(view->hadjust != NULL);
    cassert(view->vadjust != NULL);

    if (x >= 0)
        gtk_adjustment_set_value(view->hadjust, (gdouble)x);

    if (y >= 0)
        gtk_adjustment_set_value(view->vadjust, (gdouble)y);
}

/*---------------------------------------------------------------------------*/

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y)
{
    cassert_no_null(view);
    cassert(view->area != NULL);
    cassert(i_IS_SCROLLED_WINDOW(view->control.widget) == TRUE);
    cassert(view->hadjust != NULL);
    cassert(view->vadjust != NULL);

    if (x != NULL)
        *x = (real32_t)gtk_adjustment_get_value(view->hadjust);

    if (y != NULL)
        *y = (real32_t)gtk_adjustment_get_value(view->vadjust);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_size(const OSView *view, real32_t *width, real32_t *height)
{
    cassert_no_null(view);
    cassert(view->area != NULL);
    cassert(i_IS_SCROLLED_WINDOW(view->control.widget) == TRUE);
    // In GTK scrollbars are overlapping
    if (width != NULL)
        *width = 0;

    if (height != NULL)
        *height = 0;
}

/*---------------------------------------------------------------------------*/

void osview_content_size(OSView *view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    gint w, h;
    cassert_no_null(view);
    cassert(view->area != NULL);
    cassert(i_IS_SCROLLED_WINDOW(view->control.widget) == TRUE);
    unref(line_width);
    unref(line_height);

    w = (gint)width;
    h = (gint)height;

    if (view->flags & ekBORDER)
    {
        w -= 2;
        h -= 2;
    }

    gtk_widget_set_size_request(view->area, w, h);
    view->area_width = width;
    view->area_height = height;
}

/*---------------------------------------------------------------------------*/

real32_t osview_scale_factor(const OSView *view)
{
    unref(view);
    return 1;
}

/*---------------------------------------------------------------------------*/

void osview_set_need_display(OSView *view)
{
    cassert_no_null(view);
    if (view->area != NULL)
        gtk_widget_queue_draw(view->area);
    else
        gtk_widget_queue_draw(view->control.widget);
}

/*---------------------------------------------------------------------------*/

void *osview_get_native_view(const OSView *view)
{
    cassert_no_null(view);
    return view->control.widget;
}

/*---------------------------------------------------------------------------*/

void osview_attach(OSView *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void osview_detach(OSView *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void osview_visible(OSView *view, const bool_t is_visible)
{
    _oscontrol_set_visible((OSControl*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void osview_enabled(OSView *view, const bool_t is_enabled)
{
    _oscontrol_set_enabled((OSControl*)view, is_enabled);
}

/*---------------------------------------------------------------------------*/

void osview_size(const OSView *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void osview_origin(const OSView *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void osview_frame(OSView *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
    view->clip_width = width;
    view->clip_height = height;
}

/*---------------------------------------------------------------------------*/

void _osview_detach_and_destroy(OSView **view, OSPanel *panel)
{
    cassert_no_null(view);
    osview_detach(*view, panel);
    osview_destroy(view);
}

/*---------------------------------------------------------------------------*/

void _osview_set_focus(OSView *view)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
    {
        bool_t params = TRUE;
        listener_event(view->OnFocus, ekEVFOCUS, view, &params, NULL, OSView, bool_t, void);
    }
}

/*---------------------------------------------------------------------------*/

void _osview_unset_focus(OSView *view)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
    {
        bool_t params = FALSE;
        listener_event(view->OnFocus, ekEVFOCUS, view, &params, NULL, OSView, bool_t, void);
    }
}

