/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.c
 *
 */

/* Operating System split view */

#include "oslistener.inl"
#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "ostext_gtk.inl"
#include "ossplit_gtk.inl"
#include "osview_gtk.inl"
#include "osweb_gtk.inl"
#include "../ossplit.h"
#include <geom2d/r2d.h>
#include <core/arrpt.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef struct _ossplittrack_t OSSplitTrack;

struct _ossplit_t
{
    OSControl control;
    split_flag_t flags;
    R2Df divrect;
    V2Df mouse_st;
    Listener *OnDrag;
};

struct _ossplittrack_t
{
    ArrPt(OSSplit) *splits;
    OSSplit *pressed;
    OSSplit *captured;
};

DeclPt(OSSplit);

/*---------------------------------------------------------------------------*/

static OSSplitTrack i_SPLIT_TRACKS = {NULL, NULL, NULL};

/*---------------------------------------------------------------------------*/

static void i_set_capture(GtkWidget *widget, OSSplit *split)
{
    if (GTK_IS_CONTAINER(widget) == TRUE && GTK_IS_SCROLLED_WINDOW(widget) == FALSE)
    {
        gtk_container_foreach(GTK_CONTAINER(widget), (GtkCallback)i_set_capture, split);
    }
    else
    {
        OSControl *control = cast(g_object_get_data(G_OBJECT(widget), "OSControl"), OSControl);
        if (control != NULL)
        {
            if (control->type == ekGUI_TYPE_PANEL)
                _ospanel_set_capture(cast(control, OSPanel), cast(split, OSControl));
            else if (control->type == ekGUI_TYPE_TEXTVIEW)
                _ostext_set_capture(cast(control, OSText), cast(split, OSControl));
            else if (control->type == ekGUI_TYPE_CUSTOMVIEW)
                _osview_set_capture(cast(control, OSView), cast(split, OSControl));
            else if (control->type == ekGUI_TYPE_WEBVIEW)
                _osweb_set_capture(cast(control, OSWeb), cast(split, OSControl));
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_release_capture(GtkWidget *widget, gpointer data)
{
    unref(data);
    if (GTK_IS_CONTAINER(widget) == TRUE && GTK_IS_SCROLLED_WINDOW(widget) == FALSE)
    {
        gtk_container_foreach(GTK_CONTAINER(widget), i_release_capture, NULL);
    }
    else
    {
        OSControl *control = cast(g_object_get_data(G_OBJECT(widget), "OSControl"), OSControl);
        if (control != NULL)
        {
            if (control->type == ekGUI_TYPE_PANEL)
                _ospanel_release_capture(cast(control, OSPanel));
            else if (control->type == ekGUI_TYPE_TEXTVIEW)
                _ostext_release_capture(cast(control, OSText));
            else if (control->type == ekGUI_TYPE_CUSTOMVIEW)
                _osview_release_capture(cast(control, OSView));
            else if (control->type == ekGUI_TYPE_WEBVIEW)
                _osweb_release_capture(cast(control, OSWeb));
        }
    }
}

/*---------------------------------------------------------------------------*/
/* https://stackoverflow.com/questions/63647507/gdkeventmotion-x-and-y-coordinates-appears-to-refer-to-location-within-a-differe */
static void i_mouse_pos(GtkWidget *widget, const gdouble x_root, const gdouble y_root, real32_t *x, real32_t *y)
{
    int mouse_x = 0;
    int mouse_y = 0;
    cassert_no_null(x);
    cassert_no_null(y);
    gdk_window_get_origin(gtk_widget_get_window(widget), &mouse_x, &mouse_y);
    *x = (real32_t)(x_root - mouse_x);
    *y = (real32_t)(y_root - mouse_y);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSSplit *view)
{
    cassert_no_null(view);
    cassert_unref(widget == view->control.widget, widget);
    _ossplit_OnMove(view, event);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    cassert_no_null(view);
    cassert_unref(view->control.widget == widget, widget);
    _ossplit_OnPress(view, event);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnRelease(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    cassert_no_null(view);
    cassert_unref(view->control.widget == widget, widget);
    _ossplit_OnRelease(view, event);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

OSSplit *ossplit_create(const uint32_t flags)
{
    OSSplit *view = heap_new0(OSSplit);
    GtkWidget *widget = gtk_layout_new(NULL, NULL);
    gint moved_signal = 0;
    gint pressed_signal = 0;
    gint release_signal = 0;
    view->flags = flags;
    _oscontrol_init(&view->control, ekGUI_TYPE_SPLITVIEW, widget, widget, TRUE);
    _oslistener_signal(view->control.widget, TRUE, &moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    arrpt_append(i_SPLIT_TRACKS.splits, view, OSSplit);
    return view;
}

/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);

    {
        uint32_t pos = arrpt_find(i_SPLIT_TRACKS.splits, *view, OSSplit);
        arrpt_delete(i_SPLIT_TRACKS.splits, pos, NULL, OSSplit);
        if (i_SPLIT_TRACKS.captured == *view)
            i_SPLIT_TRACKS.captured = NULL;
        if (i_SPLIT_TRACKS.pressed == *view)
            i_SPLIT_TRACKS.pressed = NULL;
    }

    listener_destroy(&(*view)->OnDrag);
    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSSplit);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(view);
    _oscontrol_attach_to_parent(control, view->control.widget);
    if (control->type == ekGUI_TYPE_CUSTOMVIEW)
        _osview_set_parent_split(cast(control, OSView), view);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(view);
    _oscontrol_detach_from_parent(control, view->control.widget);
}

/*---------------------------------------------------------------------------*/

void ossplit_OnDrag(OSSplit *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnDrag, listener);
}

/*---------------------------------------------------------------------------*/

void ossplit_track_area(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    view->divrect.pos.x = x;
    view->divrect.pos.y = y;
    view->divrect.size.width = width;
    view->divrect.size.height = height;
}

/*---------------------------------------------------------------------------*/

void ossplit_attach(OSSplit *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_detach(OSSplit *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_visible(OSSplit *view, const bool_t visible)
{
    _oscontrol_set_visible(cast(view, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ossplit_enabled(OSSplit *view, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(view, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ossplit_origin(const OSSplit *view, real32_t *x, real32_t *y)
{
    unref(view);
    unref(x);
    unref(y);
    cassert_msg(FALSE, "Not implemented");
}

/*---------------------------------------------------------------------------*/

void ossplit_frame(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(view, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ossplit_create_tracks(void)
{
    i_SPLIT_TRACKS.splits = arrpt_create(OSSplit);
    i_SPLIT_TRACKS.pressed = NULL;
    i_SPLIT_TRACKS.captured = NULL;
}

/*---------------------------------------------------------------------------*/

void _ossplit_destroy_tracks(void)
{
    arrpt_destroy(&i_SPLIT_TRACKS.splits, NULL, OSSplit);
    i_SPLIT_TRACKS.splits = NULL;
    i_SPLIT_TRACKS.pressed = NULL;
    i_SPLIT_TRACKS.captured = NULL;
}

/*---------------------------------------------------------------------------*/

void _ossplit_OnPress(OSSplit *view, GdkEventButton *event)
{
    unref(view);
    cassert_no_null(event);

    /* Left button */
    if (event->button == 1)
    {
        if (i_SPLIT_TRACKS.pressed == NULL && i_SPLIT_TRACKS.captured != NULL)
        {
            i_SPLIT_TRACKS.pressed = i_SPLIT_TRACKS.captured;
            i_mouse_pos(i_SPLIT_TRACKS.pressed->control.widget, event->x_root, event->y_root, &i_SPLIT_TRACKS.pressed->mouse_st.x, &i_SPLIT_TRACKS.pressed->mouse_st.y);
        }
    }
}

/*---------------------------------------------------------------------------*/

void _ossplit_OnRelease(OSSplit *view, GdkEventButton *event)
{
    unref(event);
    unref(view);
    if (i_SPLIT_TRACKS.pressed != NULL)
    {
        i_release_capture(i_SPLIT_TRACKS.pressed->control.widget, NULL);
        _osgui_default_cursor(i_SPLIT_TRACKS.pressed->control.widget);
        if (i_SPLIT_TRACKS.pressed->OnDrag != NULL)
        {
            EvMouse params;
            i_mouse_pos(i_SPLIT_TRACKS.pressed->control.widget, event->x_root, event->y_root, &params.x, &params.y);
            params.lx = i_SPLIT_TRACKS.pressed->mouse_st.x;
            params.ly = i_SPLIT_TRACKS.pressed->mouse_st.y;
            params.button = ekGUI_MOUSE_LEFT;
            params.count = 0;
            params.modifiers = 0;
            params.tag = 0;
            listener_event(i_SPLIT_TRACKS.pressed->OnDrag, ekGUI_EVENT_UP, i_SPLIT_TRACKS.pressed, &params, NULL, OSSplit, EvMouse, void);
        }

        i_SPLIT_TRACKS.pressed = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void _ossplit_OnMove(OSSplit *view, GdkEventMotion *event)
{
    cassert_no_null(event);
    unref(view);
    if (i_SPLIT_TRACKS.pressed != NULL)
    {
        if (i_SPLIT_TRACKS.pressed->OnDrag != NULL)
        {
            EvMouse params;
            i_mouse_pos(i_SPLIT_TRACKS.pressed->control.widget, event->x_root, event->y_root, &params.x, &params.y);
            params.lx = i_SPLIT_TRACKS.pressed->mouse_st.x;
            params.ly = i_SPLIT_TRACKS.pressed->mouse_st.y;
            params.button = ekGUI_MOUSE_LEFT;
            params.count = 0;
            params.modifiers = 0;
            params.tag = 0;
            listener_event(i_SPLIT_TRACKS.pressed->OnDrag, ekGUI_EVENT_DRAG, i_SPLIT_TRACKS.pressed, &params, NULL, OSSplit, EvMouse, void);
        }
    }
    else
    {
        OSSplit *inside = NULL;
        arrpt_foreach(split, i_SPLIT_TRACKS.splits, OSSplit)
            real32_t mouse_x = 0;
            real32_t mouse_y = 0;
            i_mouse_pos(split->control.widget, event->x_root, event->y_root, &mouse_x, &mouse_y);
            if (r2d_containsf(&split->divrect, mouse_x, mouse_y) == TRUE)
            {
                inside = split;
                break;
            }
        arrpt_end()

        if (inside != NULL)
        {
            if (i_SPLIT_TRACKS.captured != inside)
            {
                if (i_SPLIT_TRACKS.captured != NULL)
                    i_release_capture(i_SPLIT_TRACKS.captured->control.widget, NULL);

                i_SPLIT_TRACKS.captured = inside;
                i_set_capture(i_SPLIT_TRACKS.captured->control.widget, i_SPLIT_TRACKS.captured);
            }

            if (split_get_type(i_SPLIT_TRACKS.captured->flags) == ekSPLIT_HORZ)
                _osgui_ns_resize_cursor(view->control.widget);
            else
                _osgui_ew_resize_cursor(view->control.widget);
        }
        else
        {
            if (i_SPLIT_TRACKS.captured != NULL)
            {
                i_release_capture(i_SPLIT_TRACKS.captured->control.widget, NULL);
                i_SPLIT_TRACKS.captured = NULL;
            }

            _osgui_default_cursor(view->control.widget);
        }
    }
}
