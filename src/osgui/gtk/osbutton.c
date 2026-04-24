/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.c
 *
 */

/* Operating System native button */

#include "osgui_gtk.inl"
#include "osglobals_gtk.inl"
#include "osbutton_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osbutton.h"
#include "../osbutton.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

struct _osbutton_t
{
    OSControl control;
    button_flag_t flags;
    bool_t launch_event;
    bool_t is_default;
    uint32_t vpadding;
    uint32_t hpadding;
    real32_t twidth;
    real32_t theight;
    gui_pos_t imgpos;
    GtkWidget *radio;
    Font *font;
    String *text;
    String *markup;
    Image *image;
    GtkCssProvider *css_padding;
    GtkCssProvider *css_font;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

static const real32_t i_PUSHBUTTON_EXTRAWIDTH = 2.f;
static const real32_t i_CHECKBOX_EXTRAHEIGHT = 2.f;
static real32_t i_BUTTON_IMAGE_SEP = 4.f;

/*---------------------------------------------------------------------------*/

static gui_state_t i_get_state(const OSButton *button)
{
    cassert_no_null(button);
    switch (button_get_type(button->flags))
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_FLAT:
        return ekGUI_ON;

    case ekBUTTON_RADIO:
    case ekBUTTON_CHECK2:
    {
        gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button->control.widget));
        return active == TRUE ? ekGUI_ON : ekGUI_OFF;
    }

    case ekBUTTON_CHECK3:
        if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget)) == TRUE)
        {
            return ekGUI_MIXED;
        }
        else
        {
            gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button->control.widget));
            return active == TRUE ? ekGUI_ON : ekGUI_OFF;
        }

    case ekBUTTON_FLATGLE:
    {
        gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button->control.widget));
        return active == TRUE ? ekGUI_ON : ekGUI_OFF;
    }

    default:
        cassert_default(button_get_type(button->flags));
    }

    return ekGUI_OFF;
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(GtkWidget *widget, OSButton *button)
{
    cassert_no_null(button);
    cassert_unref(widget == button->control.widget, widget);
    if (button_get_type(button->flags) == ekBUTTON_CHECK3)
    {
        if (gtk_toggle_button_get_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget)) == TRUE)
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget), FALSE);
    }

    if (button->launch_event == TRUE && button->OnClick != NULL)
    {
        EvButton params;
        params.index = 0;
        params.state = i_get_state(button);
        params.text = NULL;
        listener_event(button->OnClick, ekGUI_EVENT_BUTTON, button, &params, NULL, OSButton, EvButton, void);
    }

    _oswindow_release_transient_focus(cast(button, OSControl));
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnKeyPress(GtkWidget *widget, GdkEventKey *event, OSButton *button)
{
    guint key = 0;
    cassert_no_null(event);
    unref(widget);
    unref(button);

    key = event->keyval;

    /* Avoid the "DUMMY" GTK button navigation using arrows */
    if (key == GDK_KEY_Left || key == GDK_KEY_Right || key == GDK_KEY_Down || key == GDK_KEY_Up)
        return TRUE;

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSButton *button)
{
    unref(widget);
    unref(event);
    if (_oswindow_mouse_down(cast(button, OSControl)) == TRUE)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPushDraw(GtkWidget *widget, cairo_t *cr, OSButton *button)
{
    unref(cr);
    unref(button);
    if (button->is_default == TRUE)
        gtk_widget_set_state_flags(widget, GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_FOCUSED, FALSE);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_update_text_extents(OSButton *button)
{
    cassert_no_null(button);
    if (button->twidth < 0.f)
    {
        if (str_empty(button->text) == FALSE)
        {
            font_extents(button->font, tc(button->text), -1.f, &button->twidth, &button->theight);
            button->twidth = bmath_ceilf(button->twidth);
            button->theight = bmath_ceilf(button->theight);
        }
        else
        {
            button->twidth = 0.f;
            button->theight = 0.f;
        }
    }
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnButtonDraw(GtkWidget *widget, cairo_t *cr, OSButton *button)
{
    PangoLayout *layout = NULL;
    real32_t bwidth = 0, bheight = 0;
    bool_t draw_text = TRUE;
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);

    _oscontrol_widget_get_size(button->control.widget, &bwidth, &bheight);

    if (button_is_flat(button->flags) == TRUE && (str_empty(button->text) == TRUE || button->imgpos == ekGUI_POS_NONE))
        draw_text = FALSE;

    if (draw_text == TRUE)
        i_update_text_extents(button);

    if (draw_text == TRUE)
    {
        GdkRGBA color;
        layout = gtk_widget_create_pango_layout(widget, NULL);
        pango_layout_set_font_description(layout, cast(font_native(button->font), PangoFontDescription));
        _oscontrol_to_gdkrgba(ekSYSCOLOR_LABEL, &color);
        cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
        pango_layout_set_markup(layout, tc(button->markup), -1);
    }

    /* Draw PushButton text with font scaling */
    if (button_get_type(button->flags) == ekBUTTON_PUSH)
    {
        /* In pushbuttons, we have to center the content */
        real32_t cwidth = draw_text == TRUE ? button->twidth + i_PUSHBUTTON_EXTRAWIDTH : 0.f;
        cairo_save(cr);

        if (button->image != NULL)
        {
            real32_t imgwidth = (real32_t)image_width(button->image);
            if (draw_text == TRUE)
            {
                cwidth += imgwidth + (real32_t)kBUTTON_IMAGE_SEP;
                cairo_translate(cr, imgwidth + (real32_t)kBUTTON_IMAGE_SEP, 0);
            }
            else
            {
                cwidth = imgwidth;
            }
        }

        cairo_translate(cr, (bwidth - cwidth) / 2, ((bheight - (draw_text == TRUE ? button->theight : 0.f) - 4) / 2));
        cairo_scale(cr, font_xscale(button->font), 1);
        cairo_move_to(cr, 0, 0);
        pango_cairo_show_layout(cr, layout);

        /* Draw the pushbutton image. I don't know why, if I render the image first, text is not shown */
        if (button->image != NULL)
        {
            GdkPixbuf *pixbuf = cast(image_native(button->image), GdkPixbuf);
            real32_t imgw = (real32_t)image_width(button->image);
            real32_t imgh = (real32_t)image_height(button->image);
            real32_t xpos = (draw_text == TRUE) ? -(imgw + (real32_t)kBUTTON_IMAGE_SEP) : 0.f;
            real32_t ypos = (draw_text == TRUE) ? (button->theight - imgh) / 2 : 0.f;
            gdk_cairo_set_source_pixbuf(cr, pixbuf, xpos, ypos);
            cairo_paint(cr);
        }

        cairo_restore(cr);
    }
    else if (button_is_flat(button->flags) == TRUE)
    {
        real32_t imgwidth = 0.f;
        real32_t imgheight = 0.f;
        real32_t imgsep = 0.f;
        real32_t imgx = 0.f;
        real32_t imgy = 0.f;
        real32_t tx = 0.f;
        real32_t ty = 0.f;

        if (button->image != NULL)
        {
            imgwidth = (real32_t)image_width(button->image);
            imgheight = (real32_t)image_height(button->image);
            imgsep = i_BUTTON_IMAGE_SEP;
        }

        _osbutton_flat_position(bwidth, bheight, imgwidth, imgheight, imgsep, button->imgpos, button->twidth, button->theight, &imgx, &imgy, &tx, &ty);

        if (draw_text == TRUE)
        {
            cairo_save(cr);
            cairo_translate(cr, tx, ty);
            cairo_move_to(cr, 0, 0);
            pango_cairo_show_layout(cr, layout);
            cairo_restore(cr);
        }

        if (imgwidth > 0.f)
        {
            GdkPixbuf *pixbuf = cast(image_native(button->image), GdkPixbuf);
            gdk_cairo_set_source_pixbuf(cr, pixbuf, imgx, imgy);
            cairo_paint(cr);
        }
    }

    if (layout != NULL)
        g_object_unref(layout);

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_css_obj(const uint32_t flags)
{
    /* Initial padding for button */
    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        return _osglobals_css_button();
    case ekBUTTON_RADIO:
        return _osglobals_css_radio();
    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
        return _osglobals_css_check();
    default:
        cassert_default(button_get_type(flags));
    }

    return "";
}

/*---------------------------------------------------------------------------*/

OSButton *osbutton_create(const uint32_t flags)
{
    OSButton *button = heap_new0(OSButton);
    GtkWidget *widget = NULL;
    GtkWidget *focus_widget = NULL;
    const char_t *cssobj = NULL;
    button->flags = flags;
    button->twidth = -1.f;
    button->theight = -1.f;
    button->imgpos = ekGUI_POS_NONE;

    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
        widget = gtk_button_new_with_label("");
        gtk_button_set_use_underline(GTK_BUTTON(widget), TRUE);
        g_signal_connect(widget, "draw", G_CALLBACK(i_OnPushDraw), button);
        g_signal_connect_after(widget, "draw", G_CALLBACK(i_OnButtonDraw), button);
        focus_widget = widget;
        break;

    case ekBUTTON_FLAT:
        widget = gtk_button_new_with_label("");
        gtk_button_set_use_underline(GTK_BUTTON(widget), TRUE);
        gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
        gtk_style_context_add_class(gtk_widget_get_style_context(widget), GTK_STYLE_CLASS_FLAT);
        g_signal_connect_after(widget, "draw", G_CALLBACK(i_OnButtonDraw), button);
        focus_widget = widget;
        break;

    case ekBUTTON_FLATGLE:
        widget = gtk_toggle_button_new_with_label("");
        gtk_button_set_use_underline(GTK_BUTTON(widget), TRUE);
        gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
        gtk_style_context_add_class(gtk_widget_get_style_context(widget), GTK_STYLE_CLASS_FLAT);
        g_signal_connect_after(widget, "draw", G_CALLBACK(i_OnButtonDraw), button);
        focus_widget = widget;
        break;

    case ekBUTTON_RADIO:
        widget = gtk_radio_button_new_with_label(NULL, "");
        gtk_button_set_use_underline(GTK_BUTTON(widget), TRUE);
        button->radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(widget), "");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->radio), TRUE);
        focus_widget = widget;
        break;

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
        widget = gtk_check_button_new_with_label("");
        gtk_button_set_use_underline(GTK_BUTTON(widget), TRUE);
        focus_widget = widget;
        break;
    default:
        cassert_default(button_get_type(flags));
    }

    cssobj = i_css_obj(button->flags);
    g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(i_OnClick), (gpointer)button);
    _oscontrol_init(&button->control, ekGUI_TYPE_BUTTON, widget, focus_widget, FALSE);

    /*
     * Set CSS padding to 0. Like Windows, the button padding in the difference
     * between button frame and content size.
     */
    button->vpadding = kBUTTON_VPADDING;
    button->hpadding = kBUTTON_HPADDING;
    _oscontrol_update_css_padding(focus_widget, cssobj, 0, 0, &button->css_padding);

    /*
     * We set the button font infinitely small for two reasons.
     * 1) That the button text does not influence its minimum size, allowing to create buttons of any size.
     * 2) Ability to assign text with mnemonics, maintain the GTK shortcuts support.
     */
    if (_osbutton_text_allowed(flags) == TRUE)
    {
        Font *fake_font = font_system(0, 0);
        Font *ffont = NULL;
        _oscontrol_update_css_font(button->control.widget, cssobj, fake_font, &ffont, &button->css_font);
        font_destroy(&fake_font);
        ptr_destopt(font_destroy, &ffont, Font);
        /* The real font and text for button */
        button->text = str_c("");
        button->markup = str_c("");
        button->font = _osgui_create_default_font();
    }

    g_signal_connect(G_OBJECT(focus_widget), "button-press-event", G_CALLBACK(i_OnPressed), (gpointer)button);
    g_signal_connect(G_OBJECT(focus_widget), "key-press-event", G_CALLBACK(i_OnKeyPress), (gpointer)button);
    button->launch_event = TRUE;
    return button;
}

