/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ospopup.c
 *
 */

/* Operating System native popup button */

#include "osglobals_gtk.inl"
#include "osgui_gtk.inl"
#include "oscontrol_gtk.inl"
#include "oscombo_gtk.inl"
#include "ospanel_gtk.inl"
#include "ospopup_gtk.inl"
#include "oswindow_gtk.inl"
#include "../ospopup.h"
#include "../ospopup.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/arrpt.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _ospopup_t
{
    OSControl control;
    uint32_t fsize;
    uint32_t natural_width;
    uint32_t natural_height;
    bool_t has_focus;
    GtkWidget *popup;
    GtkWidget *button;
    GtkCellRenderer *imgcell;
    GtkCellRenderer *txtcell;
    Font *font;
    GtkCssProvider *css_padding;
    GtkCssProvider *css_font;
    bool_t launch_event;
    Listener *OnSelect;
    ArrPt(String) *texts;
    ArrPt(Image) *images;
};

/*---------------------------------------------------------------------------*/

static const real32_t i_FILLER_LPADDING = 8.f;
static const real32_t i_FILLER_RPADDING = 16.f;
static const real32_t i_FILLER_IMAGE_SEP = 1.f;
static const uint32_t i_FILLER_WIDTH_IMAGE = 80;
static const uint32_t i_FILLER_WIDTH_NOIMAGE = 60;

/*---------------------------------------------------------------------------*/

