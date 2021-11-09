/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ospanel.c
 *
 */

/* Operating System native view */

#include "ospanel.h"
#include "ospanel.inl"
#include "ossplit.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "oscontrol.inl"
#include "oslabel.inl"
#include "osbutton.inl"
#include "ospopup.inl"
#include "osedit.inl"
#include "oscombo.inl"
#include "osslider.inl"
#include "osupdown.inl"
#include "osprogress.inl"
#include "ostext.inl"
#include "osview.inl"
#include "arrst.h"
#include "cassert.h"
#include "color.h"
#include "heap.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef struct _area_t Area;

struct _area_t
{
    void *obj;
    real32_t x;
    real32_t y;
    real32_t w;
    real32_t h;
    color_t bgcolor;
    color_t skcolor;
};

struct _ospanel_t
{
    OSControl control;
    OSControl *capture;
    GtkWidget *content;
    GtkAdjustment *hadjust;
    GtkAdjustment *vadjust;
    ArrSt(Area) *areas;
};

DeclSt(Area);
#define i_red(rgba)     (((double)((uint8_t)((rgba) >>  0))) / 255.)
#define i_green(rgba)   (((double)((uint8_t)((rgba) >>  8))) / 255.)
#define i_blue(rgba)    (((double)((uint8_t)((rgba) >> 16))) / 255.)
#define i_alpha(rgba)   (((double)((uint8_t)((rgba) >> 24))) / 255.)

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSPanel *panel)
{
    cassert_no_null(panel);
    if (panel->areas == NULL)
        return FALSE;

    cairo_save(cr);

    if (panel->content != NULL)
    {
        gdouble x = gtk_adjustment_get_value(panel->hadjust);
        gdouble y = gtk_adjustment_get_value(panel->vadjust);
        cairo_translate(cr, - (double)x, - (double)y);
    }

    arrst_foreach(area, panel->areas, Area)
        if (area->bgcolor != kCOLOR_TRANSPARENT)
        {
            real32_t r, g, b, a;
            color_get_rgbaf(area->bgcolor, &r, &g, &b, &a);
            cairo_set_source_rgba(cr, (double)r, (double)g, (double)b, (double)a);
            cairo_rectangle(cr, (double)area->x, (double)area->y, (double)area->w, (double)area->h);
            cairo_fill(cr);
        }

        if (area->skcolor != kCOLOR_TRANSPARENT)
        {
            real32_t r, g, b, a;
            cairo_antialias_t ca;
            color_get_rgbaf(area->skcolor, &r, &g, &b, &a);
            cairo_set_source_rgba(cr, (double)r, (double)g, (double)b, (double)a);
            cairo_set_line_width(cr, 1.);
            ca = cairo_get_antialias(cr);
            cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
            cairo_rectangle(cr, (double)area->x + 1, (double)area->y + 1, (double)area->w - 1, (double)area->h - 1);
            cairo_stroke(cr);
            cairo_set_antialias(cr, ca);
        }

    arrst_end();
    cairo_restore(cr);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSPanel *panel)
{
    if (panel->capture != NULL)
    {
        if (panel->capture->type == ekGUI_COMPONENT_SPLITVIEW)
        {
            _ossplit_OnPress((OSSplit*)panel->capture, event);

        }
//    // Left button
//    if (event->button == 1)
//    {
//        if (view->inside_rect == TRUE)
//        {
//            gtk_grab_add(widget);
//            bstd_printf("PRESSED!!!!!!!!!!!!!!\n");
//            view->left_button = TRUE;
//            return FALSE;
//        }
//    }

        return TRUE;
    }
    // The handler will be called before the default handler of the signal.
    // This is the default behaviour
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

OSPanel *ospanel_create(const uint32_t flags)
{
    OSPanel *panel = heap_new0(OSPanel);
    GtkWidget *widget = gtk_layout_new(NULL, NULL);
    g_signal_connect(widget, "draw", G_CALLBACK(i_OnDraw), (gpointer)panel);

    if (flags & ekHSCROLL || flags & ekVSCROLL)
    {
        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        panel->content = widget;
        gtk_widget_show(panel->content);
        _oscontrol_init(&panel->control, ekGUI_COMPONENT_PANEL, scroll, scroll, FALSE);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        gtk_container_add(GTK_CONTAINER(panel->control.widget), panel->content);

        // A parent widget can "capture" the mouse
        {
            GtkWidget *vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(panel->control.widget));
            GtkWidget *hscroll = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(panel->control.widget));
            g_signal_connect(G_OBJECT(vscroll), "button-press-event", G_CALLBACK(i_OnPressed), panel);
            g_signal_connect(G_OBJECT(hscroll), "button-press-event", G_CALLBACK(i_OnPressed), panel);
        }

        panel->hadjust = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(panel->control.widget));
        panel->vadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(panel->control.widget));
    }
    else
    {
        _oscontrol_init(&panel->control, ekGUI_COMPONENT_PANEL, widget, widget, TRUE);
    }

    return panel;
}
    
