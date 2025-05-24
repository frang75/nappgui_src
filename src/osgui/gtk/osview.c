/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.c
 *
 */

/* Operating System native custom view */

#include "oslistener.inl"
#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "osscroll_gtk.inl"
#include "ossplit_gtk.inl"
#include "osview_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osview.h"
#include "../osview.inl"
#include "../osgui.inl"
#include "../osscrolls.inl"
#include <draw2d/dctxh.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osview_t
{
    OSControl control;
    DCtx *ctx;
    uint32_t flags;
    GtkWidget *darea;
    OSScrolls *scroll;
    OSControl *capture;
    OSSplit *split;
    real32_t clip_width;
    real32_t clip_height;
    ViewListeners listeners;
    GtkCssProvider *css_bdcolor;
    bool_t allow_tab;
    Listener *OnFocus;
    Listener *OnResignFocus;
    Listener *OnAcceptFocus;
    Listener *OnOverlay;
};

/*---------------------------------------------------------------------------*/

static const gint i_FRAME_HPADDING = 2;
static const gint i_FRAME_VPADDING = 2;

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
            {
                /* dctx_update_view(view->ctx, cast(widget, void)); */
            }
        }
    }

    return TRUE;
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

        if (view->scroll != NULL)
        {
            params.x = (real32_t)_osscrolls_x_pos(view->scroll);
            params.y = (real32_t)_osscrolls_y_pos(view->scroll);
        }

        if (view->ctx == NULL)
            view->ctx = dctx_create();

        params.ctx = view->ctx;

        dctx_set_gcontext(view->ctx, cr, (uint32_t)view->clip_width, (uint32_t)view->clip_height, params.x, params.y, 0, TRUE);
        _oslistener_redraw(cast(view, OSControl), &params, &view->listeners);
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
        params.width = view->clip_width;
        params.height = view->clip_height;
        params.ctx = NULL;
        _oslistener_redraw(cast(view, OSControl), &params, &view->listeners);
    }

    /* Important! Return false for draw the scrollbars */
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
    _oslistener_redraw(cast(view, OSControl), &params, &view->listeners);
    return TRUE;
}

