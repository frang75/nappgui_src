/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osslider.c
 *
 */

/* Operating System native slider */

#include "osgui_gtk.inl"
#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osslider_gtk.inl"
#include "ospanel_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osslider.h"
#include "../osslider.inl"
#include "../osgui.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osslider_t
{
    OSControl control;
    gui_size_t size;
    bool_t launch_event;
    bool_t discrete;
    bool_t has_focus;
    uint32_t flags;
    Listener *OnMoved;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnMoved(GtkRange *range, GtkScrollType step, double value, OSSlider *slider)
{
    cassert_unref(slider->control.widget == GTK_WIDGET(range), range);
    unref(step);
    if (slider->launch_event == TRUE && slider->OnMoved != NULL)
    {
        EvSlider params;
        params.pos = (real32_t)value;
        params.incr = 0.f;
        params.step = UINT32_MAX;

        if (value < 0)
            params.pos = 0;
        else if (value > 1)
            params.pos = 1;

        listener_event(slider->OnMoved, ekGUI_EVENT_SLIDER, slider, &params, NULL, OSSlider, EvSlider, void);
        return slider->discrete;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEvent *event, OSSlider *slider)
{
    cassert_no_null(slider);
    unref(widget);
    unref(event);
    if (_oswindow_mouse_down(cast(slider, OSControl)) == TRUE)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnFocusDrawBefore(GtkWidget *widget, cairo_t *cr, OSSlider *slider)
{
    cassert_no_null(slider);
    if (_osslider_has_focus(slider) == TRUE)
    {
        GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
        GtkStateFlags state = gtk_style_context_get_state(ctx) | GTK_STATE_FLAG_FOCUSED;
        GdkRectangle rect;
        int margin = 4;
        gtk_range_get_range_rect(GTK_RANGE(widget), &rect);
        gtk_style_context_save(ctx);
        gtk_style_context_set_state(ctx, state);
        gtk_render_focus(ctx, cr, (double)(rect.x - margin), (double)(rect.y - margin), (double)(rect.width + 2 * margin), (double)(rect.height + 2 * margin));
        gtk_style_context_restore(ctx);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPlaceholderDraw(GtkWidget *widget, cairo_t *cr, OSSlider *slider)
{
    GdkPixbuf *bitmap = _osglobals_slider_bitmap();
    uint32_t iw = _osglobals_slider_width();
    uint32_t ih = _osglobals_slider_height();
    uint32_t dw = iw, dh = ih;
    uint32_t cell = 0;
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    real32_t x, y;
    GdkPixbuf *cellbmp;
    cassert_no_null(slider);

    if (gtk_widget_get_sensitive(slider->control.widget) == FALSE)
        cell = 1;

    /* Not even the real thumb fits at its natural size: scale the same bitmap down */
    if ((uint32_t)w < iw || (uint32_t)h < ih)
    {
        real32_t scale = (real32_t)w / (real32_t)iw;
        real32_t scaleh = (real32_t)h / (real32_t)ih;
        if (scaleh < scale)
            scale = scaleh;

        dw = (uint32_t)((real32_t)iw * scale);
        dh = (uint32_t)((real32_t)ih * scale);
        if (dw < 1)
            dw = 1;
        if (dh < 1)
            dh = 1;
    }

    x = ((real32_t)w - (real32_t)dw) / 2.f;
    y = ((real32_t)h - (real32_t)dh) / 2.f;
    cellbmp = gdk_pixbuf_new_subpixbuf(bitmap, (int)(cell * iw), 0, (int)iw, (int)ih);

    if (dw != iw || dh != ih)
    {
        GdkPixbuf *scaled = gdk_pixbuf_scale_simple(cellbmp, (int)dw, (int)dh, GDK_INTERP_BILINEAR);
        g_object_unref(cellbmp);
        cellbmp = scaled;
    }

    gdk_cairo_set_source_pixbuf(cr, cellbmp, (double)x, (double)y);
    cairo_rectangle(cr, (double)x, (double)y, (double)dw, (double)dh);
    cairo_fill(cr);
    g_object_unref(cellbmp);

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPlaceholderPressed(GtkWidget *widget, GdkEventButton *event, OSSlider *slider)
{
    unref(widget);
    unref(event);
    if (_oswindow_mouse_down(cast(slider, OSControl)) == TRUE)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_placeholder(OSControl *control)
{
    OSSlider *slider = cast(control, OSSlider);
    GtkWidget *placeholder = gtk_drawing_area_new();
    gtk_widget_set_can_focus(placeholder, TRUE);
    gtk_widget_add_events(placeholder, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(placeholder), "draw", G_CALLBACK(i_OnPlaceholderDraw), (gpointer)slider);
    g_signal_connect(G_OBJECT(placeholder), "button-press-event", G_CALLBACK(i_OnPlaceholderPressed), (gpointer)slider);
    return placeholder;
}

/*---------------------------------------------------------------------------*/

OSSlider *osslider_create(const uint32_t flags)
{
    OSSlider *slider = heap_new0(OSSlider);
    GtkOrientation o = slider_get_type(flags) == ekSLIDER_HORZ ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    GtkWidget *widget = gtk_scale_new_with_range(o, 0., 1., 0.1);
    gtk_scale_set_draw_value(GTK_SCALE(widget), FALSE);
    gtk_range_set_increments(GTK_RANGE(widget), .1, .1);
    g_signal_connect(widget, "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)slider);
    g_signal_connect(widget, "change-value", G_CALLBACK(i_OnMoved), (gpointer)slider);
    g_signal_connect(widget, "draw", G_CALLBACK(i_OnFocusDrawBefore), (gpointer)slider);

    /*
    if (o == GTK_ORIENTATION_HORIZONTAL)
       _oscontrol_widget_set_css(widget, "scale {min-height:5px;padding-left:5px;padding-right:5px}");
    else
       _oscontrol_widget_set_css(widget, "scale {min-height:5px;padding-top:5px;padding-bottom:5px}");
    */

    _oscontrol_init(&slider->control, ekGUI_TYPE_SLIDER, widget, widget, TRUE);
    slider->size = ENUM_MAX(gui_size_t);
    slider->flags = flags;
    slider->launch_event = TRUE;
    slider->discrete = FALSE;
    _oscontrol_squeeze_filler(&slider->control, i_placeholder);
    _oscontrol_squeeze_fixed_width(&slider->control, 20);
    return slider;
}

/*---------------------------------------------------------------------------*/

void osslider_destroy(OSSlider **slider)
{
    cassert_no_null(slider);
    cassert_no_null(*slider);
    listener_destroy(&(*slider)->OnMoved);
    _oscontrol_destroy(*dcast(slider, OSControl));
    heap_delete(slider, OSSlider);
}

/*---------------------------------------------------------------------------*/

void osslider_OnMoved(OSSlider *slider, Listener *listener)
{
    cassert_no_null(slider);
    listener_update(&slider->OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void osslider_tooltip(OSSlider *slider, const char_t *text)
{
    cassert_no_null(slider);
    gtk_widget_set_tooltip_text(slider->control.widget, (const gchar *)text);
}

/*---------------------------------------------------------------------------*/

void osslider_tickmarks(OSSlider *slider, const uint32_t num_tickmarks, const bool_t tickmarks_at_left_top)
{
    gdouble steps = 0.1;
    cassert_no_null(slider);
    cassert(num_tickmarks > 1);

    if (num_tickmarks != UINT32_MAX)
    {
        steps = 1. / (num_tickmarks - 1.);
        slider->discrete = TRUE;
    }
    else
    {
        slider->discrete = FALSE;
    }

    gtk_range_set_increments(GTK_RANGE(slider->control.widget), steps, steps);
    unref(tickmarks_at_left_top);
}

/*---------------------------------------------------------------------------*/

void osslider_position(OSSlider *slider, const real32_t position)
{
    cassert_no_null(slider);
    slider->launch_event = FALSE;
    gtk_range_set_value(GTK_RANGE(slider->control.widget), (gdouble)position);
    slider->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

real32_t osslider_get_position(const OSSlider *slider)
{
    cassert_no_null(slider);
    return (real32_t)gtk_range_get_value(GTK_RANGE(slider->control.widget));
}

/*---------------------------------------------------------------------------*/

void osslider_bounds(const OSSlider *slider, const real32_t length, const gui_size_t knob_size, real32_t *width, real32_t *height)
{
    GtkRequisition s;
    cassert_no_null(slider);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert_unref(knob_size == ekGUI_SIZE_REGULAR, knob_size);

    if (slider->control.filler.active == TRUE)
    {
        s.width = (gint)slider->control.filler.width;
        s.height = (gint)slider->control.filler.height;
    }
    else
    {
        gtk_widget_get_preferred_size(slider->control.widget, &s, NULL);
    }

    switch (slider_get_type(slider->flags))
    {
    case ekSLIDER_HORZ:
        *width = length;
        *height = (real32_t)s.height;
        break;
    case ekSLIDER_VERT:
        *width = (real32_t)s.width;
        *height = length;
        break;
    default:
        cassert_default(slider_get_type(slider->flags));
    }
}

/*---------------------------------------------------------------------------*/

void osslider_attach(OSSlider *slider, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(slider, OSControl));
}

/*---------------------------------------------------------------------------*/

void osslider_detach(OSSlider *slider, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(slider, OSControl));
}

/*---------------------------------------------------------------------------*/

void osslider_visible(OSSlider *slider, const bool_t visible)
{
    _oscontrol_set_visible(cast(slider, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osslider_enabled(OSSlider *slider, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(slider, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osslider_size(const OSSlider *slider, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(slider, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osslider_origin(const OSSlider *slider, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(slider, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osslider_frame(OSSlider *slider, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(slider, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osslider_focus_widget(OSSlider *slider)
{
    cassert_no_null(slider);

    if (slider->control.filler.active == TRUE)
        return slider->control.filler.widget;

    return slider->control.widget;
}

/*---------------------------------------------------------------------------*/

bool_t _osslider_has_focus(const OSSlider *slider)
{
    cassert_no_null(slider);
    return slider->has_focus;
}

/*---------------------------------------------------------------------------*/

void _osslider_focus(OSSlider *slider, const bool_t focus)
{
    cassert_no_null(slider);
    slider->has_focus = focus;

    if (slider->control.filler.widget != NULL)
        gtk_widget_queue_draw(slider->control.filler.widget);
}