/*---------------------------------------------------------------------------*/

void ospanel_destroy(OSPanel **panel)
{
    cassert_no_null(panel);
    cassert_no_null(*panel);

    if ((*panel)->areas != NULL)
        arrst_destroy(&(*panel)->areas, NULL, Area);

    if ((*panel)->content != NULL)
    {
        // The object is unref when removed
        //g_object_ref((*view)->area);
        gtk_container_remove(GTK_CONTAINER((*panel)->control.widget), (*panel)->content);
    }

    _oscontrol_destroy(*(OSControl**)panel);
    heap_delete(panel, OSPanel);
}

/*---------------------------------------------------------------------------*/

void ospanel_area(OSPanel *panel, void *obj, const color_t bgcolor, const color_t skcolor, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(panel);
    if (obj != NULL)
    {
        Area *area = NULL;

        if (panel->areas == NULL)
            panel->areas = arrst_create(Area);

        arrst_foreach(larea, panel->areas, Area)
            if (larea->obj == obj)
            {
                area = larea;
                break;
            }
        arrst_end();

        if (area == NULL)
        {
            area = arrst_new(panel->areas, Area);
            area->obj = obj;
        }

        area->x = x;
        area->y = y;
        area->w = width;
        area->h = height;
        area->bgcolor = bgcolor;
        area->skcolor = skcolor;
    }
    else
    {
        if (panel->areas != NULL)
            arrst_clear(panel->areas, NULL, Area);
    }
}

/*---------------------------------------------------------------------------*/

#if defined __ASSERTS__

/*---------------------------------------------------------------------------*/

// Working os SplitView WIP
//static bool_t i_IS_SCROLLED_WINDOW(GtkWidget *widget)
//{
//#if GTK_CHECK_VERSION(3, 8, 0)
//    return GTK_IS_SCROLLED_WINDOW(widget);
//#else
//
//    if (GTK_IS_EVENT_BOX(widget) == TRUE)
//    {
//        GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
//        return GTK_IS_SCROLLED_WINDOW(child);
//    }
//
//    return FALSE;
//#endif
//}

/*---------------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------------*/

void ospanel_scroller_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    cassert_no_null(panel);

    // In GTK scrollbars are overlapping
    if (width != NULL)
        *width = 0;

    if (height != NULL)
        *height = 0;
}

/*---------------------------------------------------------------------------*/

void ospanel_content_size(OSPanel *panel, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    cassert_no_null(panel);
    cassert(panel->content != NULL);
    cassert(GTK_IS_SCROLLED_WINDOW(panel->control.widget) == TRUE);
    gtk_layout_set_size(GTK_LAYOUT(panel->content), (guint)width, (guint)height);
}

/*---------------------------------------------------------------------------*/

void ospanel_display(OSPanel *panel)
{
    cassert_no_null(panel);
    gtk_widget_queue_draw(panel->control.widget);
}

/*---------------------------------------------------------------------------*/

