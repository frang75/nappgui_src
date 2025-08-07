/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.c
 *
 */

/* Operating System edit box */

#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "osedit_gtk.inl"
#include "osentry_gtk.inl"
#include "ospanel_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osedit.h"
#include "../osedit.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osedit_t
{
    OSControl control;
    uint32_t fsize_render;
    uint32_t vpadding;
    uint32_t hpadding;
    GtkWidget *tview;
    GtkCssProvider *css_padding;
    OSEntry *entry;
};

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSEdit *edit)
{
    const Font *font = NULL;
    real32_t fsize = 0;
    real32_t fscale = 1;
    cassert_no_null(edit);
    font = _osentry_get_font(edit->entry);
    fsize = font_size(font);
    fscale = font_xscale(font);
    edit->fsize_render = (uint32_t)fsize;

    /*
     * GtkEntry is not prepared to edit strings with scaled fonts,
     * causing precision errors when clicking on a character and limiting
     * the width of the PangoLayout. For this reason, scaled fonts are only
     * shown when the control does not have keyboard focus.
     */
    if (bmath_absf(fscale - 1) >= .01f && gtk_widget_has_focus(widget) == FALSE)
    {
        int w = gtk_widget_get_allocated_width(widget);
        int h = gtk_widget_get_allocated_height(widget);
        GtkStyleContext *ctx = _osglobals_entry_context();
        PangoLayout *layout = gtk_entry_get_layout(GTK_ENTRY(widget));
        gtk_render_background(ctx, cr, 0, 0, w, h);
        gtk_render_frame(ctx, cr, 0, 0, w, h);
        cairo_save(cr);
        cairo_translate(cr, edit->hpadding / 2, edit->vpadding / 2);
        cairo_scale(cr, fscale, 1);
        pango_cairo_show_layout(cr, layout);
        cairo_restore(cr);
        return TRUE;
    }
    else
    {
        /* We call to standard draw pipeline */
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_DrawBackground(GtkWidget *widget, cairo_t *cr, OSEdit *edit)
{
    GtkStyleContext *ctx = _osglobals_entry_context();
    const Font *font = NULL;
    GtkCssProvider *bgcolor = NULL;
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    cassert_no_null(edit);
    font = _osentry_get_font(edit->entry);
    bgcolor = _osentry_get_bgcolor(edit->entry);

    if (bgcolor != NULL)
        gtk_style_context_add_provider(ctx, GTK_STYLE_PROVIDER(bgcolor), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_style_context_save(ctx);
    gtk_style_context_set_state(ctx, gtk_widget_get_state_flags(widget));
    gtk_render_background(ctx, cr, 0, 0, w, h);
    gtk_render_frame(ctx, cr, 0, 0, w, h);
    gtk_style_context_restore(ctx);
    edit->fsize_render = (uint32_t)font_size(font);

    if (bgcolor != NULL)
        gtk_style_context_remove_provider(ctx, GTK_STYLE_PROVIDER(bgcolor));

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSEdit *edit)
{
    guint key = 0;
    unref(widget);
    cassert_no_null(event);
    cassert_no_null(edit);
    cassert(edit->tview == NULL);

    key = event->keyval;
    /* Avoid up-down GTK edit navigation  */
    if (key == GDK_KEY_Down || key == GDK_KEY_Up)
        return TRUE;

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSEdit *edit)
{
    unref(widget);
    cassert_no_null(event);
    if (_oswindow_mouse_down(cast(edit, OSControl)) == FALSE)
        return TRUE;

    /* Propagate the event */
    _osentry_deselect(edit->entry);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const uint32_t flags)
{
    OSEdit *edit = heap_new0(OSEdit);
    GtkWidget *widget = NULL;
    edit->vpadding = kENTRY_VPADDING;
    edit->hpadding = kENTRY_HPADDING;

    switch (edit_get_type(flags))
    {
    case ekEDIT_SINGLE:
    {
        const char_t *cssobj = _osglobals_css_entry();
        widget = gtk_entry_new();
        gtk_entry_set_width_chars(GTK_ENTRY(widget), 0);
        _oscontrol_update_css_padding(widget, cssobj, edit->vpadding, edit->hpadding, &edit->css_padding);
        g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(i_OnDraw), (gpointer)edit);
        g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)edit);
        g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)edit);
        break;
    }

    case ekEDIT_MULTI:
    {
        const char_t *cssobj = _osglobals_css_textview();
        edit->tview = gtk_text_view_new();
        _oscontrol_update_css_padding(edit->tview, cssobj, edit->vpadding, edit->hpadding, &edit->css_padding);
        gtk_text_view_set_accepts_tab(GTK_TEXT_VIEW(edit->tview), FALSE);
        widget = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_set_border_width(GTK_CONTAINER(widget), 0);
        gtk_container_add(GTK_CONTAINER(widget), edit->tview);
        g_signal_connect(G_OBJECT(edit->tview), "draw", G_CALLBACK(i_DrawBackground), (gpointer)edit);
        g_signal_connect(G_OBJECT(edit->tview), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)edit);
        gtk_widget_show(edit->tview);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(edit->tview), GTK_WRAP_WORD_CHAR);

        /*
        CSS padding works OK. Check in older GTK3 versions
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(edit->tview), kENTRY_HPADDING / 2);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(edit->tview), kENTRY_HPADDING / 2);
#if GTK_CHECK_VERSION(3, 18, 0)
        gtk_text_view_set_top_margin(GTK_TEXT_VIEW(edit->tview), kENTRY_VPADDING / 2);
        gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(edit->tview), kENTRY_VPADDING / 2);
#endif
        */

        {
            String *css = str_printf("%s {background-image:none;background-color:transparent;}", cssobj);
            _oscontrol_fixed_css_provider(edit->tview, tc(css));
            str_destroy(&css);
        }

        {
            String *css = str_printf("%s text {background-image:none;background-color:transparent;}", cssobj);
            _oscontrol_fixed_css_provider(edit->tview, tc(css));
            str_destroy(&css);
        }
        break;
    }

        cassert_default();
    }

    edit->entry = _osentry_from_entry(edit->tview ? edit->tview : widget, cast(edit, OSControl));

    {
        Font *font = _osgui_create_default_font();
        edit->fsize_render = (uint32_t)font_size(font);
        _osentry_font(edit->entry, font);
        font_destroy(&font);
    }

    _oscontrol_init(&edit->control, ekGUI_TYPE_EDITBOX, widget, edit->tview ? edit->tview : widget, TRUE);
    return edit;
}

