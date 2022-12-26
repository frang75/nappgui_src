/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
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
#include "oscontrol.inl"
#include "oslistener.inl"
#include "ospanel.inl"
#include "ossplit.inl"
#include "cassert.h"
#include "dctxh.h"
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
    GtkWidget *darea;
    GtkWidget *hscroll;
    GtkWidget *vscroll;
    GtkAdjustment *hadjust;
    GtkAdjustment *vadjust;
    OSControl *capture;
    bool_t realized;
    real32_t area_width;
    real32_t area_height;
    real32_t clip_width;
    real32_t clip_height;
    ViewListeners listeners;
    Listener *OnFocus;
    Listener *OnNotify;
    Listener *OnOverlay;
};

/*---------------------------------------------------------------------------*/

static const gint i_BORDER_HPADDING = 1;
static const gint i_BORDER_VPADDING = 2;

/*---------------------------------------------------------------------------*/

static gboolean i_OnConfig(GtkWidget *widget, GdkEventConfigure *event, OSView *view)
{
    cassert(str_equ_c(G_OBJECT_TYPE_NAME(widget), "GtkLayout") || str_equ_c(G_OBJECT_TYPE_NAME(widget), "GtkDrawingArea"));
    unref(event);

    if ((view->flags & ekVIEW_OPENGL) == 0)
    {
        if (view->ctx != NULL)
        {
            uint32_t w = (uint32_t)gtk_widget_get_allocated_width(widget);
            uint32_t h = (uint32_t)gtk_widget_get_allocated_height(widget);
            if (w != view->clip_width || h != view->clip_height)
                dctx_update_view(view->ctx, (void*)widget);
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_scroller_position(OSView *view)
{
    if (view->clip_width > 0 && view->clip_height > 0)
    {
        if (view->hscroll != NULL)
        {
            gint scroll_height = (gint)osglobals_scroll_height();
            gtk_layout_move(GTK_LAYOUT(view->darea), view->hscroll, 0, (gint)view->clip_height - scroll_height);
            gtk_widget_set_size_request(view->hscroll, (gint)view->clip_width, scroll_height);
        }

        if (view->vscroll != NULL)
        {
            gint scroll_width = (gint)osglobals_scroll_width();
            gtk_layout_move(GTK_LAYOUT(view->darea), view->vscroll, (gint)view->clip_width - scroll_width, 0);
            gtk_widget_set_size_request(view->vscroll, scroll_width, (gint)view->clip_height);
        }
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSView *view)
{
    if (str_equ_c(gtk_widget_get_name(widget), "NAppGUICairoCtx") == TRUE)
    {
        EvDraw params;
        params.x = 0;
        params.y = 0;
        params.width = view->clip_width;
        params.height = view->clip_height;

        if (view->hadjust != NULL)
            params.x = (real32_t)(int)gtk_adjustment_get_value(view->hadjust);

        if (view->vadjust != NULL)
            params.y = (real32_t)(int)gtk_adjustment_get_value(view->vadjust);

        if (view->ctx == NULL)
            view->ctx = dctx_create();

        params.ctx = view->ctx;

        dctx_set_gcontext(view->ctx, cr, (uint32_t)view->clip_width, (uint32_t)view->clip_height, params.x, params.y, 0, TRUE);
        _oslistener_redraw((OSControl*)view, &params, &view->listeners);
        dctx_unset_gcontext(view->ctx);

        if (view->OnOverlay != NULL)
        {
            params.x = 0;
            params.y = 0;
            dctx_set_gcontext(view->ctx, cr, (uint32_t)view->clip_width, (uint32_t)view->clip_height, 0, 0, 0, TRUE);
            listener_event(view->OnOverlay, ekGUI_EVENT_OVERLAY, view, &params, NULL, OSView, EvDraw, void);
            dctx_unset_gcontext(view->ctx);
        }
    }
    else
    {
        EvDraw params;
        params.x = 0;
        params.y = 0;
        params.width = (real32_t)view->area_width;
        params.height = (real32_t)view->area_height;
        params.ctx = NULL;
        cassert(view->area_width == view->clip_width);
        cassert(view->area_height == view->clip_height);
        _oslistener_redraw((OSControl*)view, &params, &view->listeners);
    }

    /* Important! Draw the scrollbars */
    return FALSE;
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

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSView *view)
{
    int w, h;
    cassert_no_null(event);
    cassert_no_null(view);
    cassert_unref(widget == view->darea, widget);
    w = gdk_window_get_width(event->window);
    h = gdk_window_get_height(event->window);

    /* Scroll bars can send confusing move events
       Only we accept the motion over scroll window */
    if ((int)view->clip_width == w && (int)view->clip_height == h)
    {
        _oslistener_mouse_moved((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    cassert(event->type == GDK_ENTER_NOTIFY);
    if (event->mode == GDK_CROSSING_NORMAL)
        _oslistener_mouse_enter((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnExit(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    cassert(event->type == GDK_LEAVE_NOTIFY);
    if (event->mode == GDK_CROSSING_NORMAL)
        _oslistener_mouse_exit((OSControl*)view, event, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSView *view)
{
    if (view->capture != NULL)
    {
        if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
        {
            _ossplit_OnPress((OSSplit*)view->capture, event);
        }
    }
    else
    {
        gboolean can_focus = FALSE;

        g_object_get(G_OBJECT(widget), "can-focus", &can_focus, NULL);
        if (can_focus == TRUE)
            gtk_widget_grab_focus(widget);

        _oslistener_mouse_down((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    }

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
    cassert_no_null(view);
    unref(widget);
    if (view->vscroll != NULL)
    {
        if (gtk_widget_get_realized(view->vscroll) == TRUE)
            gtk_widget_event(view->vscroll, (GdkEvent*)event);
    }

    _oslistener_scroll_whell((OSControl*)view, event, view->hadjust, view->vadjust, &view->listeners);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    unref(widget);

    /* TAB Alt-TAB Navigation */
    if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab)
        return FALSE;

    return (gboolean)_oslistener_key_down((OSControl*)view, event, &view->listeners);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyRelease(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    unref(widget);

    /* TAB Alt-TAB Navigation */
    if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab)
        return FALSE;

    return (gboolean)_oslistener_key_up((OSControl*)view, event, &view->listeners);
}

/*---------------------------------------------------------------------------*/

static void i_OnScroll(GtkRange *range, OSView *view)
{
    unref(range);
    cassert_no_null(view);
    cassert_no_null(view->darea);
    gtk_widget_queue_draw(view->darea);
}

/*---------------------------------------------------------------------------*/

OSView *osview_create(const uint32_t flags)
{
    OSView *view = heap_new0(OSView);
    GtkWidget *area = NULL;
    GtkWidget *top = NULL;

    /* Creating a Cairo-based drawing area */
    if ((view->flags & ekVIEW_OPENGL) == 0)
    {
        /* GtkLayout --> Blank container (similar to GtkDrawingArea) but accepts children widgets */
        if (flags & ekVIEW_HSCROLL || flags & ekVIEW_VSCROLL)
        {
            area = gtk_layout_new(NULL, NULL);
        }
        else
        {
            area = gtk_drawing_area_new();
        }

        gtk_widget_set_name(area, "NAppGUICairoCtx");
        g_signal_connect(area, "configure-event", G_CALLBACK(i_OnConfig), (gpointer)view);
        g_signal_connect(area, "draw", G_CALLBACK(i_OnDraw), (gpointer)view);
    }
    /* Creating a OpenGL-based drawing area */
    else
    {
    #if GTK_CHECK_VERSION(3, 16, 0)
        area = gtk_gl_area_new();
        g_signal_connect(area, "render", G_CALLBACK(i_OnRender), (gpointer)view);

    #else
        cassert(FALSE);

    #endif
    }

    /* DrawingArea or Layout have their own GDK window for event listeners */
    cassert(gtk_widget_get_has_window(area) == TRUE);
    top = area;

    /* Horizontal scrollbar */
    if (flags & ekVIEW_HSCROLL)
    {
        view->hscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
        view->hadjust = gtk_range_get_adjustment(GTK_RANGE(view->hscroll));
        g_signal_connect(view->hscroll, "value-changed", G_CALLBACK(i_OnScroll), (gpointer)view);
        gtk_layout_put(GTK_LAYOUT(area), view->hscroll, 0, 0);
    }

    /* Vertical scrollbar */
    if (flags & ekVIEW_VSCROLL)
    {
        view->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
        view->vadjust = gtk_range_get_adjustment(GTK_RANGE(view->vscroll));
        g_signal_connect(view->vscroll, "value-changed", G_CALLBACK(i_OnScroll), (gpointer)view);
        gtk_layout_put(GTK_LAYOUT(area), view->vscroll, 0, 0);
        g_signal_connect(area, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
    }

    /* Creating the frame (border) view */
    if (flags & ekVIEW_BORDER)
    {
        GtkWidget *frame = gtk_frame_new(NULL);
        cassert(gtk_widget_get_has_window(frame) == FALSE);
        gtk_container_add(GTK_CONTAINER(frame), top);
        gtk_widget_show(top);
        top = frame;
    }

    cassert(area != NULL);
    view->flags = flags;
    view->darea = area;
    view->listeners.button = ENUM_MAX(gui_mouse_t);
    view->listeners.is_enabled = TRUE;

    _oscontrol_init(&view->control, ekGUI_TYPE_CUSTOMVIEW, top, area, TRUE);
    gtk_widget_show_all (top);
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
    listener_destroy(&(*view)->OnOverlay);

    if ((*view)->ctx != NULL)
        dctx_destroy(&(*view)->ctx);

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

void osview_OnOverlay(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnOverlay, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnEnter(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnEnter, listener);
    _oslistener_signal(view->darea, listener != NULL, &view->listeners.enter_signal, GDK_ENTER_NOTIFY_MASK, "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnExit(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnExit, listener);
    _oslistener_signal(view->darea, listener != NULL, &view->listeners.leave_signal, GDK_LEAVE_NOTIFY_MASK, "leave-notify-event", G_CALLBACK(i_OnExit), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnMoved(OSView *view, Listener *listener)
{
    bool_t add_signal;
    cassert_no_null(view);
    listener_update(&view->listeners.OnMoved, listener);
    add_signal = listener != NULL || view->listeners.OnDrag;
    _oslistener_signal(view->darea, add_signal, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDown, listener);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnUp, listener);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnClick != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnClick(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnClick, listener);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnDown != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->darea, listener != NULL || view->listeners.OnUp != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnDrag(OSView *view, Listener *listener)
{
    bool_t add_signal;
    cassert_no_null(view);
    listener_update(&view->listeners.OnDrag, listener);
    add_signal = listener != NULL || view->listeners.OnMoved;
    _oslistener_signal(view->darea, add_signal, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnWheel(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnWheel, listener);
    _oslistener_signal(view->darea, listener != NULL, &view->listeners.wheel_signal, GDK_SCROLL_MASK, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyDown, listener);
    _oslistener_signal(view->darea, listener != NULL, &view->listeners.keypressed_signal, 0, "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyUp, listener);
    _oslistener_signal(view->darea, listener != NULL, &view->listeners.keyrelease_signal, 0, "key-release-event", G_CALLBACK(i_OnKeyRelease), (gpointer)view);
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

void osview_scroll(OSView *view, const real32_t x, const real32_t y)
{
    cassert_no_null(view);

    if (x >= 0 && view->hadjust != NULL)
        gtk_adjustment_set_value(view->hadjust, (gdouble)x);

    if (y >= 0 && view->vadjust != NULL)
        gtk_adjustment_set_value(view->vadjust, (gdouble)y);
}

/*---------------------------------------------------------------------------*/

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y)
{
    cassert_no_null(view);

    if (x != NULL && view->hadjust != NULL)
        *x = (real32_t)gtk_adjustment_get_value(view->hadjust);

    if (y != NULL && view->vadjust != NULL)
        *y = (real32_t)gtk_adjustment_get_value(view->vadjust);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_size(const OSView *view, real32_t *width, real32_t *height)
{
    cassert_no_null(view);

    if (width != NULL)
    {
        if (view->vscroll != NULL)
            *width = (real32_t)osglobals_scroll_width();
        else
            *width = 0;
    }

    if (height != NULL)
    {
        if (view->hscroll != NULL)
            *height = (real32_t)osglobals_scroll_height();
        else
            *height = 0;
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_scroll(GtkAdjustment *adjust, GtkWidget *scroller, const real32_t view_size, const real32_t area_size)
{
    if (view_size > 0 && area_size > 0 && view_size < area_size)
    {
        gtk_adjustment_set_lower(adjust, 0);
        gtk_adjustment_set_upper(adjust, (gdouble)area_size);
        gtk_adjustment_set_page_size(adjust, (gdouble)view_size);
        gtk_widget_show(scroller);
    }
    /* Scrollbar is not necessary */
    else
    {
        gtk_widget_hide(scroller);
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_scrolls(OSView *view)
{
    cassert_no_null(view);
    if (view->hadjust != NULL)
        i_update_scroll(view->hadjust, view->hscroll, view->clip_width, view->area_width);

    if (view->vadjust != NULL)
        i_update_scroll(view->vadjust, view->vscroll, view->clip_height, view->area_height);
}

/*---------------------------------------------------------------------------*/

void osview_content_size(OSView *view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    cassert_no_null(view);
    unref(line_width);
    unref(line_height);
    if (GTK_IS_FRAME(view->control.widget) == TRUE)
    {
        view->area_width = width - i_BORDER_HPADDING;
        view->area_height = height - i_BORDER_VPADDING;
    }
    else
    {
        view->area_width = width;
        view->area_height = height;
    }
    i_update_scrolls(view);
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
    cassert_no_null(view->darea);
    gtk_widget_queue_draw(view->darea);
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
    cassert_no_null(view);
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
    if (view->control.widget != view->darea)
    {
        cassert(GTK_IS_FRAME(view->control.widget) == TRUE);
        gtk_widget_set_size_request(view->darea, (gint)width - 1, (gint)height - 2);
        view->clip_width = width - i_BORDER_HPADDING;
        view->clip_height = height - i_BORDER_VPADDING;
    }
    else
    {
        view->clip_width = width;
        view->clip_height = height;
    }

    i_update_scrolls(view);
    i_scroller_position(view);
}

/*---------------------------------------------------------------------------*/

void _osview_detach_and_destroy(OSView **view, OSPanel *panel)
{
    cassert_no_null(view);
    osview_detach(*view, panel);
    osview_destroy(view);
}

/*---------------------------------------------------------------------------*/

void _osview_set_capture(OSView *view, OSControl *control)
{
    cassert_no_null(view);
    view->capture = control;
}

/*---------------------------------------------------------------------------*/

void _osview_release_capture(OSView *view)
{
    cassert_no_null(view);
    view->capture = NULL;
}

/*---------------------------------------------------------------------------*/

void _osview_set_focus(OSView *view)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
    {
        bool_t params = TRUE;
        listener_event(view->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSView, bool_t, void);
    }
}

/*---------------------------------------------------------------------------*/

void _osview_unset_focus(OSView *view)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
    {
        bool_t params = FALSE;
        listener_event(view->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSView, bool_t, void);
    }
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osview_focus(OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->darea);
    return view->darea;
}