static void i_OnSelect(GtkComboBox *widget, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert(popup->popup == GTK_WIDGET(widget));
    if (popup->launch_event == TRUE && popup->OnSelect != NULL)
    {
        EvButton params;
        params.state = ekGUI_ON;
        params.index = (uint32_t)gtk_combo_box_get_active(widget);
        params.text = NULL;
        listener_event(popup->OnSelect, ekGUI_EVENT_POPUP, popup, &params, NULL, OSPopUp, EvButton, void);
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_no_null(event);
    cassert_unref(widget == popup->control.widget, widget);
    cassert(GTK_IS_EVENT_BOX(widget) == TRUE);
    if (event->button.button == 1)
    {
        if (_oswindow_mouse_down(cast(popup, OSControl)) == TRUE)
            gtk_combo_box_popup(GTK_COMBO_BOX(popup->popup));
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnEnter(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_unref(widget == popup->control.widget, widget);
    unref(event);
    gtk_widget_set_state_flags(popup->button, GTK_STATE_FLAG_PRELIGHT, FALSE);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnLeave(GtkWidget *widget, GdkEvent *event, OSPopUp *popup)
{
    cassert_no_null(popup);
    cassert_unref(widget == popup->control.widget, widget);
    unref(event);
    gtk_widget_unset_state_flags(popup->button, GTK_STATE_FLAG_PRELIGHT);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPopupFillerDraw(GtkWidget *widget, cairo_t *cr, OSPopUp *popup)
{
    GtkStyleContext *ctx = _osglobals_button_context();
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    gint current;
    gint cellw = 0;
    const Image *image = NULL;
    const String *text = NULL;
    real32_t tx = i_FILLER_LPADDING;
    real32_t avail = (real32_t)w - i_FILLER_LPADDING - i_FILLER_RPADDING;
    cassert_no_null(popup);

    if (avail < 0.f)
        avail = 0.f;

    gtk_style_context_save(ctx);
    gtk_style_context_set_state(ctx, gtk_widget_get_state_flags(widget));
    gtk_render_background(ctx, cr, 0, 0, w, h);
    gtk_render_frame(ctx, cr, 0, 0, w, h);
    gtk_style_context_restore(ctx);

    current = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->popup));
    if (current != -1)
    {
        image = arrpt_get_const(popup->images, (uint32_t)current, Image);
        text = arrpt_get_const(popup->texts, (uint32_t)current, String);
    }

    /* Reserve the image slot whenever any item in the list carries an image, even if this one is NULL */
    gtk_cell_renderer_get_fixed_size(popup->imgcell, &cellw, NULL);
    if (cellw > 0)
    {
        real32_t slot = (real32_t)cellw;
        if (slot > avail)
            slot = avail;

        if (image != NULL)
        {
            GdkPixbuf *pixbuf = cast(image_native(image), GdkPixbuf);
            uint32_t niw = (uint32_t)image_width(image);
            uint32_t nih = (uint32_t)image_height(image);
            uint32_t dw = niw, dh = nih;
            real32_t x, y;

            /* Not even the icon fits at its natural size: scale it down, like Check/Radio */
            if (slot < (real32_t)niw || h < (int)nih)
            {
                real32_t scale = slot / (real32_t)niw;
                real32_t scaleh = (real32_t)h / (real32_t)nih;
                if (scaleh < scale)
                    scale = scaleh;

                dw = (uint32_t)((real32_t)niw * scale);
                dh = (uint32_t)((real32_t)nih * scale);
                if (dw < 1)
                    dw = 1;
                if (dh < 1)
                    dh = 1;
            }

            x = tx;
            y = ((real32_t)h - (real32_t)dh) / 2.f;

            if (dw != niw || dh != nih)
            {
                GdkPixbuf *scaled = gdk_pixbuf_scale_simple(pixbuf, (int)dw, (int)dh, GDK_INTERP_BILINEAR);
                gdk_cairo_set_source_pixbuf(cr, scaled, (double)x, (double)y);
                cairo_rectangle(cr, (double)x, (double)y, (double)dw, (double)dh);
                cairo_fill(cr);
                g_object_unref(scaled);
            }
            else
            {
                gdk_cairo_set_source_pixbuf(cr, pixbuf, (double)x, (double)y);
                cairo_rectangle(cr, (double)x, (double)y, (double)dw, (double)dh);
                cairo_fill(cr);
            }
        }

        tx += slot + i_FILLER_IMAGE_SEP;
        avail -= slot + i_FILLER_IMAGE_SEP;
        if (avail < 0.f)
            avail = 0.f;
    }

    if (avail > 0.f && text != NULL && str_empty(text) == FALSE)
    {
        real32_t ellw = 0.f, ellh = 0.f;
        font_extents(popup->font, "...", -1.f, &ellw, &ellh);

        if (avail >= ellw)
        {
            PangoLayout *layout = gtk_widget_create_pango_layout(widget, NULL);
            PangoRectangle rect;
            GdkRGBA color;
            pango_layout_set_font_description(layout, cast(font_native(popup->font), PangoFontDescription));
            pango_layout_set_text(layout, tc(text), -1);
            pango_layout_set_width(layout, (int)(avail * (real32_t)PANGO_SCALE));
            pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
            pango_layout_get_pixel_extents(layout, NULL, &rect);

            _oscontrol_to_gdkrgba((color_t)ekSYSCOLOR_LABEL, &color);
            cairo_save(cr);
            cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
            cairo_move_to(cr, (double)tx, (double)(((real32_t)h - (real32_t)rect.height) / 2.f));
            pango_cairo_show_layout(cr, layout);
            cairo_restore(cr);
            g_object_unref(layout);
        }
    }

    if (_ospopup_has_focus(popup) == TRUE)
    {
        gtk_style_context_save(ctx);
        gtk_render_focus(ctx, cr, 0, 0, w, h);
        gtk_style_context_restore(ctx);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPopupFillerPressed(GtkWidget *widget, GdkEventButton *event, OSPopUp *popup)
{
    unref(widget);
    unref(event);
    if (_oswindow_mouse_down(cast(popup, OSControl)) == TRUE)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static GtkWidget *i_popup_filler(OSControl *control)
{
    OSPopUp *popup = cast(control, OSPopUp);
    GtkWidget *filler = gtk_drawing_area_new();
    gtk_widget_set_can_focus(filler, TRUE);
    gtk_widget_add_events(filler, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(filler), "draw", G_CALLBACK(i_OnPopupFillerDraw), (gpointer)popup);
    g_signal_connect(G_OBJECT(filler), "button-press-event", G_CALLBACK(i_OnPopupFillerPressed), (gpointer)popup);
    return filler;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnFocusDrawAfter(GtkWidget *widget, cairo_t *cr, OSPopUp *popup)
{
    cassert_no_null(popup);
    if (_ospopup_has_focus(popup) == TRUE)
    {
        GtkStyleContext *ctx = gtk_widget_get_style_context(widget);
        GtkStateFlags state = gtk_style_context_get_state(ctx) | GTK_STATE_FLAG_FOCUSED;
        int w = gtk_widget_get_allocated_width(widget);
        int h = gtk_widget_get_allocated_height(widget);
        gtk_style_context_save(ctx);
        gtk_style_context_set_state(ctx, state);
        gtk_render_focus(ctx, cr, 0, 0, w, h);
        gtk_style_context_restore(ctx);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

OSPopUp *ospopup_create(const uint32_t flags)
{
    OSPopUp *popup = heap_new0(OSPopUp);
    GtkWidget *widget = gtk_combo_box_new();
    Font *font = _osgui_create_default_font();
    const char_t *cssobj = _osglobals_css_combobox();
    cassert_unref(flags == ekPOPUP_FLAG, flags);

#if GTK_CHECK_VERSION(3, 16, 0)
    /* GtkCellView->GtkBox->GtkToggleButton */
    popup->button = gtk_bin_get_child(GTK_BIN(widget));
    popup->button = gtk_widget_get_parent(gtk_widget_get_parent(popup->button));
    cassert(GTK_IS_TOGGLE_BUTTON(popup->button));
#else
    popup->button = widget;
#endif

    popup->imgcell = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), popup->imgcell, FALSE);
    gtk_cell_renderer_set_alignment(popup->imgcell, 0.f, .5);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), popup->imgcell, "pixbuf", 0, NULL);
    gtk_cell_renderer_set_padding(popup->imgcell, 0, 0);
    popup->txtcell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_end(GTK_CELL_LAYOUT(widget), popup->txtcell, TRUE);
    gtk_cell_renderer_set_alignment(popup->txtcell, 0.f, .5);
    gtk_cell_renderer_set_padding(popup->txtcell, 0, 0);
    g_object_set(popup->txtcell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), popup->txtcell, "text", 1, NULL);
    g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(i_OnSelect), (gpointer)popup);

    /*
      Encapsulate the popup widget in an event box
      We need to capture the 'button-press-event' and is not possible doing directly in GtkComboBox
      https://stackoverflow.com/questions/9145741/how-to-detect-mouse-click-in-gtkcombobox
    */
    popup->popup = widget;
    widget = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(widget), popup->popup);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(widget), TRUE);
    gtk_widget_add_events(widget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)popup);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(i_OnEnter), (gpointer)popup);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(i_OnLeave), (gpointer)popup);
    gtk_widget_show(popup->popup);

    g_signal_connect_after(G_OBJECT(popup->button), "draw", G_CALLBACK(i_OnFocusDrawAfter), (gpointer)popup);
    _oscontrol_init(&popup->control, ekGUI_TYPE_POPUP, widget, popup->button, TRUE);
    _oscontrol_update_css_font(popup->popup, cssobj, font, &popup->font, &popup->css_font);
    _oscontrol_update_css_padding(popup->button, _osglobals_css_button(), kPOPUP_VPADDING, kPOPUP_HPADDING, &popup->css_padding);
    popup->fsize = (uint32_t)(font_size(font) + 2.5f);
    font_destroy(&font);
    popup->launch_event = TRUE;
    popup->natural_width = UINT32_MAX;
    popup->texts = arrpt_create(String);
    popup->images = arrpt_create(Image);
    _oscontrol_squeeze_filler(&popup->control, i_popup_filler);
    _oscontrol_squeeze_fixed_width(&popup->control, i_FILLER_WIDTH_NOIMAGE);
    return popup;
}