void ospanel_attach(OSPanel *panel, OSPanel *parent_panel)
{
    _ospanel_attach_control(parent_panel, (OSControl*)panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_detach(OSPanel *panel, OSPanel *parent_panel)
{
    _ospanel_detach_control(parent_panel, (OSControl*)panel);
}

/*---------------------------------------------------------------------------*/

void ospanel_visible(OSPanel *panel, const bool_t is_visible)
{
    _oscontrol_set_visible((OSControl*)panel, is_visible);
}

/*---------------------------------------------------------------------------*/

void ospanel_enabled(OSPanel *panel, const bool_t is_enabled)
{   
    _oscontrol_set_enabled((OSControl*)panel, is_enabled);
}

/*---------------------------------------------------------------------------*/

void ospanel_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)panel, width, height);
}

/*---------------------------------------------------------------------------*/

void ospanel_origin(const OSPanel *panel, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)panel, x, y);
}

/*---------------------------------------------------------------------------*/

void ospanel_frame(OSPanel *panel, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)panel, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void ospanel_position(OSPanel *panel, const real32_t x, const real32_t y)
{
    unref(panel);
    unref(x);
    unref(y);
    cassert_msg(FALSE, "Not implemented");
}

/*---------------------------------------------------------------------------*/

static void i_destroy_child(GtkWidget *widget, gpointer data)
{
    OSPanel *panel = (OSPanel*)data;
    OSControl *control = (OSControl*)g_object_get_data(G_OBJECT(widget), "OSControl");
    cassert_no_null(control);
    switch (control->type) {
    case ekGUI_COMPONENT_LABEL:
        _oslabel_detach_and_destroy((OSLabel**)&control, panel);
        break;
    case ekGUI_COMPONENT_BUTTON:
        _osbutton_detach_and_destroy((OSButton**)&control, panel);
        break;
    case ekGUI_COMPONENT_POPUP:
        _ospopup_detach_and_destroy((OSPopUp**)&control, panel);
        break;
    case ekGUI_COMPONENT_EDITBOX:
        _osedit_detach_and_destroy((OSEdit**)&control, panel);
        break;
    case ekGUI_COMPONENT_COMBOBOX:
        _oscombo_detach_and_destroy((OSCombo**)&control, panel);
        break;
    case ekGUI_COMPONENT_SLIDER:
        _osslider_detach_and_destroy((OSSlider**)&control, panel);
        break;
    case ekGUI_COMPONENT_UPDOWN:
        _osupdown_detach_and_destroy((OSUpDown**)&control, panel);
        break;
    case ekGUI_COMPONENT_PROGRESS:
        _osprogress_detach_and_destroy((OSProgress**)&control, panel);
        break;
    case ekGUI_COMPONENT_TEXTVIEW:
        _ostext_detach_and_destroy((OSText**)&control, panel);
        break;
    case ekGUI_COMPONENT_CUSTOMVIEW:
        _osview_detach_and_destroy((OSView**)&control, panel);
        break;
    case ekGUI_COMPONENT_PANEL:
        ospanel_detach((OSPanel*)control, panel);
        _ospanel_destroy((OSPanel**)&control);
        break;

    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
    case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void _ospanel_destroy(OSPanel **panel)
{
    cassert_no_null(panel);
    cassert_no_null(*panel);
    gtk_container_foreach(GTK_CONTAINER((*panel)->control.widget), i_destroy_child, (gpointer)*panel);
    ospanel_destroy(panel);
}

/*---------------------------------------------------------------------------*/

void _ospanel_attach_control(OSPanel *panel, OSControl *control)
{
    cassert_no_null(panel);
    _oscontrol_attach_to_parent(control, panel->content ? panel->content : panel->control.widget);
}

/*---------------------------------------------------------------------------*/

void _ospanel_detach_control(OSPanel *panel, OSControl *control)
{
    cassert_no_null(panel);
    _oscontrol_detach_from_parent(control, panel->content ? panel->content : panel->control.widget);
}

/*---------------------------------------------------------------------------*/

void _ospanel_set_capture(OSPanel *panel, OSControl *control)
{
    cassert_no_null(panel);
    cassert(panel->capture == NULL);
    panel->capture = control;
}

/*---------------------------------------------------------------------------*/

void _ospanel_release_capture(OSPanel *panel)
{
    cassert_no_null(panel);
    cassert(panel->capture != NULL);
    panel->capture = NULL;
}
