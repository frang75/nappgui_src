/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostext.c
 *
 */

/* Operating System native text view */

#include "ostext.h"
#include "ostext.inl"
#include "osglobals.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "ossplit.inl"
#include "cassert.h"
#include "color.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "strings.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _ostext_t
{
    OSControl control;
    char_t family[64];
    real32_t size;
    uint32_t units;
    uint32_t style;
    color_t color;
    color_t bgcolor;
    align_t align;
    gint lspacing;
    gint bfspace;
    gint afspace;
    GtkTextTag *curtag;
    GtkWidget *tview;
    GtkTextBuffer *buffer;
    GtkCssProvider *pgcolor;
    OSControl *capture;
    Listener *OnChange;
};

static real32_t i_PANGO_TO_PIXELS = -1;

/*---------------------------------------------------------------------------*/

static real32_t i_device_to_pixels(void)
{
    if (i_PANGO_TO_PIXELS < 0)
    {
        /* This object is owned by Pango and must not be freed */
        PangoFontMap *fontmap = pango_cairo_font_map_get_default();
        real32_t dpi = (real32_t)pango_cairo_font_map_get_resolution((PangoCairoFontMap*)fontmap);
        i_PANGO_TO_PIXELS = (dpi / 72.f) / PANGO_SCALE;
    }

    return i_PANGO_TO_PIXELS;
}

/*---------------------------------------------------------------------------*/

static void i_OnTViewDestroy(GtkWidget *obj, gpointer data)
{
    OSControl *control = (OSControl*)data;
    cassert_no_null(obj);
    unref(obj);
    unref(data);
    unref(control);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSText *view)
{
    unref(widget);
    if (view->capture != NULL)
    {
        if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
        {
            _ossplit_OnPress((OSSplit*)view->capture, event);

        }

        return TRUE;
    }
    /*
    The handler will be called before the default handler of the signal.
    This is the default behaviour
    */
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

OSText *ostext_create(const uint32_t flags)
{
    OSText *view = heap_new0(OSText);
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *top = NULL;
    GtkWidget *focus = NULL;
    unref(flags);
    view->family[0] = '\0';
    view->size = 1e10f;
    view->units = UINT32_MAX;
    view->style = UINT32_MAX;
    view->color = kCOLOR_DEFAULT;
    view->bgcolor = kCOLOR_DEFAULT;
    view->align = ENUM_MAX(align_t);
    view->lspacing = 0;
    view->bfspace = 0;
    view->afspace = 0;
    view->curtag = NULL;
    view->tview = gtk_text_view_new();
    g_signal_connect(view->tview, "destroy", G_CALLBACK(i_OnTViewDestroy), (gpointer)view);

    /* A parent widget can "capture" the mouse */
    {
        GtkWidget *vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(scrolled));
        GtkWidget *hscroll = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(scrolled));
        g_signal_connect(G_OBJECT(vscroll), "button-press-event", G_CALLBACK(i_OnPressed), view);
        g_signal_connect(G_OBJECT(hscroll), "button-press-event", G_CALLBACK(i_OnPressed), view);
        g_signal_connect(G_OBJECT(scrolled), "button-press-event", G_CALLBACK(i_OnPressed), view);
        g_signal_connect(G_OBJECT(view->tview), "button-press-event", G_CALLBACK(i_OnPressed), view);
    }

    /* g_signal_connect(G_OBJECT(vscroll), "button-press-event", G_CALLBACK(i_OnPressed), panel); */

    gtk_widget_show(view->tview);
    view->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view->tview));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view->tview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view->tview),GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), view->tview);
    top = scrolled;
    focus = scrolled;

    /* Creating the frame (border) view */
    {
        GtkWidget *frame = gtk_frame_new(NULL);
        cassert(gtk_widget_get_has_window(frame) == FALSE);
        gtk_container_add(GTK_CONTAINER(frame), top);
        gtk_widget_show(top);
        g_object_set_data(G_OBJECT(scrolled), "OSControl", &view->control);
        top = frame;
    }

    _oscontrol_init(&view->control, ekGUI_TYPE_TEXTVIEW, top, focus, TRUE);
    return view;
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_scrolled_window(const OSText *view)
{
    GtkWidget *scrolled = NULL;
    cassert_no_null(view);

    scrolled = view->control.widget;

    {
        cassert(GTK_IS_FRAME(scrolled) == TRUE);
        scrolled = gtk_bin_get_child(GTK_BIN(scrolled));
    }

    cassert(GTK_IS_SCROLLED_WINDOW(scrolled));
    return scrolled;
}

/*---------------------------------------------------------------------------*/

void ostext_destroy(OSText **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    listener_destroy(&(*view)->OnChange);
    gtk_container_remove(GTK_CONTAINER(i_scrolled_window(*view)), (*view)->tview);
    _oscontrol_destroy(*(OSControl**)view);
    heap_delete(view, OSText);
}

/*---------------------------------------------------------------------------*/

