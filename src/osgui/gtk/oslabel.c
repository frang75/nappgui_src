/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oslabel.c
 *
 */

/* Operating System label */

#include "oslistener.inl"
#include "osgui_gtk.inl"
#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "../oslabel.h"
#include "../oslabel.inl"
#include "../osgui.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _oslabel_t
{
    OSControl control;
    GtkWidget *label;
    String *text;
    Font *font;
    color_t tcolor;
    color_t bgcolor;
    align_t align;
    PangoEllipsizeMode ellipsis;
    bool_t layout_updated;
    PangoLayout *layout;
    gint enter_signal;
    gint exit_signal;
    gint click_signal;
    real32_t control_width;
    real32_t control_height;
    real32_t text_width;
    real32_t text_height;
    Listener *OnClick;
    Listener *OnMouseEnter;
    Listener *OnMouseExit;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEventCrossing *event, OSLabel *label)
{
    unref(widget);
    unref(event);
    cassert_no_null(label);
    if (label->OnMouseEnter != NULL)
    {
        EvMouse params;
        params.x = (real32_t)event->x;
        params.y = (real32_t)event->y;
        params.lx = params.x;
        params.ly = params.y;
        params.button = ENUM_MAX(gui_mouse_t);
        params.count = 0;
        params.modifiers = 0;
        params.tag = 0;
        listener_event(label->OnMouseEnter, ekGUI_EVENT_ENTER, label, &params, NULL, OSLabel, EvMouse, void);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnExit(GtkWidget *widget, GdkEventCrossing *event, OSLabel *label)
{
    unref(widget);
    unref(event);
    cassert_no_null(label);
    if (label->OnMouseExit != NULL)
        listener_event(label->OnMouseExit, ekGUI_EVENT_EXIT, label, NULL, NULL, OSLabel, void, void);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnClick(GtkWidget *widget, GdkEventButton *event, OSLabel *label)
{
    unref(widget);
    unref(event);
    cassert_no_null(label);
    if (label->OnClick != NULL)
    {
        EvText params;
        params.text = NULL;
        listener_event(label->OnClick, ekGUI_EVENT_LABEL, label, &params, NULL, OSLabel, EvText, void);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_set_text(OSLabel *label)
{
    uint32_t fstyle = 0;
    String *format = NULL;
    cassert_no_null(label);
    fstyle = font_style(label->font);
    format = str_printf("<span ");

    if (fstyle & ekFUNDERLINE)
        str_cat(&format, " underline=\"single\"");

    if (fstyle & ekFSTRIKEOUT)
        str_cat(&format, " strikethrough=\"true\"");

    {
        char_t html[16];
        color_to_html(label->tcolor != kCOLOR_DEFAULT ? label->tcolor : ekSYSCOLOR_LABEL, html, sizeof(html));
        str_cat(&format, " foreground=\"");
        str_cat(&format, html);
        str_cat(&format, "\"");
    }

    str_cat(&format, ">");
    str_cat(&format, tc(label->text));
    str_cat(&format, "</span>");

    pango_layout_set_markup(label->layout, tc(format), -1);
    str_destroy(&format);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSLabel *label)
{
    real32_t xscale = 1;
    cassert_no_null(label);
    cassert_unref(widget == label->label, widget);

    xscale = font_xscale(label->font);

    if (label->layout == NULL)
        label->layout = pango_cairo_create_layout(cr);

    if (label->layout_updated == FALSE)
    {
        const PangoFontDescription *fdesc = cast(font_native(label->font), PangoFontDescription);
        pango_layout_set_font_description(label->layout, fdesc);
        pango_layout_set_width(label->layout, (int)((label->control_width / xscale) * PANGO_SCALE));
        pango_layout_set_height(label->layout, -1);
        pango_layout_set_ellipsize(label->layout, label->ellipsis);
        font_extents(label->font, tc(label->text), label->control_width / xscale, &label->text_width, &label->text_height);
        i_set_text(label);
        label->layout_updated = TRUE;
    }

    if (label->bgcolor != kCOLOR_DEFAULT)
    {
        real32_t r, g, b, a;
        color_get_rgbaf(label->bgcolor, &r, &g, &b, &a);
        cairo_set_source_rgba(cr, (double)r, (double)g, (double)b, (double)a);
        cairo_rectangle(cr, 0, 0, (double)label->control_width, (double)label->control_height);
        cairo_fill(cr);
    }

    cairo_save(cr);

    if (label->control_width > label->text_width)
    {
        switch (label->align)
        {
        case ekLEFT:
        case ekJUSTIFY:
            break;
        case ekCENTER:
            cairo_translate(cr, (double)((label->control_width - label->text_width) / 2), 0);
            break;
        case ekRIGHT:
            cairo_translate(cr, (double)(label->control_width - label->text_width), 0);
            break;
        }
    }

    cairo_scale(cr, xscale, 1);
    pango_cairo_show_layout(cr, label->layout);
    cairo_restore(cr);

    /* Stop other handlers from being invoked for the event */
    return TRUE;
}

/*---------------------------------------------------------------------------*/

OSLabel *oslabel_create(const uint32_t flags)
{
    OSLabel *label = heap_new0(OSLabel);
    GtkWidget *widget = gtk_event_box_new();
    _oscontrol_init(&label->control, ekGUI_TYPE_LABEL, widget, widget, TRUE);
    label->label = gtk_drawing_area_new();
    label->text = str_c("");
    label->font = _osgui_create_default_font();
    label->tcolor = kCOLOR_DEFAULT;
    label->bgcolor = kCOLOR_DEFAULT;
    label->align = ekLEFT;
    label->ellipsis = PANGO_ELLIPSIZE_NONE;
    label->layout_updated = FALSE;
    g_signal_connect(G_OBJECT(label->label), "draw", G_CALLBACK(i_OnDraw), label);
    gtk_widget_show(label->label);
    gtk_container_add(GTK_CONTAINER(widget), label->label);
    unref(flags);
    return label;
}

/*---------------------------------------------------------------------------*/

void oslabel_destroy(OSLabel **label)
{
    cassert_no_null(label);
    cassert_no_null(*label);
    listener_destroy(&(*label)->OnClick);
    listener_destroy(&(*label)->OnMouseEnter);
    listener_destroy(&(*label)->OnMouseExit);
    font_destroy(&(*label)->font);
    str_destroy(&(*label)->text);

    if ((*label)->layout != NULL)
    {
        g_object_unref((*label)->layout);
        (*label)->layout = NULL;
    }

    _oscontrol_destroy(*dcast(label, OSControl));
    heap_delete(label, OSLabel);
}

/*---------------------------------------------------------------------------*/

void oslabel_OnClick(OSLabel *label, Listener *listener)
{
    cassert_no_null(label);
    listener_update(&label->OnClick, listener);
    _oslistener_signal(label->control.widget, listener != NULL, &label->click_signal, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK, "button-press-event", G_CALLBACK(i_OnClick), (gpointer)label);
}

/*---------------------------------------------------------------------------*/

void oslabel_OnEnter(OSLabel *label, Listener *listener)
{
    cassert_no_null(label);
    listener_update(&label->OnMouseEnter, listener);
    _oslistener_signal(label->control.widget, listener != NULL, &label->enter_signal, GDK_ENTER_NOTIFY_MASK, "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)label);
}

/*---------------------------------------------------------------------------*/

void oslabel_OnExit(OSLabel *label, Listener *listener)
{
    cassert_no_null(label);
    listener_update(&label->OnMouseExit, listener);
    _oslistener_signal(label->control.widget, listener != NULL, &label->exit_signal, GDK_LEAVE_NOTIFY_MASK, "leave-notify-event", G_CALLBACK(i_OnExit), (gpointer)label);
}

/*---------------------------------------------------------------------------*/

void oslabel_text(OSLabel *label, const char_t *text)
{
    cassert_no_null(label);
    str_upd(&label->text, text);
    label->layout_updated = FALSE;
    gtk_widget_queue_draw(label->label);
}

/*---------------------------------------------------------------------------*/

void oslabel_font(OSLabel *label, const Font *font)
{
    cassert_no_null(label);
    if (font_equals(font, label->font) == FALSE)
    {
        font_destroy(&label->font);
        label->font = font_copy(font);
        label->layout_updated = FALSE;
        gtk_widget_queue_draw(label->label);
    }
}

/*---------------------------------------------------------------------------*/

void oslabel_flags(OSLabel *label, const uint32_t flags)
{
    unref(label);
    unref(flags);
}

/*---------------------------------------------------------------------------*/

void oslabel_align(OSLabel *label, const align_t align)
{
    cassert_no_null(label);
    label->align = align;
    label->layout_updated = TRUE;
    gtk_widget_queue_draw(label->label);
}

/*---------------------------------------------------------------------------*/

static PangoEllipsizeMode i_ellipsis(const ellipsis_t ellipsis)
{
    switch (ellipsis)
    {
    case ekELLIPNONE:
    case ekELLIPMLINE:
        return PANGO_ELLIPSIZE_NONE;
    case ekELLIPBEGIN:
        return PANGO_ELLIPSIZE_START;
    case ekELLIPMIDDLE:
        return PANGO_ELLIPSIZE_MIDDLE;
    case ekELLIPEND:
        return PANGO_ELLIPSIZE_END;
        cassert_default();
    }

    return PANGO_ELLIPSIZE_NONE;
}

/*---------------------------------------------------------------------------*/

void oslabel_ellipsis(OSLabel *label, const ellipsis_t ellipsis)
{
    cassert_no_null(label);
    label->ellipsis = i_ellipsis(ellipsis);
    label->layout_updated = FALSE;
    gtk_widget_queue_draw(label->label);
}

/*---------------------------------------------------------------------------*/

void oslabel_color(OSLabel *label, const color_t color)
{
    cassert_no_null(label);
    if (label->tcolor != color)
    {
        label->tcolor = color;
        gtk_widget_queue_draw(label->label);
    }
}

/*---------------------------------------------------------------------------*/

void oslabel_bgcolor(OSLabel *label, const color_t color)
{
    cassert_no_null(label);
    if (label->bgcolor != color)
    {
        label->bgcolor = color;
        gtk_widget_queue_draw(label->label);
    }
}

/*---------------------------------------------------------------------------*/

void oslabel_bounds(const OSLabel *label, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    cassert_no_null(label);
    cassert_no_null(width);
    cassert_no_null(height);
    font_extents(label->font, text, refwidth, width, height);
}

/*---------------------------------------------------------------------------*/

void oslabel_attach(OSLabel *label, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(label, OSControl));
}

/*---------------------------------------------------------------------------*/

void oslabel_detach(OSLabel *label, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(label, OSControl));
}

/*---------------------------------------------------------------------------*/

void oslabel_visible(OSLabel *label, const bool_t visible)
{
    _oscontrol_set_visible(cast(label, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void oslabel_enabled(OSLabel *label, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(label, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void oslabel_size(const OSLabel *label, real32_t *width, real32_t *height)
{
    cassert_no_null(label);
    cassert_no_null(width);
    cassert_no_null(height);
    *width = label->control_width;
    *height = label->control_height;
}

/*---------------------------------------------------------------------------*/

void oslabel_origin(const OSLabel *label, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(label, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void oslabel_frame(OSLabel *label, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(label);
    _oscontrol_set_frame(cast(label, OSControl), x, y, width, height);
    gtk_widget_set_size_request(label->label, (gint)width, (gint)height);
    label->control_width = width;
    label->control_height = height;
    label->layout_updated = FALSE;
    gtk_widget_queue_draw(label->label);
}