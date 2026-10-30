/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
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

#if GTK_CHECK_VERSION(3, 16, 0)

typedef void *(*FPtr_osview_gl_upgrade)(void *view);

typedef struct _osview_glhook_t
{
    FPtr_osview_gl_upgrade func_upgrade;
    void *view;
} OSViewGLHook;

#endif

struct _osview_t
{
    OSControl control;
    DCtx *ctx;
    uint32_t flags;
    GtkWidget *darea;
    OSScrolls *scroll;
    OSControl *capture;
    real32_t clip_width;
    real32_t clip_height;
    ViewListeners listeners;
    GtkCssProvider *css_noborder;
    bool_t has_focus;
    bool_t allow_tab;
    Listener *OnFocus;
    Listener *OnResignFocus;
    Listener *OnAcceptFocus;
    Listener *OnOverlay;
#if GTK_CHECK_VERSION(3, 16, 0)
    OSViewGLHook glhook;
    GtkWidget *glarea;
#endif
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnConfig(GtkWidget *widget, GdkEventConfigure *event, OSView *view)
{
    cassert(str_equ_c(G_OBJECT_TYPE_NAME(widget), "GtkLayout") || str_equ_c(G_OBJECT_TYPE_NAME(widget), "GtkDrawingArea"));
    unref(event);

    if ((view->flags & ekVIEW_OPENGL) == 0)
    {
        uint32_t w = (uint32_t)gtk_widget_get_allocated_width(widget);
        uint32_t h = (uint32_t)gtk_widget_get_allocated_height(widget);
        if ((real32_t)w != view->clip_width || (real32_t)h != view->clip_height)
        {
            view->clip_width = (real32_t)w;
            view->clip_height = (real32_t)h;
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

        {
            bool_t clipped = FALSE;
            if (view->scroll != NULL)
            {
                uint32_t avail_width = 0, avail_height = 0;
                _osscrolls_avail_size(view->scroll, &avail_width, &avail_height);
                if ((real32_t)avail_width < view->clip_width || (real32_t)avail_height < view->clip_height)
                {
                    cairo_save(cr);
                    cairo_rectangle(cr, 0, 0, (double)avail_width, (double)avail_height);
                    cairo_clip(cr);
                    clipped = TRUE;
                }
            }

            dctx_set_gcontext(view->ctx, cr, view->clip_width, view->clip_height, 96, 1.f, params.x, params.y, 0, (view->flags & ekVIEW_CONTROL) ? FALSE : TRUE);
            _oslistener_redraw(cast(view, OSControl), &params, &view->listeners);
            dctx_unset_gcontext(view->ctx);

            if (clipped == TRUE)
                cairo_restore(cr);
        }

        if (view->OnOverlay != NULL)
        {
            params.x = 0;
            params.y = 0;
            dctx_set_gcontext(view->ctx, cr, view->clip_width, view->clip_height, 96, 1.f, 0, 0, 0, (view->flags & ekVIEW_CONTROL) ? FALSE : TRUE);
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
    cassert(GTK_WIDGET(widget) == view->glarea);
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
        gpointer owner = NULL;
        gdk_window_get_user_data(event->window, &owner);

        /* Scroll bars can send confusing move events
        Only we accept the motion over scroll window */
        if (cast(owner, GtkWidget) == view->darea)
        {
            int w = gtk_widget_get_allocated_width(view->darea);
            int h = gtk_widget_get_allocated_height(view->darea);
            real32_t scroll_x = view->scroll ? (real32_t)_osscrolls_x_pos(view->scroll) : 0;
            real32_t scroll_y = view->scroll ? (real32_t)_osscrolls_y_pos(view->scroll) : 0;

            if ((real32_t)w != view->clip_width || (real32_t)h != view->clip_height)
            {
                view->clip_width = (real32_t)w;
                view->clip_height = (real32_t)h;
            }

            if (view->scroll != NULL)
                _osscrolls_activity(view->scroll);

            _oslistener_mouse_moved(cast(view, OSControl), event, scroll_x, scroll_y, &view->listeners);
        }
    }

    _ossplit_OnMove(view->darea, event);
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
        gui_orient_t orient = ekGUI_VERTICAL;
        gui_scroll_t ev = _osscroll_wheel_event(event, &orient);
        _osscrolls_activity(view->scroll);
        if (ev != ENUM_MAX(gui_scroll_t))
        {
            if (_osscrolls_event(view->scroll, orient, ev, FALSE) == TRUE)
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

static void i_wire_common_signals(GtkWidget *area, GtkWidget *top, OSView *view)
{
    gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(area, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    g_signal_connect(area, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    _oslistener_signal(area, TRUE, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
    gtk_widget_set_can_focus(area, TRUE);
    gtk_widget_set_can_focus(top, TRUE);
}

/*---------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(3, 16, 0)

static void i_wire_dynamic_signals(GtkWidget *area, OSView *view)
{
    view->listeners.enter_signal = 0;
    view->listeners.leave_signal = 0;
    view->listeners.pressed_signal = 0;
    view->listeners.release_signal = 0;
    view->listeners.wheel_signal = 0;
    view->listeners.keypressed_signal = 0;
    view->listeners.keyrelease_signal = 0;

    _oslistener_signal(area, view->listeners.OnEnter != NULL, &view->listeners.enter_signal, GDK_ENTER_NOTIFY_MASK, "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnExit != NULL, &view->listeners.leave_signal, GDK_LEAVE_NOTIFY_MASK, "leave-notify-event", G_CALLBACK(i_OnExit), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnClick != NULL || view->listeners.OnDown != NULL, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnClick != NULL || view->listeners.OnUp != NULL, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnWheel != NULL, &view->listeners.wheel_signal, GDK_SCROLL_MASK, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnKeyDown != NULL, &view->listeners.keypressed_signal, 0, "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)view);
    _oslistener_signal(area, view->listeners.OnKeyUp != NULL, &view->listeners.keyrelease_signal, 0, "key-release-event", G_CALLBACK(i_OnKeyRelease), (gpointer)view);
}

/*---------------------------------------------------------------------------*/

static void *i_upgrade_to_opengl(void *view_ptr)
{
    OSView *view = cast(view_ptr, OSView);
    GtkWidget *parent = NULL;
    GtkWidget *eventbox = NULL;
    GtkWidget *new_area = NULL;
    gint x = 0, y = 0, width = 0, height = 0;
    gboolean was_visible = FALSE;
    cassert_no_null(view);
    cassert_msg((view->flags & (ekVIEW_BORDER | ekVIEW_HSCROLL | ekVIEW_VSCROLL)) == 0, "OpenGL views cannot use ekVIEW_BORDER/HSCROLL/VSCROLL");
    cassert(view->control.widget == view->darea);
    parent = gtk_widget_get_parent(view->darea);
    cassert_no_null(parent);
    cassert(GTK_IS_LAYOUT(parent) == TRUE);

    gtk_container_child_get(GTK_CONTAINER(parent), view->darea, "x", &x, "y", &y, NULL);
    gtk_widget_get_size_request(view->darea, &width, &height);
    was_visible = gtk_widget_get_visible(view->darea);
    gtk_widget_destroy(view->darea);
    g_object_unref(view->control.widget);
    eventbox = gtk_event_box_new();
    gtk_widget_set_name(eventbox, "NAppGUIOpenGLCtx");
    new_area = gtk_gl_area_new();
    g_signal_connect(new_area, "render", G_CALLBACK(i_OnRender), (gpointer)view);
    gtk_container_add(GTK_CONTAINER(eventbox), new_area);
    g_object_set_data(G_OBJECT(eventbox), "nappgui-oglview-hook", &view->glhook);
    gtk_layout_put(GTK_LAYOUT(parent), eventbox, x, y);
    gtk_widget_set_size_request(eventbox, width, height);

    view->darea = eventbox;
    view->glarea = new_area;
    view->clip_width = (real32_t)width;
    view->clip_height = (real32_t)height;
    view->flags |= ekVIEW_OPENGL;
    _oscontrol_init(&view->control, ekGUI_TYPE_CUSTOMVIEW, eventbox, eventbox, FALSE);
    i_wire_common_signals(eventbox, eventbox, view);
    i_wire_dynamic_signals(eventbox, view);

    if (was_visible)
        gtk_widget_show(eventbox);

    return new_area;
}

#endif

/*---------------------------------------------------------------------------*/

static gboolean i_OnFrameDraw(GtkWidget *widget, cairo_t *cr, OSView *view)
{
    cassert_no_null(view);
    if (view->has_focus == TRUE)
    {
        GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
        int w = gtk_widget_get_allocated_width(widget);
        int h = gtk_widget_get_allocated_height(widget);
        gtk_render_focus(ctx, cr, 0, 0, w, h);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSView *osview_create(const uint32_t flags)
{
    OSView *view = heap_new0(OSView);
    GtkWidget *area = NULL;
    GtkWidget *top = NULL;

    if ((flags & ekVIEW_HSCROLL) || (flags & ekVIEW_VSCROLL))
    {
        /* GtkLayout --> Blank container (similar to GtkDrawingArea) but accepts children widgets */
        area = gtk_layout_new(NULL, NULL);
    }
    else
    {
        area = gtk_drawing_area_new();
    }

    gtk_widget_set_name(area, "NAppGUICairoCtx");
    g_signal_connect(area, "configure-event", G_CALLBACK(i_OnConfig), (gpointer)view);
    g_signal_connect(area, "draw", G_CALLBACK(i_OnDraw), (gpointer)view);

    /* DrawingArea or Layout have their own GDK window for event listeners */
    cassert(gtk_widget_get_has_window(area) == TRUE);
    top = area;

    /* Creating the frame (border) view */
    if (flags & ekVIEW_BORDER)
    {
        GtkWidget *frame = gtk_frame_new(NULL);
        String *css = _osglobals_frame_hide_css();
        cassert(gtk_widget_get_has_window(frame) == FALSE);
        gtk_container_add(GTK_CONTAINER(frame), top);
        gtk_widget_show(top);
        g_signal_connect_after(frame, "draw", G_CALLBACK(i_OnFrameDraw), (gpointer)view);
        view->css_noborder = _oscontrol_css_provider(tc(css));
        str_destroy(&css);
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
        g_signal_connect(area, "scroll-event", G_CALLBACK(i_OnWheel), (gpointer)view);
    }

    i_wire_common_signals(view->darea, top, view);

#if GTK_CHECK_VERSION(3, 16, 0)
    view->glhook.func_upgrade = i_upgrade_to_opengl;
    view->glhook.view = view;
    g_object_set_data(G_OBJECT(top), "nappgui-oglview-hook", &view->glhook);
#endif

    return view;
}

/*---------------------------------------------------------------------------*/

void osview_destroy(OSView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    (*view)->capture = NULL;
    _oslistener_remove(&(*view)->listeners);
    listener_destroy(&(*view)->OnFocus);
    listener_destroy(&(*view)->OnResignFocus);
    listener_destroy(&(*view)->OnAcceptFocus);
    listener_destroy(&(*view)->OnOverlay);

    if ((*view)->ctx != NULL)
        dctx_destroy(&(*view)->ctx);

    if ((*view)->scroll != NULL)
        _osscrolls_destroy(&(*view)->scroll);

    _oscontrol_destroy_css_provider(&(*view)->css_noborder);
    _oscontrol_destroy(*dcast(view, OSControl));
    heap_delete(view, OSView);
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
    if (view->scroll == NULL)
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

void osview_tooltip(OSView *view, const char_t *text)
{
    cassert_no_null(view);
    gtk_widget_set_tooltip_text(view->control.widget, cast_const(text, gchar));
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
        *width = (real32_t)_osscrolls_bar_reserve_width(view->scroll);

    if (height != NULL)
        *height = (real32_t)_osscrolls_bar_reserve_height(view->scroll);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_visible(OSView *view, const bool_t horizontal, const bool_t vertical)
{
    cassert_no_null(view);
    _osscrolls_visible(view->scroll, horizontal, vertical);
}

/*---------------------------------------------------------------------------*/

bool_t osview_scroll_transition(OSView *view, const real64_t prtime, const real64_t crtime)
{
    cassert_no_null(view);
    if (view->scroll != NULL)
        return _osscrolls_transition(view->scroll, prtime, crtime);
    return FALSE;
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
    cassert_no_null(view);
#if GTK_CHECK_VERSION(3, 10, 0)
    /* GTK3 only supports integer scales. With fractional desktop scaling (eg. 125%)
       the compositor rounds up (x2) and downsamples, so GL framebuffers are x2 too */
    return (real32_t)gtk_widget_get_scale_factor(view->control.widget);
#else
    unref(view);
    return 1;
#endif
}

/*---------------------------------------------------------------------------*/

void osview_set_need_display(OSView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->darea);

#if GTK_CHECK_VERSION(3, 16, 0)
    if ((view->flags & ekVIEW_OPENGL) != 0)
    {
        gtk_gl_area_queue_render(GTK_GL_AREA(view->glarea));
        return;
    }
#endif

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
        gtk_widget_set_size_request(view->darea, (gint)width, (gint)height);
    }

    view->clip_width = width;
    view->clip_height = height;

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

void _osview_scrolls_activity(OSView *view)
{
    cassert_no_null(view);
    if (view->scroll != NULL)
        _osscrolls_activity(view->scroll);
}

/*---------------------------------------------------------------------------*/

void _osview_scrolls_resync(OSView *view)
{
    cassert_no_null(view);
    if (view->scroll != NULL)
        _osscrolls_resync(view->scroll);
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

    if (view->flags & ekVIEW_BORDER)
    {
        cassert(GTK_IS_FRAME(view->control.widget));
        view->has_focus = focus;

        if (focus == TRUE)
            _oscontrol_add_css_provider(view->control.widget, view->css_noborder);
        else
            _oscontrol_remove_css_provider(view->control.widget, view->css_noborder);

        gtk_widget_queue_draw(view->control.widget);
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