/*---------------------------------------------------------------------------*/

void osedit_destroy(OSEdit **edit)
{
    cassert_no_null(edit);
    cassert_no_null(*edit);

    if ((*edit)->tview != NULL)
    {
        /* Is destroyed by scrolled window
        g_object_unref((*edit)->tview); */
        (*edit)->tview = NULL;
    }

    _oscontrol_destroy_css_provider(&(*edit)->css_padding);
    _osentry_destroy(&(*edit)->entry);
    _oscontrol_destroy(*dcast(edit, OSControl));
    heap_delete(edit, OSEdit);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFilter(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _osentry_OnFilter(edit->entry, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnChange(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _osentry_OnChange(edit->entry, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFocus(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _osentry_OnFocus(edit->entry, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_text(OSEdit *edit, const char_t *text)
{
    cassert_no_null(edit);
    _osentry_text(edit->entry, text);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    cassert_no_null(edit);
    _osentry_tooltip(edit->entry, text);
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    cassert_no_null(edit);
    _osentry_font(edit->entry, font);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    cassert_no_null(edit);
    _osentry_align(edit->entry, align);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    cassert_no_null(edit);
    _osentry_passmode(edit->entry, passmode);
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    cassert_no_null(edit);
    _osentry_editable(edit->entry, is_editable);
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    cassert_no_null(edit);
    _osentry_autoselect(edit->entry, autoselect);
}

/*---------------------------------------------------------------------------*/

void osedit_select(OSEdit *edit, const int32_t start, const int32_t end)
{
    cassert_no_null(edit);
    _osentry_select(edit->entry, start, end);
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    _osentry_color(edit->entry, color);
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    _osentry_bgcolor(edit->entry, color);
}

/*---------------------------------------------------------------------------*/

void osedit_vpadding(OSEdit *edit, const real32_t padding)
{
    const char_t *cssobj = NULL;
    GtkWidget *widget = NULL;
    cassert_no_null(edit);

    if (edit->tview != NULL)
    {
        cssobj = _osglobals_css_textview();
        widget = edit->tview;
    }
    else
    {
        cssobj = _osglobals_css_entry();
        widget = edit->control.widget;
    }

    edit->vpadding = padding >= 0 ? (uint32_t)padding : kENTRY_VPADDING;
    _oscontrol_update_css_padding(widget, cssobj, edit->vpadding, edit->hpadding, &edit->css_padding);
}

/*---------------------------------------------------------------------------*/

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height)
{
    const Font *font = NULL;
    cassert_no_null(edit);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert_unref(lines == 1, lines);
    font = _osentry_get_font(edit->entry);
    if (edit->tview != NULL)
    {
        GtkRequisition s1, s2;
        real32_t fheight = bmath_ceilf(edit->vpadding + font_size(font));
        gtk_widget_set_size_request(edit->control.widget, (gint)refwidth, (gint)fheight);
        gtk_widget_set_size_request(edit->tview, (gint)refwidth, (gint)fheight);
        gtk_widget_get_preferred_size(edit->control.widget, &s1, NULL);
        gtk_widget_get_preferred_size(edit->tview, &s2, NULL);
        gtk_widget_set_size_request(edit->control.widget, -1, -1);
        gtk_widget_set_size_request(edit->tview, -1, -1);
        *width = refwidth;
        *height = (real32_t)(s1.height > s2.height ? s1.height : s2.height);
    }
    else
    {
        GtkRequisition s;
        gtk_widget_set_size_request(edit->control.widget, -1, -1);
        gtk_widget_get_preferred_size(edit->control.widget, &s, NULL);
        *width = refwidth;
        *height = (real32_t)s.height + font_size(font) - edit->fsize_render;
    }
}

/*---------------------------------------------------------------------------*/

void osedit_clipboard(OSEdit *edit, const clipboard_t clipboard)
{
    cassert_no_null(edit);
    _osentry_clipboard(edit->entry, clipboard);
}

/*---------------------------------------------------------------------------*/

void osedit_attach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(edit, OSControl));
}

/*---------------------------------------------------------------------------*/

void osedit_detach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(edit, OSControl));
}

/*---------------------------------------------------------------------------*/

void osedit_visible(OSEdit *edit, const bool_t visible)
{
    _oscontrol_set_visible(cast(edit, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osedit_enabled(OSEdit *edit, const bool_t enabled)
{
    cassert_no_null(edit);
    _osentry_enabled(edit->entry, enabled);
}

/*---------------------------------------------------------------------------*/

void osedit_size(const OSEdit *edit, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(edit, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osedit_origin(const OSEdit *edit, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(edit, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osedit_frame(OSEdit *edit, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(edit, OSControl), x, y, width, height);
    if (edit->tview != NULL)
        gtk_widget_set_size_request(edit->tview, (gint)width, (gint)height);
}

/*---------------------------------------------------------------------------*/

bool_t _osedit_autosel(const OSEdit *edit)
{
    cassert_no_null(edit);
    return _osentry_get_autoselect(edit->entry);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osedit_focus_widget(OSEdit *edit)
{
    cassert_no_null(edit);
    if (edit->tview != NULL)
        return edit->tview;
    return edit->control.widget;
}

/*---------------------------------------------------------------------------*/

bool_t _osedit_resign_focus(const OSEdit *edit)
{
    cassert_no_null(edit);
    return _osentry_resign_focus(edit->entry);
}

/*---------------------------------------------------------------------------*/

void _osedit_focus(OSEdit *edit, const bool_t focus)
{
    cassert_no_null(edit);
    _osentry_focus(edit->entry, focus);
}