/*---------------------------------------------------------------------------*/

void osbutton_destroy(OSButton **button)
{
    cassert_no_null(button);
    cassert_no_null(*button);

    if ((*button)->radio != NULL)
    {
        /*
        A floating object was finalized. This means that someone
        called g_object_unref() on an object that had only a floating
        reference; the initial floating reference is not owned by anyone
        and must be removed with g_object_ref_sink().
        */
        g_object_ref_sink((*button)->radio);
        g_object_unref((*button)->radio);
    }

    listener_destroy(&(*button)->OnClick);
    str_destopt(&(*button)->text);
    str_destopt(&(*button)->markup);
    ptr_destopt(image_destroy, &(*button)->image, Image);
    ptr_destopt(font_destroy, &(*button)->font, Font);
    _oscontrol_destroy_css_provider(&(*button)->css_padding);
    _oscontrol_destroy_css_provider(&(*button)->css_font);
    _oscontrol_destroy(*dcast(button, OSControl));
    heap_delete(button, OSButton);
}

/*---------------------------------------------------------------------------*/

void osbutton_OnClick(OSButton *button, Listener *listener)
{
    cassert_no_null(button);
    listener_update(&button->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

static void i_update_mnemonic_underline(OSButton *button, const char_t *text, const uint32_t pos)
{
    cassert_no_null(button);

    /* Push/flat buttons already render underlines through the custom Pango markup path */
    if (button_get_type(button->flags) == ekBUTTON_CHECK2 || button_get_type(button->flags) == ekBUTTON_CHECK3 || button_get_type(button->flags) == ekBUTTON_RADIO)
    {
        GtkWidget *label = gtk_bin_get_child(GTK_BIN(button->control.widget));

        if (label != NULL && GTK_IS_LABEL(label))
        {
            const gchar *ltext = cast_const(text, gchar);
            glong nchars = g_utf8_strlen(ltext, -1);

            if (pos != UINT32_MAX && pos < (uint32_t)nchars)
            {
                const gchar *start = g_utf8_offset_to_pointer(ltext, (glong)pos);
                const gchar *end = g_utf8_next_char(start);
                PangoAttrList *attrs = pango_attr_list_new();
                PangoAttribute *attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
                attr->start_index = (guint)(start - ltext);
                attr->end_index = (guint)(end - ltext);
                pango_attr_list_insert(attrs, attr);
                gtk_label_set_attributes(GTK_LABEL(label), attrs);
                pango_attr_list_unref(attrs);
            }
            else
            {
                gtk_label_set_attributes(GTK_LABEL(label), NULL);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_text(OSButton *button, const char_t *text)
{
    /*
     * We need to manage three kind of text strings:
     * 1) 'shortcut': translate incomming 'text' NAppGUI '&' to GTK '_' mnemonics.
     * 2) 'markup': pango markup '<span>' to underline the shortcut.
     * 3) 'plain': Without any markup or mnemonic for text measure.
     */
    char_t shortcut[256], markup[256], plain[256];
    uint32_t pos = UINT32_MAX;
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    pos = _osgui_underline_gtk_text(text, shortcut, sizeof(shortcut));
    _osgui_underline_markup(shortcut, pos, markup, sizeof(markup));
    _osgui_underline_plain(shortcut, pos, plain, sizeof(plain));
    str_upd(&button->text, plain);
    str_upd(&button->markup, markup);

    /*
     * The button inner label is a fake with '0' font size (invisible).
     * The use it just for GTK automatic manage of shortcuts.
     * The button 'real' text will be rendered in 'i_OnLabelDraw'
     */
    gtk_button_set_label(GTK_BUTTON(button->control.widget), shortcut);
    i_update_mnemonic_underline(button, plain, pos);
    gtk_widget_queue_draw(button->control.widget);
    button->twidth = -1.f;
    button->theight = -1.f;
}

/*---------------------------------------------------------------------------*/

void osbutton_tooltip(OSButton *button, const char_t *text)
{
    cassert_no_null(button);
    gtk_widget_set_tooltip_text(button->control.widget, cast_const(text, gchar));
}

/*---------------------------------------------------------------------------*/

void osbutton_font(OSButton *button, const Font *font)
{
    cassert_no_null(button);
    if (font_equals(font, button->font) == FALSE)
    {
        font_destroy(&button->font);
        button->font = font_copy(font);
        button->twidth = -1.f;
        button->theight = -1.f;
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_align(OSButton *button, const align_t align)
{
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    _oscontrol_set_halign(cast(button, OSControl), align);
}

/*---------------------------------------------------------------------------*/

void osbutton_image(OSButton *button, const Image *image)
{
    cassert_no_null(button);
    cassert(_osbutton_image_allowed(button->flags) == TRUE);
    if (button->image != NULL)
        image_destroy(&button->image);
    button->image = ptr_copyopt(image_copy, image, Image);
    gtk_widget_queue_draw(button->control.widget);
}

/*---------------------------------------------------------------------------*/

void osbutton_image_pos(OSButton *button, const gui_pos_t pos)
{
    cassert_no_null(button);
    cassert(button_get_type(button->flags) == ekBUTTON_FLAT || button_get_type(button->flags) == ekBUTTON_FLATGLE);
    button->imgpos = pos;
    gtk_widget_queue_draw(button->control.widget);
}

/*---------------------------------------------------------------------------*/

void osbutton_state(OSButton *button, const gui_state_t state)
{
    cassert_no_null(button);
    button->launch_event = FALSE;
    switch (button_get_type(button->flags))
    {
    case ekBUTTON_RADIO:
        if (state == ekGUI_ON)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->control.widget), TRUE);
        else
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->radio), TRUE);
        break;

    case ekBUTTON_CHECK2:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->control.widget), state == ekGUI_ON ? TRUE : FALSE);
        break;

    case ekBUTTON_CHECK3:
        switch (state)
        {
        case ekGUI_ON:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget), FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->control.widget), TRUE);
            break;
        case ekGUI_OFF:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget), FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->control.widget), FALSE);
            break;
        case ekGUI_MIXED:
            gtk_toggle_button_set_inconsistent(GTK_TOGGLE_BUTTON(button->control.widget), TRUE);
            break;
        default:
            cassert_default(state);
        }

        break;

    case ekBUTTON_FLATGLE:
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button->control.widget), (state == ekGUI_ON) ? TRUE : FALSE);
        break;

    default:
        cassert_default(button_get_type(button->flags));
    }
    button->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

