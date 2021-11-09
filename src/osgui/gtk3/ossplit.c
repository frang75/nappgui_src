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
#include "ossplit.inl"
#include "ospanel.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "oscontrol.inl"
#include "cassert.h"
#include "event.h"
#include "event.inl"
#include "oslistener.inl"
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
    bool_t left_button;
    bool_t inside_rect;
    Listener *OnDrag;
};

/*---------------------------------------------------------------------------*/

static void i_set_capture(GtkWidget *widget, OSSplit *split)
{
    if (GTK_IS_CONTAINER(widget) == TRUE && GTK_IS_SCROLLED_WINDOW(widget) == FALSE)
    {
        gtk_container_foreach(GTK_CONTAINER(widget), (GtkCallback)i_set_capture, split);
    }
//    else if (GTK_IS_SCROLLED_WINDOW(widget) == TRUE)
//    {
//        GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
//        OSControl *control = (OSControl*)g_object_get_data(G_OBJECT(child), "OSControl");
//        if (control->type == ekGUI_COMPONENT_PANEL)
//             {
//                 bstd_printf("Capture PANEL in SCROLL\n");
//             }
//             else if (control->type == ekGUI_COMPONENT_CUSTOMVIEW)
//             {
//                 bstd_printf("Capture VIEW in SCROLL\n");
//
//             }
//             else if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//            {
//                bstd_printf("Capture TEXT in SCROLL\n");
//            }
//
//    }
    else
    {
        OSControl *control = (OSControl*)g_object_get_data(G_OBJECT(widget), "OSControl");
        if (control->type == ekGUI_COMPONENT_PANEL)
        {
            _ospanel_set_capture((OSPanel*)control, (OSControl*)split);
            //bstd_printf("Capture PANEL in SCROLL\n");
        }
//        else if (control->type == ekGUI_COMPONENT_CUSTOMVIEW)
//        {
//        bstd_printf("Capture VIEW in SCROLL\n");
//
//        }
//        else if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//        {
//        bstd_printf("Capture TEXT in SCROLL\n");
//        }
//        if (control != NULL)
//        {
//            if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//            {
//                bstd_printf("Capture TEXT\n");
//            }
//        }
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
//    else if (GTK_IS_SCROLLED_WINDOW(widget) == TRUE)
//    {
//        GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
//        OSControl *control = (OSControl*)g_object_get_data(G_OBJECT(child), "OSControl");
//        if (control->type == ekGUI_COMPONENT_PANEL)
//             {
//                 bstd_printf("Capture PANEL in SCROLL\n");
//             }
//             else if (control->type == ekGUI_COMPONENT_CUSTOMVIEW)
//             {
//                 bstd_printf("Capture VIEW in SCROLL\n");
//
//             }
//             else if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//            {
//                bstd_printf("Capture TEXT in SCROLL\n");
//            }
//
//    }
    else
    {
        OSControl *control = (OSControl*)g_object_get_data(G_OBJECT(widget), "OSControl");
        if (control->type == ekGUI_COMPONENT_PANEL)
        {
            _ospanel_release_capture((OSPanel*)control);
            //bstd_printf("Capture PANEL in SCROLL\n");
        }
//        else if (control->type == ekGUI_COMPONENT_CUSTOMVIEW)
//        {
//        bstd_printf("Capture VIEW in SCROLL\n");
//
//        }
//        else if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//        {
//        bstd_printf("Capture TEXT in SCROLL\n");
//        }
//        if (control != NULL)
//        {
//            if (control->type == ekGUI_COMPONENT_TEXTVIEW)
//            {
//                bstd_printf("Capture TEXT\n");
//            }
//        }
    }

}

/*---------------------------------------------------------------------------*/

static gboolean i_OnMove(GtkWidget *widget, GdkEventMotion *event, OSSplit *view)
{
    cassert(widget == view->control.widget);
//    if (event->type == GDK_MOTION_NOTIFY)
//    {
//        bstd_printf("GDK_MOTION_NOTIFY\n");
//    }
//    else if (event->type == GDK_DRAG_MOTION)
//    {
//        bstd_printf("GDK_DRAG_MOTION\n");
//    }
//    else
//    {
//        bstd_printf("UNKNOWN_MOTION\n");
//    }

    if (view->left_button == TRUE)
    {
        // bstd_printf("Dragging!!!!!!!!!!!!!!!!!!\n");

    }
    else
    {
        int mouse_x = 0;
        int mouse_y = 0;
//        int delta_x;
//        int delta_y;
        // https://stackoverflow.com/questions/63647507/gdkeventmotion-x-and-y-coordinates-appears-to-refer-to-location-within-a-differe
        //bstd_printf("MOVING\n");

        gdk_window_get_origin(gtk_widget_get_window(widget)/*event->window*/, &mouse_x, &mouse_y);
        mouse_x = event->x_root - mouse_x;
        mouse_y = event->y_root - mouse_y;
//        bstd_printf("Move (%.2f, %.2f)\n", (real32_t)mouse_x, (real32_t)mouse_y);
//        bstd_printf("%x DivRect Pos:(%.2f, %.2f) Size:(%.2f, %.2f)\n", view, view->divrect.pos.x, view->divrect.pos.y, view->divrect.size.width, view->divrect.size.height);

        if (r2d_containsf(&view->divrect, (real32_t)mouse_x, (real32_t)mouse_y) == TRUE)
        //if (r2d_containsf(&view->divrect, (real32_t)event->x, (real32_t)event->y) == TRUE)
        {
            //bstd_printf("INSIDE!!!!\n");

            if (view->inside_rect == FALSE)
            {
                i_set_capture(widget, view);
                view->inside_rect = TRUE;
            }

//            GtkWidget *grab = gtk_grab_get_current();
//            bstd_printf("Grab %x\n", grab);
//
//            if (gtk_widget_has_grab(widget) == FALSE)
//                gtk_grab_add(widget);
//
//            cassert(gtk_widget_is_sensitive(widget) == TRUE);
//            cassert(gtk_widget_has_grab(widget) == TRUE);

            if (split_type(view->flags) == ekSPHORZ)
                _osgui_ns_resize_cursor(widget);
            else
                _osgui_ew_resize_cursor(widget);

        }
        else
        {
            if (view->inside_rect == TRUE)
            {
                i_release_capture(widget, NULL);
                view->inside_rect = FALSE;
            }
            _osgui_default_cursor(widget);
        }
    }

    // Manually compute correct coordinates for the event
//    int screen_x;
//    int screen_y;
//    int delta_x;
//    int delta_y;
//    GdkWindow *window = gtk_widget_get_window(widget);
//    gdk_window_get_origin(gtk_widget_get_window(widget)/*event->window*/, &screen_x, &screen_y);
//    delta_x = event->x_root - screen_x;
//    delta_y = event->y_root - screen_y;
//
//    bstd_printf("GDKWindow: Widget:%x Event:%x\n", window, event->window);
//    bstd_printf("Window: X:%d Y:%d\n", screen_x, screen_y);
//    bstd_printf("Delta: X:%d Y:%d\n", delta_x, delta_y);
//    bstd_printf("%x DivRect Pos:(%.2f, %.2f) Size:(%.2f, %.2f) Mouse (%.2f, %.2f)\n", view, view->divrect.pos.x, view->divrect.pos.y, view->divrect.size.width, view->divrect.size.height, (real32_t)event->x, (real32_t)event->y);

    //_oslistener_mouse_moved((OSControl*)view, event, NULL, NULL, &view->listeners);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    _ossplit_OnPress(view, event);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnRelease(GtkWidget *widget, GdkEventButton *event, OSSplit *view)
{
    view->left_button = FALSE;
    //bstd_printf("RELEASED!!!!\n");
    if (view->inside_rect == TRUE)
    {
        i_release_capture(widget, NULL);
        view->inside_rect = FALSE;
    }

    //_oslistener_mouse_up((OSControl*)view, event, NULL, NULL, &view->listeners);
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
    gint moved_signal = 0;
    gint pressed_signal = 0;
    gint release_signal = 0;
    view->flags = flags;
    _oscontrol_init(&view->control, ekGUI_COMPONENT_SPLITVIEW, widget, widget, TRUE);
    _oslistener_signal(view->control.widget, TRUE, &moved_signal, GDK_POINTER_MOTION_MASK, "motion-notify-event", G_CALLBACK(i_OnMove), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &pressed_signal, GDK_BUTTON_PRESS_MASK, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)view);
    _oslistener_signal(view->control.widget, TRUE, &release_signal, GDK_BUTTON_RELEASE_MASK, "button-release-event", G_CALLBACK(i_OnRelease), (gpointer)view);
    return view;
}
    
/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    listener_destroy(&(*view)->OnDrag);
    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSSplit);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(view);
    _oscontrol_attach_to_parent(control, view->control.widget);
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

/*---------------------------------------------------------------------------*/

void _ossplit_OnPress(OSSplit *view, GdkEventButton *event)
{
    cassert_no_null(view);
    cassert_no_null(event);

    // Left button
    if (event->button == 1)
    {
        if (view->inside_rect == TRUE)
        {
            //bstd_printf("PRESSED!!!!!!!!!!!!!!\n");
            view->left_button = TRUE;
        }
    }
}