#endif

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSView *view)
{
    cassert_no_null(event);
    cassert_no_null(view);
    cassert_unref(widget == view->darea, widget);
    if (view->capture == NULL)
    {
        int w = gdk_window_get_width(event->window);
        int h = gdk_window_get_height(event->window);

        /* Scroll bars can send confusing move events
        Only we accept the motion over scroll window */
        if ((int)view->clip_width == w && (int)view->clip_height == h)
        {
            real32_t scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
            real32_t scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;
            _oslistener_mouse_moved(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
        }

        if (view->split != NULL)
            _ossplit_OnMove(view->split, event);
    }
    else
    {
        if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
            _ossplit_OnMove(cast(view->capture, OSSplit), event);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    cassert_no_null(event);
    cassert_no_null(view);
    cassert(event->type == GDK_ENTER_NOTIFY);
    unref(widget);
    if (event->mode == GDK_CROSSING_NORMAL)
    {
        real32_t scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
        real32_t scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;
        _oslistener_mouse_enter(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnExit(GtkWidget *widget, GdkEventCrossing *event, OSView *view)
{
    cassert_no_null(event);
    cassert_no_null(view);
    cassert(event->type == GDK_LEAVE_NOTIFY);
    unref(widget);
    if (event->mode == GDK_CROSSING_NORMAL)
        _oslistener_mouse_exit(cast(view, OSControl), event, &view->listeners);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(event);
    unref(widget);
    if (event->button == 1)
    {
        if (_oswindow_mouse_down(cast(view, OSControl)) == TRUE)
        {
            if (view->capture != NULL)
            {
                if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
                {
                    _ossplit_OnPress(cast(view->capture, OSSplit), event);
                    return FALSE;
                }
            }
        }
        else
        {
            return TRUE;
        }
    }

    {
        real32_t scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
        real32_t scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;
        _oslistener_mouse_down(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
    }

    /* Propagate the event */
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnRelease(GtkWidget *widget, GdkEventButton *event, OSView *view)
{
    cassert_no_null(view);
    unref(widget);
    if (view->capture == NULL)
    {
        real32_t scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
        real32_t scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;
        _oslistener_mouse_up(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
    }
    else
    {
        if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
            _ossplit_OnRelease(cast(view, OSSplit), event);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnWheel(GtkWidget *widget, GdkEventScroll *event, OSView *view)
{
    real32_t scroll_x = 0.f;
    real32_t scroll_y = 0.f;
    cassert_no_null(view);
    cassert_no_null(event);
    unref(widget);

    if (view->scroll != NULL)
    {
        gui_scroll_t ev = _osscroll_wheel_event(event);
        if (ev != ENUM_MAX(gui_scroll_t))
        {
            if (_osscrolls_event(view->scroll, ekGUI_VERTICAL, ev, FALSE) == TRUE)
                gtk_widget_queue_draw(view->darea);
        }
    }

    scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
    scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;
    _oslistener_scroll_whell(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(event);
    unref(widget);

    /* TAB Alt-TAB Navigation */
    if (view->allow_tab == FALSE && (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab))
        return FALSE;

    return (gboolean)_oslistener_key_down(cast(view, OSControl), event, &view->listeners);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyRelease(GtkWidget *widget, GdkEventKey *event, OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(event);
    unref(widget);

    /* TAB Alt-TAB Navigation */
    if (event->keyval == GDK_KEY_Tab || event->keyval == GDK_KEY_ISO_Left_Tab)
        return FALSE;

    return (gboolean)_oslistener_key_up(cast(view, OSControl), event, &view->listeners);
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
        if ((flags & ekVIEW_HSCROLL) || (flags & ekVIEW_VSCROLL))
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

    /* Creating the frame (border) view */
    if (flags & ekVIEW_BORDER)
    {
        GtkWidget *frame = gtk_frame_new(NULL);
        cassert(gtk_widget_get_has_window(frame) == FALSE);
        gtk_container_add(GTK_CONTAINER(frame), top);
        gtk_widget_show(top);

        {
            String *css = _osglobals_frame_focus_css();
            view->css_bdcolor = _oscontrol_css_provider(tc(css));
            str_destroy(&css);
        }

        top = frame;
    }

    cassert(area != NULL);
    view->flags = flags;
    view->darea = area;
    view->listeners.button = ENUM_MAX(gui_mouse_t);
    view->listeners.is_enabled = TRUE;
    view->allow_tab = FALSE;
    _oscontrol_init(&view->control, ekGUI_TYPE_CUSTOMVIEW, top, area, TRUE);

    if ((flags & ekVIEW_HSCROLL) || (flags & ekVIEW_VSCROLL))
    {
        view->scroll = _osscrolls_create(cast(view, OSControl), (bool_t)(flags & ekVIEW_HSCROLL) != 0, (bool_t)(flags & ekVIEW_VSCROLL) != 0);
        if (flags & ekVIEW_VSCROLL)
            g_signal_connect(area, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
    }

    gtk_widget_add_events(view->darea, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(view->darea, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    g_signal_connect(view->darea, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    _oslistener_signal(view->darea, TRUE, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
    gtk_widget_set_can_focus(view->darea, TRUE);
    gtk_widget_set_can_focus(top, TRUE);
    return view;
}

/*---------------------------------------------------------------------------*/

void osview_destroy(OSView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    (*view)->capture = NULL;
    (*view)->split = NULL;
    _oslistener_remove(&(*view)->listeners);
    listener_destroy(&(*view)->OnFocus);
    listener_destroy(&(*view)->OnResignFocus);
    listener_destroy(&(*view)->OnAcceptFocus);
    listener_destroy(&(*view)->OnOverlay);

    if ((*view)->ctx != NULL)
        dctx_destroy(&(*view)->ctx);

    if ((*view)->scroll != NULL)
        _osscrolls_destroy(&(*view)->scroll);

    _oscontrol_destroy_css_provider(&(*view)->css_bdcolor);
    _oscontrol_destroy(*dcast(view, OSControl));
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
    cassert_no_null(view);
    listener_update(&view->listeners.OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDown, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnUp, listener);
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
    cassert_no_null(view);
    listener_update(&view->listeners.OnDrag, listener);
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

void osview_OnResignFocus(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnResignFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnAcceptFocus(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnAcceptFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnScroll(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    _osscrolls_OnScroll(view->scroll, listener);
}

/*---------------------------------------------------------------------------*/

void osview_allow_key(OSView *view, const vkey_t key, const uint32_t value)
{
    cassert_no_null(view);
    cassert_unref(key == ekKEY_TAB, key);
    cassert(value == 0 || value == 1);
    view->allow_tab = (bool_t)value;
}

/*---------------------------------------------------------------------------*/

void osview_scroll(OSView *view, const real32_t x, const real32_t y)
{
    cassert_no_null(view);
    _osscrolls_set(view->scroll, x >= 0 ? (uint32_t)x : UINT32_MAX, y >= 0 ? (uint32_t)y : UINT32_MAX, FALSE);
}

/*---------------------------------------------------------------------------*/

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y)
{
    cassert_no_null(view);
    if (x != NULL)
        *x = (real32_t)_osscrolls_x_pos(view->scroll);

    if (y != NULL)
        *y = (real32_t)_osscrolls_y_pos(view->scroll);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_size(const OSView *view, real32_t *width, real32_t *height)
{
    cassert_no_null(view);
    if (width != NULL)
        *width = (real32_t)_osscrolls_bar_width(view->scroll, TRUE);

    if (height != NULL)
        *height = (real32_t)_osscrolls_bar_height(view->scroll, TRUE);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_visible(OSView *view, const bool_t horizontal, const bool_t vertical)
{
    cassert_no_null(view);
    _osscrolls_visible(view->scroll, horizontal, vertical);
}

/*---------------------------------------------------------------------------*/

void osview_content_size(OSView *view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    cassert_no_null(view);
    _osscrolls_content_size(view->scroll, (uint32_t)width, (uint32_t)height, (uint32_t)line_width, (uint32_t)line_height);
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
    _ospanel_attach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osview_detach(OSView *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osview_visible(OSView *view, const bool_t visible)
{
    _oscontrol_set_visible(cast(view, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osview_enabled(OSView *view, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(view, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osview_size(const OSView *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osview_origin(const OSView *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(view, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osview_frame(OSView *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    _oscontrol_set_frame(cast(view, OSControl), x, y, width, height);
    if (view->control.widget != view->darea)
    {
        cassert(GTK_IS_FRAME(view->control.widget) == TRUE);
        gtk_widget_set_size_request(view->darea, (gint)width - i_FRAME_HPADDING, (gint)height - i_FRAME_VPADDING);
        view->clip_width = width - i_FRAME_HPADDING;
        view->clip_height = height - i_FRAME_VPADDING;
    }
    else
    {
        view->clip_width = width;
        view->clip_height = height;
    }

    if (view->scroll != NULL)
        _osscrolls_control_size(view->scroll, (uint32_t)width, (uint32_t)height);
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

void _osview_set_parent_split(OSView *view, OSSplit *split)
{
    cassert_no_null(view);
    view->split = split;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osview_focus_widget(OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->darea);
    return view->darea;
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osview_area_widget(OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->darea);
    return view->darea;
}

/*---------------------------------------------------------------------------*/

void _osview_scroll_event(OSView *view, const gui_orient_t orient, const gui_scroll_t event)
{
    cassert_no_null(view);
    if (_osscrolls_event(view->scroll, orient, event, FALSE) == TRUE)
        gtk_widget_queue_draw(view->darea);
}

/*---------------------------------------------------------------------------*/

bool_t _osview_resign_focus(const OSView *view)
{
    bool_t resign = TRUE;
    cassert_no_null(view);
    if (view->OnResignFocus != NULL)
        listener_event(view->OnResignFocus, ekGUI_EVENT_FOCUS_RESIGN, view, NULL, &resign, OSView, void, bool_t);
    return resign;
}

/*---------------------------------------------------------------------------*/

bool_t _osview_accept_focus(const OSView *view)
{
    bool_t accept = TRUE;
    cassert_no_null(view);
    if (view->OnAcceptFocus != NULL)
        listener_event(view->OnAcceptFocus, ekGUI_EVENT_FOCUS_ACCEPT, view, NULL, &accept, OSView, void, bool_t);
    return accept;
}

/*---------------------------------------------------------------------------*/

void _osview_focus(OSView *view, const bool_t focus)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event(view->OnFocus, ekGUI_EVENT_FOCUS, view, &params, NULL, OSView, bool_t, void);
    }

    if (view->css_bdcolor != NULL)
    {
        cassert(GTK_IS_FRAME(view->control.widget));
        if (focus == TRUE)
            _oscontrol_add_css_provider(view->control.widget, view->css_bdcolor);
        else
            _oscontrol_remove_css_provider(view->control.widget, view->css_bdcolor);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _osview_capture_tab(const OSView *view)
{
    cassert_no_null(view);
    if (view->listeners.OnKeyDown == NULL)
        return FALSE;
    return view->allow_tab;
}