gui_state_t osbutton_get_state(const OSButton *button)
{
    return i_get_state(button);
}

/*---------------------------------------------------------------------------*/

void osbutton_hpadding(OSButton *button, const real32_t padding)
{
    cassert_no_null(button);
    button->hpadding = padding >= 0 ? (uint32_t)padding : kBUTTON_HPADDING;
}

/*---------------------------------------------------------------------------*/

void osbutton_vpadding(OSButton *button, const real32_t padding)
{
    cassert_no_null(button);
    button->vpadding = padding >= 0 ? (uint32_t)padding : kBUTTON_VPADDING;
}

/*---------------------------------------------------------------------------*/

#if defined __ASSERTS__

static ___INLINE bool_t i_equal_button_text(const OSButton *button, const char_t *text)
{
    char_t shortcut[256], plain[256];
    uint32_t pos = UINT32_MAX;
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    pos = _osgui_underline_gtk_text(text, shortcut, sizeof(shortcut));
    _osgui_underline_plain(shortcut, pos, plain, sizeof(plain));
    return str_equ_c(plain, tc(button->text));
}

#endif

/*---------------------------------------------------------------------------*/

void osbutton_bounds(const OSButton *button, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height)
{
    cassert_no_null(button);
    cassert_no_null(width);
    cassert_no_null(height);

    i_update_text_extents(cast(button, OSButton));

    switch (button_get_type(button->flags))
    {
    case ekBUTTON_PUSH:
    {
        cassert_unref(i_equal_button_text(button, text) == TRUE, text);
        cassert(button->vpadding != UINT32_MAX);
        cassert(button->hpadding != UINT32_MAX);

        *width = button->twidth + i_PUSHBUTTON_EXTRAWIDTH;

        /* Image width  */
        if (refwidth > 0.f)
        {
            *width += refwidth;
            *width += (real32_t)kBUTTON_IMAGE_SEP;
        }

        *width += (real32_t)button->hpadding;

        /* Image height */
        if (refheight > button->theight)
            *height = refheight;
        else
            *height = button->theight;

        *height += (real32_t)button->vpadding;
        break;
    }

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
    {
        cassert_unref(i_equal_button_text(button, text) == TRUE, text);
        cassert(button->vpadding != UINT32_MAX);
        cassert(button->hpadding != UINT32_MAX);
        *width = button->twidth + i_PUSHBUTTON_EXTRAWIDTH;
        *width += (real32_t)_osglobals_check_width();
        *width += kCHECKBOX_IMAGE_SEP;
        *height = (real32_t)_osglobals_check_height();
        if (button->theight > *height)
            *height = button->theight;
        *height += i_CHECKBOX_EXTRAHEIGHT;
        break;
    }

    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        _osbutton_flat_bounds(text, button->font, refwidth, refheight, i_BUTTON_IMAGE_SEP, button->imgpos, button->hpadding, button->vpadding, width, height);
        break;

    default:
        cassert_default(button_get_type(button->flags));
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_attach(OSButton *button, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(button, OSControl));
}

/*---------------------------------------------------------------------------*/

void osbutton_detach(OSButton *button, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(button, OSControl));
}

/*---------------------------------------------------------------------------*/

void osbutton_visible(OSButton *button, const bool_t visible)
{
    _oscontrol_set_visible(cast(button, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osbutton_enabled(OSButton *button, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(button, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osbutton_size(const OSButton *button, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(button, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osbutton_origin(const OSButton *button, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(button, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osbutton_frame(OSButton *button, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(button, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osbutton_focus_widget(OSButton *button)
{
    cassert_no_null(button);
    switch (button_get_type(button->flags))
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_RADIO:
    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        return button->control.widget;

    default:
        cassert_default(button_get_type(button->flags));
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void _osbutton_command(OSButton *button)
{
    cassert_no_null(button->control.widget);
    i_OnClick(button->control.widget, button);
}

/*---------------------------------------------------------------------------*/

void _osbutton_set_default(OSButton *button, const bool_t is_default)
{
    cassert_no_null(button);
    if (button_get_type(button->flags) == ekBUTTON_PUSH)
    {
        if (button->is_default != is_default)
        {
            button->is_default = is_default;
            if (is_default == FALSE)
                gtk_widget_unset_state_flags(button->control.widget, GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_FOCUSED);

            gtk_widget_queue_draw(button->control.widget);
        }
    }
}