/*---------------------------------------------------------------------------*/

static void i_img_dest(Image **image)
{
    cassert_no_null(image);
    if (*image != NULL)
        image_destroy(image);
}

/*---------------------------------------------------------------------------*/

void ospopup_destroy(OSPopUp **popup)
{
    cassert_no_null(popup);
    cassert_no_null(*popup);
    listener_destroy(&(*popup)->OnSelect);
    arrpt_destroy(&(*popup)->texts, str_destroy, String);
    arrpt_destroy(&(*popup)->images, i_img_dest, Image);
    font_destroy(&(*popup)->font);
    _oscontrol_destroy_css_provider(&(*popup)->css_padding);
    _oscontrol_destroy_css_provider(&(*popup)->css_font);
    _oscontrol_destroy(*dcast(popup, OSControl));
    heap_delete(popup, OSPopUp);
}

/*---------------------------------------------------------------------------*/

void ospopup_OnSelect(OSPopUp *popup, Listener *listener)
{
    cassert_no_null(popup);
    listener_update(&popup->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ospopup_tooltip(OSPopUp *popup, const char_t *text)
{
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    gtk_widget_set_tooltip_text(popup->popup, cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

void ospopup_font(OSPopUp *popup, const Font *font)
{
    const char_t *cssobj = _osglobals_css_combobox();
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    _oscontrol_update_css_font(popup->popup, cssobj, font, &popup->font, &popup->css_font);
    popup->fsize = (uint32_t)(font_size(font) + 2.5f);
}

/*---------------------------------------------------------------------------*/

void ospopup_elem(OSPopUp *popup, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    uint32_t imgw, imgh;
    gint current;
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    popup->natural_width = UINT32_MAX;
    current = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->popup));
    _oscombo_elem(GTK_COMBO_BOX(popup->popup), op, index, text, image, popup->texts, popup->images, &imgw, &imgh);

    if (imgh < popup->fsize)
        imgh = popup->fsize;

    gtk_cell_renderer_set_fixed_size(popup->txtcell, -1, (gint)imgh);
    gtk_cell_renderer_set_fixed_size(popup->imgcell, (gint)imgw, (gint)imgh);
    _oscontrol_squeeze_fixed_width(&popup->control, imgw > 0 ? i_FILLER_WIDTH_IMAGE : i_FILLER_WIDTH_NOIMAGE);

    if (current == -1)
        current = 0;

    popup->launch_event = FALSE;
    gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), current);
    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void ospopup_list_height(OSPopUp *popup, const uint32_t num_elems)
{
    unref(popup);
    unref(num_elems);
}

/*---------------------------------------------------------------------------*/

void ospopup_selected(OSPopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    popup->launch_event = FALSE;

    if (index != UINT32_MAX)
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), (gint)index);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(popup->popup), -1);

    popup->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t ospopup_get_selected(const OSPopUp *popup)
{
    gint sel;
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    sel = gtk_combo_box_get_active(GTK_COMBO_BOX(popup->popup));
    return sel == -1 ? UINT32_MAX : (uint32_t)sel;
}