void ostext_OnTextChange(OSText *view, Listener *listener)
{
    unref(view);
    unref(listener);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

static __INLINE gint i_size_pango(const real32_t size, const uint32_t units)
{
    static gint val = 0;
    if ((units & ekFPOINTS) == ekFPOINTS)
    {
        val = (gint)(size * (real32_t)PANGO_SCALE);
    }
    else
    {
        cassert((units & ekFPIXELS) == ekFPIXELS);
        val = (gint)(size / i_device_to_pixels());
    }

    return val;
}

/*---------------------------------------------------------------------------*/

void ostext_insert_text(OSText *view, const char_t *text)
{
    GtkTextIter end;
    GtkTextTag *tag = NULL;

    cassert_no_null(view);
    cassert_no_null(text);

    if (view->curtag == NULL)
    {
        GValue gtrue = G_VALUE_INIT;
        tag = gtk_text_buffer_create_tag(view->buffer, NULL, "family-set", TRUE, "size-set", TRUE, NULL);
        g_value_init(&gtrue, G_TYPE_BOOLEAN);
        g_value_set_boolean(&gtrue, TRUE);

        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_STRING);
            g_value_set_string(&gvalue, (gchar*)view->family);
            g_object_set_property(G_OBJECT(tag), "family", &gvalue);
            g_value_unset(&gvalue);
        }

        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, i_size_pango(view->size, view->units));
            g_object_set_property(G_OBJECT(tag), "size", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->style & ekFBOLD)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, PANGO_WEIGHT_BOLD);
            g_object_set_property(G_OBJECT(tag), "weight-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "weight", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->style & ekFITALIC)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, PANGO_TYPE_STYLE);
            g_value_set_enum(&gvalue, PANGO_STYLE_ITALIC);
            g_object_set_property(G_OBJECT(tag), "style-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "style", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->style & ekFUNDERLINE)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, PANGO_TYPE_UNDERLINE);
            g_value_set_enum(&gvalue, PANGO_UNDERLINE_SINGLE);
            g_object_set_property(G_OBJECT(tag), "underline-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "underline", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->style & ekFSTRIKEOUT)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_BOOLEAN);
            g_value_set_boolean(&gvalue, TRUE);
            g_object_set_property(G_OBJECT(tag), "strikethrough-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "strikethrough", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->color != kCOLOR_DEFAULT)
        {
            GdkRGBA gdkcolor;
            GValue gvalue = G_VALUE_INIT;
            _oscontrol_to_gdkrgba(view->color, &gdkcolor);
            g_value_init(&gvalue, GDK_TYPE_RGBA);
            g_value_set_boxed(&gvalue, &gdkcolor);
            g_object_set_property(G_OBJECT(tag), "foreground-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "foreground-rgba", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->bgcolor != kCOLOR_DEFAULT)
        {
            GdkRGBA gdkcolor;
            GValue gvalue = G_VALUE_INIT;
            _oscontrol_to_gdkrgba(view->bgcolor, &gdkcolor);
            g_value_init(&gvalue, GDK_TYPE_RGBA);
            g_value_set_boxed(&gvalue, &gdkcolor);
            g_object_set_property(G_OBJECT(tag), "background-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "background-rgba", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->align != ekLEFT)
        {
            GtkJustification justif = GTK_JUSTIFY_LEFT;
            GValue gvalue = G_VALUE_INIT;
            switch(view->align) {
            case ekLEFT:
                justif = GTK_JUSTIFY_LEFT;
                break;
            case ekRIGHT:
                justif = GTK_JUSTIFY_RIGHT;
                break;
            case ekCENTER:
                justif = GTK_JUSTIFY_CENTER;
                break;
            case ekJUSTIFY:
                justif = GTK_JUSTIFY_FILL;
                break;
            cassert_default();
            }

            g_value_init(&gvalue, GTK_TYPE_JUSTIFICATION);
            g_value_set_enum(&gvalue, justif);
            g_object_set_property(G_OBJECT(tag), "justification-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "justification", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->lspacing != 0)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, view->lspacing);
            g_object_set_property(G_OBJECT(tag), "pixels-inside-wrap-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "pixels-inside-wrap", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->bfspace > 0)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, view->bfspace);
            g_object_set_property(G_OBJECT(tag), "pixels-above-lines-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "pixels-above-lines", &gvalue);
            g_value_unset(&gvalue);
        }

        if (view->afspace > 0)
        {
            GValue gvalue = G_VALUE_INIT;
            g_value_init(&gvalue, G_TYPE_INT);
            g_value_set_int(&gvalue, view->afspace);
            g_object_set_property(G_OBJECT(tag), "pixels-below-lines-set", &gtrue);
            g_object_set_property(G_OBJECT(tag), "pixels-below-lines", &gvalue);
            g_value_unset(&gvalue);
        }

        g_value_unset(&gtrue);
        view->curtag = tag;
    }
    else
    {
        tag = view->curtag;
    }

    gtk_text_buffer_get_end_iter(view->buffer, &end);
    gtk_text_buffer_insert_with_tags(view->buffer, &end, (const gchar*)text, -1, tag, NULL);
}

/*---------------------------------------------------------------------------*/

void ostext_set_text(OSText *view, const char_t *text)
{
    GtkTextBuffer *buffer;
    cassert_no_null(view);
    cassert_no_null(text);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view->tview));
    gtk_text_buffer_set_text (buffer, (const gchar*)text, -1);
}

/*---------------------------------------------------------------------------*/

void ostext_set_rtf(OSText *view, Stream *rtf_in)
{
    unref(view);
    unref(rtf_in);
}

/*---------------------------------------------------------------------------*/

void ostext_property(OSText *view, const gui_prop_t prop, const void *value)
{
    cassert_no_null(view);
    switch (prop) {
    case ekGUI_PROP_FAMILY:
        if (str_cmp_c(view->family, (const char_t*)value) != 0)
        {
            str_copy_c(view->family, sizeof(view->family), (const char_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_UNITS:
        if (view->units != *((const uint32_t*)value))
        {
            view->units = *((const uint32_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_SIZE:
        if (view->size != *((real32_t*)value))
        {
            view->size = *((real32_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_STYLE:
        if (view->style != *((uint32_t*)value))
        {
            view->style = *((uint32_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_COLOR:
        if (view->color != *((color_t*)value))
        {
            view->color = *((color_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_BGCOLOR:
        if (view->bgcolor != *((color_t*)value))
        {
            view->bgcolor = *((color_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_PGCOLOR:
        if (view->pgcolor != NULL)
        {
            _oscontrol_remove_provider(view->tview, view->pgcolor);
            view->pgcolor = NULL;
        }

        if (*(color_t*)value != kCOLOR_DEFAULT)
            _oscontrol_widget_bg_color(view->tview, "textview", *(color_t*)value, &view->pgcolor);
        break;

    case ekGUI_PROP_PARALIGN:
        if (view->align != *((align_t*)value))
        {
            view->align = *((align_t*)value);
            view->curtag = NULL;
        }
        break;

    case ekGUI_PROP_LSPACING:
    {
        real32_t spacing = *((real32_t*)value);
        gint lspacing = 0;
        if (spacing > 1)
        {
            Font *font = font_create(view->family, view->size, view->style | view->units);
            real32_t cell_size = font_height(font);
            font_destroy(&font);
            lspacing = (spacing - 1) * cell_size;
        }

        if (view->lspacing != lspacing)
        {
            view->lspacing = lspacing;
            view->curtag = NULL;
        }

        break;
    }

    case ekGUI_PROP_AFPARSPACE:
    {
        gint lspace = i_size_pango(*((real32_t*)value), view->units) / PANGO_SCALE;
        if (lspace >= 0 && lspace != view->afspace)
        {
            view->afspace = lspace;
            view->curtag = NULL;
        }

        break;
    }

    case ekGUI_PROP_BFPARSPACE:
    {
        gint lspace = i_size_pango(*((real32_t*)value), view->units) / PANGO_SCALE;
        if (lspace >= 0 && lspace != view->bfspace)
        {
            view->bfspace = lspace;
            view->curtag = NULL;
        }

        break;
    }

    case ekGUI_PROP_VSCROLL:
    {
        GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(view->control.widget));
        register gdouble max = gtk_adjustment_get_upper(vadj);
        gtk_adjustment_set_value(vadj, max);
        gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(view->control.widget), vadj);
        break;
    }

    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void ostext_editable(OSText *view, const bool_t is_editable)
{
    cassert_no_null(view);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view->tview), (gboolean)is_editable);
}

/*---------------------------------------------------------------------------*/

const char_t *ostext_get_text(const OSText *view)
{
	cassert(FALSE);
    unref(view);
	return NULL;
}

/*---------------------------------------------------------------------------*/

void ostext_set_need_display(OSText *view)
{
	unref(view);
	cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void ostext_attach(OSText *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_detach(OSText *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ostext_visible(OSText *view, const bool_t is_visible)
{
    cassert_no_null(view);
    _oscontrol_set_visible((OSControl*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void ostext_enabled(OSText *view, const bool_t is_enabled)
{
    cassert_no_null(view);
    _oscontrol_set_enabled((OSControl*)view, is_enabled);
    gtk_widget_set_sensitive(view->tview, (gboolean)is_enabled);
}

/*---------------------------------------------------------------------------*/

void ostext_size(const OSText *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ostext_origin(const OSText *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ostext_frame(OSText *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ostext_detach_and_destroy(OSText **view, OSPanel *panel)
{
    cassert_no_null(view);
    ostext_detach(*view, panel);
    ostext_destroy(view);
}

/*---------------------------------------------------------------------------*/

void _ostext_set_capture(OSText *view, OSControl *control)
{
    cassert_no_null(view);
    view->capture = control;
}

/*---------------------------------------------------------------------------*/

void _ostext_release_capture(OSText *view)
{
    cassert_no_null(view);
    view->capture = NULL;
}
