/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.c
 *
 */

/* Operating System split view */

#include "ossplit.h"
#include "ospanel.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "oscontrol.inl"
#include "oslistener.inl"
//#include "oslabel.inl"
//#include "osbutton.inl"
//#include "ospopup.inl"
//#include "osedit.inl"
//#include "oscombo.inl"
//#include "osslider.inl"
//#include "osupdown.inl"
//#include "osprogress.inl"
//#include "ostext.inl"
//#include "osview.inl"
//#include "arrst.h"
#include "cassert.h"
#include "event.h"
#include "event.inl"
#include "heap.h"
#include "r2d.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _ossplit_t
{
    OSControl control;
    split_flag_t flags;
    R2Df divrect;
    ViewListeners listeners;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSSplit *view)
{
    cassert(FALSE);
    //if (view->listeners.is_dragging == FALSE)
    {
        if (r2d_containsf(&view->divrect, (real32_t)event->x, (real32_t)event->y) == TRUE)
        {
            if (split_type(view->flags) == ekSPHORZ)
                _osgui_ns_resize_cursor(widget);
            else
                _osgui_ew_resize_cursor(widget);
        }
        else
        {
            _osgui_default_cursor(widget);
        }
    }

    _oslistener_mouse_moved((OSControl*)view, event, NULL, NULL, &view->listeners);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    _oslistener_mouse_down((OSControl*)view, event, NULL, NULL, &view->listeners);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnRelease(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    _oslistener_mouse_up((OSControl*)view, event, NULL, NULL, &view->listeners);
    _osgui_default_cursor(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

//static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSPanel *panel)
//{
//    cassert_no_null(panel);
//    if (panel->areas == NULL)
//        return FALSE;
//
//    cairo_save(cr);
//    arrst_foreach(area, panel->areas, Area)
//        if (area->bgcolor != kCOLOR_NULL)
//        {
//            real32_t r, g, b, a;
//            color_get_rgbaf(area->bgcolor, &r, &g, &b, &a);
//            cairo_set_source_rgba(cr, (double)r, (double)g, (double)b, (double)a);
//            cairo_rectangle(cr, (double)area->x, (double)area->y, (double)area->w, (double)area->h);
//            cairo_fill(cr);
//        }
//
//        if (area->skcolor != kCOLOR_NULL)
//        {
//            real32_t r, g, b, a;
//            cairo_antialias_t ca;
//            color_get_rgbaf(area->skcolor, &r, &g, &b, &a);
//            cairo_set_source_rgba(cr, (double)r, (double)g, (double)b, (double)a);
//            cairo_set_line_width(cr, 1.);
//            ca = cairo_get_antialias(cr);
//            cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
//            cairo_rectangle(cr, (double)area->x + 1, (double)area->y + 1, (double)area->w - 1, (double)area->h - 1);
//            cairo_stroke(cr);
//            cairo_set_antialias(cr, ca);
//        }
//
//    arrst_end();
//    cairo_restore(cr);
//    return FALSE;
//}

/*---------------------------------------------------------------------------*/

OSSplit *ossplit_create(const split_flag_t flags)
{
    OSSplit *view = heap_new0(OSSplit);
    GtkWidget *widget = gtk_layout_new(NULL, NULL);
    view->flags = flags;
    _oscontrol_init(&view->control, ekGUI_COMPONENT_SPLITVIEW, widget, widget, TRUE);
    _oslistener_init(&view->listeners);
    _oslistener_signal(view->control.widget, TRUE, &view->listeners.moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &view->listeners.pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &view->listeners.release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    return view;
}
    
/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    _oslistener_remove(&(*view)->listeners);
    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSSplit);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    cassert(FALSE);
    _oscontrol_attach_to_parent(control, NULL/*(OSControl*)view*/);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach_control(OSSplit *view, OSControl *control)
{
    cassert(FALSE);
    _oscontrol_detach_from_parent(control, NULL/*(OSControl*)view*/);
}

/*---------------------------------------------------------------------------*/

void ossplit_OnMoved(OSSplit *view, Listener *listener)
{
    cassert_no_null(view);
//    listener_update(&view->listeners.OnMouseStartDrag, listener_copy(listener));
//    listener_update(&view->listeners.OnMouseDragging, listener_copy(listener));
//    listener_update(&view->listeners.OnMouseEndDrag, listener);
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
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach(OSSplit *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_visible(OSSplit *view, const bool_t is_visible)
{
    _oscontrol_set_visible((OSControl*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void ossplit_enabled(OSSplit *view, const bool_t is_enabled)
{   
    _oscontrol_set_enabled((OSControl*)view, is_enabled);
}

/*---------------------------------------------------------------------------*/

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
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
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
}