/*---------------------------------------------------------------------------*/

void ospopup_bounds(const OSPopUp *popup, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    cassert_no_null(popup);
    cassert(GTK_IS_EVENT_BOX(popup->control.widget));
    cassert_no_null(width);
    cassert_no_null(height);
    unref(text);
    unref(refwidth);

    if (popup->natural_width == UINT32_MAX && popup->control.filler.active == FALSE)
    {
        GtkRequisition natural;
        gtk_widget_set_size_request(popup->control.widget, -1, -1);
        gtk_widget_get_preferred_size(popup->control.widget, NULL, &natural);
        cast(popup, OSPopUp)->natural_width = (uint32_t)natural.width;
        cast(popup, OSPopUp)->natural_height = (uint32_t)natural.height;
    }

    if (popup->natural_width != UINT32_MAX)
    {
        *width = (real32_t)popup->natural_width;
        *height = (real32_t)popup->natural_height;
    }
    else
    {
        cassert(popup->control.filler.active == TRUE);
        *width = (real32_t)popup->control.filler.width;
        *height = (real32_t)popup->control.filler.height;
    }
}

/*---------------------------------------------------------------------------*/

void ospopup_attach(OSPopUp *popup, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(popup, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospopup_detach(OSPopUp *popup, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(popup, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospopup_visible(OSPopUp *popup, const bool_t visible)
{
    _oscontrol_set_visible(cast(popup, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ospopup_enabled(OSPopUp *popup, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(popup, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ospopup_size(const OSPopUp *popup, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(popup, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ospopup_origin(const OSPopUp *popup, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(popup, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ospopup_frame(OSPopUp *popup, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(popup);
    _oscontrol_set_frame(cast(popup, OSControl), x, y, width, height);
    gtk_widget_set_size_request(popup->popup, (gint)width, (gint)height);
    gtk_widget_set_size_request(popup->button, (gint)width, (gint)height);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_ospopup_focus_widget(OSPopUp *popup)
{
    cassert_no_null(popup);
    if (popup->control.filler.active == TRUE)
        return popup->control.filler.widget;
    return popup->button;
}

/*---------------------------------------------------------------------------*/

bool_t _ospopup_has_focus(const OSPopUp *popup)
{
    cassert_no_null(popup);
    return popup->has_focus;
}

/*---------------------------------------------------------------------------*/

void _ospopup_focus(OSPopUp *popup, const bool_t focus)
{
    cassert_no_null(popup);
    popup->has_focus = focus;
    if (popup->control.filler.widget != NULL)
        gtk_widget_queue_draw(popup->control.filler.widget);
}
