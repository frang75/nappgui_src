/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osscroll.c
 *
 */

/* Operating System native scrollbar */

#include "osscroll_gtk.inl"
#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osglobals_gtk.inl"
#include "osview_gtk.inl"
#include "../osscroll.inl"
#include <core/heap.h>
#include <sewer/cassert.h>

struct _osscroll_t
{
    OSControl *control;
    GtkOrientation orient;
    GtkWidget *widget;
    GtkAdjustment *adjust;
    bool_t visible;
    bool_t is_overlay;
    bool_t is_hovering;
    uint32_t pos;
    uint32_t thickness;
    real64_t elapsed;
};

/*---------------------------------------------------------------------------*/

static const real64_t i_FADE_WAIT = 1.;
static const real64_t i_FADE_DURATION = .25;

/*---------------------------------------------------------------------------*/

static void i_set_visible(OSScroll *scroll)
{
    cassert_no_null(scroll);
    if (gtk_widget_get_realized(scroll->widget) == TRUE)
    {
        if (scroll->visible == TRUE)
            gtk_widget_show(scroll->widget);
        else
            gtk_widget_hide(scroll->widget);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnRealize(GtkWidget *widget, OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_unref(scroll->widget == widget, widget);
    i_set_visible(scroll);
}

/*---------------------------------------------------------------------------*/

static void i_OnScrollMoved(GtkRange *range, OSScroll *scroll)
{
    gui_orient_t orient = ekGUI_HORIZONTAL;
    gui_scroll_t event = ekGUI_SCROLL_THUMB;

    cassert_no_null(scroll);
    cassert_unref(range == GTK_RANGE(scroll->widget), range);
    if (scroll->orient == GTK_ORIENTATION_VERTICAL)
        orient = ekGUI_VERTICAL;

    /*
     * There is not a "secure" way in GTK to detect scrollbar steppers
     * Steppers have been removed in GTK3
     * https://blogs.gnome.org/mclasen/2013/08/05/scrolling-in-gtk/
     */
    event = ekGUI_SCROLL_THUMB;

    if (scroll->control->type == ekGUI_TYPE_CUSTOMVIEW)
    {
        _osview_scrolls_activity(cast(scroll->control, OSView));
        _osview_scroll_event(cast(scroll->control, OSView), orient, event);
    }
    else
    {
        cassert(FALSE);
    }
}

/*---------------------------------------------------------------------------*/

static void i_resync(OSScroll *scroll)
{
    cassert_no_null(scroll);
    if (scroll->control->type == ekGUI_TYPE_CUSTOMVIEW)
        _osview_scrolls_resync(cast(scroll->control, OSView));
    else
        cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnScrollEnter(GtkWidget *widget, GdkEventCrossing *event, OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(event);
    unref(widget);
    if (event->mode == GDK_CROSSING_NORMAL && scroll->is_overlay == TRUE && scroll->is_hovering == FALSE)
    {
        GtkStyleContext *context = gtk_widget_get_style_context(scroll->widget);
        gtk_style_context_add_class(context, "hovering");
        gtk_widget_queue_resize(scroll->widget);
        scroll->is_hovering = TRUE;
        scroll->elapsed = 0;
        gtk_widget_set_opacity(scroll->widget, 1);
        i_resync(scroll);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnScrollLeave(GtkWidget *widget, GdkEventCrossing *event, OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(event);
    unref(widget);
    if (event->mode == GDK_CROSSING_NORMAL && scroll->is_hovering == TRUE)
    {
        GtkStyleContext *context = gtk_widget_get_style_context(scroll->widget);
        gtk_style_context_remove_class(context, "hovering");
        gtk_widget_queue_resize(scroll->widget);
        scroll->is_hovering = FALSE;
        i_resync(scroll);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_parent(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(scroll->control);
    if (scroll->control->type == ekGUI_TYPE_CUSTOMVIEW)
        return _osview_area_widget(cast(scroll->control, OSView));
    cassert(FALSE);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static OSScroll *i_scroll_create(GtkOrientation orient, OSControl *control)
{
    OSScroll *scroll = heap_new0(OSScroll);
    GtkWidget *parent = NULL;
    scroll->control = control;
    scroll->orient = orient;
    scroll->widget = gtk_scrollbar_new(orient, NULL);
    scroll->adjust = gtk_range_get_adjustment(GTK_RANGE(scroll->widget));
    scroll->visible = TRUE;
    scroll->pos = 0;
    parent = i_parent(scroll);
    cassert(GTK_IS_LAYOUT(parent));
    gtk_layout_put(GTK_LAYOUT(parent), scroll->widget, 0, 0);
    gtk_widget_show(scroll->widget);
    g_signal_connect(scroll->widget, "realize", G_CALLBACK(i_OnRealize), (gpointer)scroll);
    g_signal_connect(scroll->widget, "value-changed", G_CALLBACK(i_OnScrollMoved), (gpointer)scroll);
    g_signal_connect(scroll->widget, "enter-notify-event", G_CALLBACK(i_OnScrollEnter), (gpointer)scroll);
    g_signal_connect(scroll->widget, "leave-notify-event", G_CALLBACK(i_OnScrollLeave), (gpointer)scroll);
    return scroll;
}

/*---------------------------------------------------------------------------*/

OSScroll *_osscroll_horizontal(OSControl *control)
{
    return i_scroll_create(GTK_ORIENTATION_HORIZONTAL, control);
}

/*---------------------------------------------------------------------------*/

OSScroll *_osscroll_vertical(OSControl *control)
{
    return i_scroll_create(GTK_ORIENTATION_VERTICAL, control);
}

/*---------------------------------------------------------------------------*/

void _osscroll_destroy(OSScroll **scroll, OSControl *control)
{
    GtkWidget *parent = NULL;
    cassert_no_null(scroll);
    cassert_no_null(*scroll);
    cassert_unref((*scroll)->control == control, control);
    parent = i_parent(*scroll);
    _oscontrol_widget_detach((*scroll)->widget, parent);
    heap_delete(scroll, OSScroll);
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_pos(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    return scroll->pos;
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_trackpos(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    return (uint32_t)gtk_adjustment_get_value(scroll->adjust);
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_bar_width(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_unref(scroll->orient == GTK_ORIENTATION_VERTICAL, scroll);
    return scroll->thickness;
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_bar_height(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_unref(scroll->orient == GTK_ORIENTATION_HORIZONTAL, scroll);
    return scroll->thickness;
}

/*---------------------------------------------------------------------------*/

void _osscroll_set_pos(OSScroll *scroll, const uint32_t pos)
{
    guint nref = 0;
    cassert_no_null(scroll);
    scroll->pos = pos;
    nref = g_signal_handlers_block_by_func(scroll->widget, (gpointer)(intptr_t)i_OnScrollMoved, (gpointer)scroll);
    cassert_unref(nref == 1, nref);
    gtk_adjustment_set_value(scroll->adjust, (gdouble)pos);
    nref = g_signal_handlers_unblock_by_func(scroll->widget, (gpointer)(intptr_t)i_OnScrollMoved, (gpointer)scroll);
    cassert_unref(nref == 1, nref);
}

/*---------------------------------------------------------------------------*/

void _osscroll_visible(OSScroll *scroll, const bool_t visible)
{
    cassert_no_null(scroll);
    if (visible == TRUE)
    {
        bool_t was_hidden = (bool_t)(scroll->visible == FALSE);
        scroll->visible = TRUE;
        i_set_visible(scroll);
        if (was_hidden == TRUE && scroll->is_overlay == TRUE)
        {
            scroll->elapsed = 0;
            gtk_widget_set_opacity(scroll->widget, 1);
        }
    }
    else
    {
        scroll->visible = FALSE;
        i_set_visible(scroll);
    }
}

/*---------------------------------------------------------------------------*/

void _osscroll_config(OSScroll *scroll, const uint32_t pos, const uint32_t max, const uint32_t page)
{
    cassert_no_null(scroll);
    gtk_adjustment_set_lower(scroll->adjust, 0);
    gtk_adjustment_set_upper(scroll->adjust, (gdouble)max);
    gtk_adjustment_set_page_size(scroll->adjust, (gdouble)page);
    _osscroll_set_pos(scroll, pos);
}

/*---------------------------------------------------------------------------*/

void _osscroll_frame(OSScroll *scroll, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
{
    GtkWidget *parent = i_parent(scroll);
    gint nx = (gint)x;
    gint ny = (gint)y;
    cassert_no_null(scroll);
    cassert(GTK_IS_LAYOUT(parent));
    gtk_layout_move(GTK_LAYOUT(parent), scroll->widget, nx, ny);
    gtk_widget_set_size_request(scroll->widget, (gint)width, (gint)height);
}

/*---------------------------------------------------------------------------*/

void _osscroll_control_scroll(OSControl *control, const int32_t incr_x, const int32_t incr_y)
{
    unref(control);
    unref(incr_x);
    unref(incr_y);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void _osscroll_sync(OSScroll *scroll)
{
    bool_t overlay = FALSE;
    cassert_no_null(scroll);

#if GTK_CHECK_VERSION(3, 16, 0)
    {
        gboolean overlay_setting = TRUE;
        g_object_get(gtk_settings_get_default(), "gtk-overlay-scrolling", &overlay_setting, NULL);
        overlay = (bool_t)overlay_setting;
    }
#endif

    if (overlay != scroll->is_overlay)
    {
        GtkStyleContext *context = gtk_widget_get_style_context(scroll->widget);
        if (overlay == TRUE)
            gtk_style_context_add_class(context, "overlay-indicator");
        else
            gtk_style_context_remove_class(context, "overlay-indicator");

        gtk_widget_queue_resize(scroll->widget);
        scroll->is_overlay = overlay;

        if (overlay == FALSE)
            gtk_widget_set_opacity(scroll->widget, 1);
    }

    {
        gint minimum = 0, natural = 0;
        gtk_widget_show(scroll->widget);
        if (scroll->orient == GTK_ORIENTATION_VERTICAL)
            gtk_widget_get_preferred_width(scroll->widget, &minimum, &natural);
        else
            gtk_widget_get_preferred_height(scroll->widget, &minimum, &natural);
        scroll->thickness = (uint32_t)natural;
    }
}

/*---------------------------------------------------------------------------*/

bool_t _osscroll_overlay(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    return scroll->is_overlay;
}

/*---------------------------------------------------------------------------*/

void _osscroll_activity(OSScroll *scroll)
{
    cassert_no_null(scroll);
    if (scroll->is_overlay == TRUE)
    {
        scroll->elapsed = 0;
        gtk_widget_set_opacity(scroll->widget, 1);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _osscroll_transition(OSScroll *scroll, const real64_t prtime, const real64_t crtime)
{
    cassert_no_null(scroll);
    if (scroll->is_overlay == FALSE)
        return FALSE;

    if (scroll->is_hovering == TRUE)
    {
        scroll->elapsed = 0;
        gtk_widget_set_opacity(scroll->widget, 1);
        return TRUE;
    }

    scroll->elapsed += (crtime - prtime);

    if (scroll->elapsed < i_FADE_WAIT)
    {
        return TRUE;
    }
    else if (scroll->elapsed < i_FADE_WAIT + i_FADE_DURATION)
    {
        real64_t alpha = 1. - (scroll->elapsed - i_FADE_WAIT) / i_FADE_DURATION;
        gtk_widget_set_opacity(scroll->widget, alpha);
        return TRUE;
    }
    else
    {
        gtk_widget_set_opacity(scroll->widget, 0);
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

gui_scroll_t _osscroll_wheel_event(const GdkEventScroll *event, gui_orient_t *orient)
{
    bool_t shift = FALSE;
    gdouble delta = 0;
    cassert_no_null(event);
    cassert_no_null(orient);
    shift = (bool_t)((event->state & GDK_SHIFT_MASK) != 0);
    *orient = ekGUI_VERTICAL;

    switch (event->direction)
    {
    case GDK_SCROLL_UP:
        delta = -1;
        break;
    case GDK_SCROLL_DOWN:
        delta = 1;
        break;
    case GDK_SCROLL_LEFT:
        *orient = ekGUI_HORIZONTAL;
        delta = -1;
        break;
    case GDK_SCROLL_RIGHT:
        *orient = ekGUI_HORIZONTAL;
        delta = 1;
        break;
    case GDK_SCROLL_SMOOTH:
        if (event->delta_x != 0)
        {
            *orient = ekGUI_HORIZONTAL;
            delta = event->delta_x;
        }
        else
        {
            delta = event->delta_y;
        }
        break;
    default:
        cassert_default(event->direction);
    }

    if (*orient == ekGUI_VERTICAL && shift == TRUE)
        *orient = ekGUI_HORIZONTAL;

    if (delta > 0)
        return ekGUI_SCROLL_STEP_RIGHT;
    else if (delta < 0)
        return ekGUI_SCROLL_STEP_LEFT;

    return ENUM_MAX(gui_scroll_t);
}
