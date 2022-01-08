/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscontrol.c
 *
 */

/* Gtk Widgets common functions */

#include "oscontrol.inl"
#include "osgui.inl"
#include "osgui_gtk.inl"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "font.inl"
#include "ptr.h"
#include "strings.h"
#include "osedit.inl"
#include "oscombo.inl"
#include "osview.inl"
#include "oswindow.inl"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

/*---------------------------------------------------------------------------*/

static gboolean i_OnFocus(GtkWidget *widget, OSControl *control)
{
    _oscontrol_set_focus(control);
    unref(widget);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnFocusOut(GtkWidget *widget, GdkEvent *e, OSControl *control)
{
    _oscontrol_unset_focus(control);
    unref(widget);
    unref(e);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_init(OSControl *control, const guitype_t type, GtkWidget *widget, GtkWidget *focus_widget, const bool_t show)
{
    cassert_no_null(control);
    control->type = type;
    control->widget = widget;
    #if defined (__ASSERTS__)
    control->is_alive = TRUE;
    #endif
    g_signal_connect(focus_widget, "grab-focus", G_CALLBACK(i_OnFocus), (gpointer)control);
    //g_signal_connect(focus_widget, "focus-in-event", G_CALLBACK(i_OnFocusIn), (gpointer)control);
    g_signal_connect(focus_widget, "focus-out-event", G_CALLBACK(i_OnFocusOut), (gpointer)control);
    g_object_ref(control->widget);
    g_object_set_data(G_OBJECT(control->widget), "OSControl", control);
    gtk_widget_hide(control->widget);
    unref(show);
//    if (show == TRUE)
//        gtk_widget_show(control->widget);
//    else
//        gtk_widget_hide(control->widget);
}

/*---------------------------------------------------------------------------*/

#if defined (__ASSERTS__)

static void i_count(GtkWidget *widget, gpointer data)
{
    uint32_t *n = (uint32_t*)data;
    unref(widget);
    *n += 1;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_num_children(GtkContainer *container)
{
    uint32_t n = 0;
    gtk_container_foreach(container, i_count, (gpointer)&n);
    return n;
}

/*---------------------------------------------------------------------------*/

static void i_OnDestroy(GtkWidget *obj, OSControl *control)
{
    if (control->type < GUI_CONTEXT_NUM_COMPONENTS)
    {
        cassert(control->is_alive == TRUE);
        control->is_alive = FALSE;
    }
}
#endif

/*---------------------------------------------------------------------------*/

void _oscontrol_destroy(OSControl *control)
{
    cassert_no_null(control);
    #if defined (__ASSERTS__)
    cassert(control->is_alive == TRUE);
    g_signal_connect(control->widget, "destroy", G_CALLBACK(i_OnDestroy), (gpointer)control);
//    if (GTK_IS_LAYOUT(control->widget) || GTK_IS_WINDOW(control->widget) || GTK_IS_SCROLLED_WINDOW(control->widget))
//    {
//        cassert(i_num_children(GTK_CONTAINER(control->widget)) == 0);
//    }
    #endif

    g_object_unref(control->widget);
    #if defined (__ASSERTS__)
    //cassert(control->is_alive == FALSE);
    #endif
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_focus(OSControl *control)
{
    cassert_no_null(control);
    switch (control->type) {
    case ekGUI_COMPONENT_EDITBOX:
        _osedit_set_focus((OSEdit*)control);
        break;
    case ekGUI_COMPONENT_COMBOBOX:
        _oscombo_set_focus((OSCombo*)control);
        break;
    case ekGUI_COMPONENT_CUSTOMVIEW:
        _osview_set_focus((OSView*)control);
        break;
    default:
        break;
    }
}

/*---------------------------------------------------------------------------*/

void _oscontrol_unset_focus(OSControl *control)
{
    cassert_no_null(control);
    switch (control->type) {
    case ekGUI_COMPONENT_EDITBOX:
        _osedit_unset_focus((OSEdit*)control);
        break;
    case ekGUI_COMPONENT_COMBOBOX:
        _oscombo_unset_focus((OSCombo*)control);
        break;
    case ekGUI_COMPONENT_CUSTOMVIEW:
        _osview_unset_focus((OSView*)control);
        break;
    case ekGUI_COMPONENT_WINDOW:
        _oswindow_unset_focus((OSWindow*)control);
        break;
    default:
        break;
    }
}

/*---------------------------------------------------------------------------*/

char_t *_oscontrol_get_text(const OSControl *control, uint32_t *tsize)
{
	unref(control);
	unref(tsize);
	cassert(FALSE);
	return NULL;
}

//*---------------------------------------------------------------------------*/

void _oscontrol_set_text(OSControl *control, const char_t *text)
{
	unref(control);
	unref(text);
	cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_remove_provider(GtkWidget *widget, GtkCssProvider *css_prov)
{
    GtkStyleContext *c = gtk_widget_get_style_context(widget);
    cassert_no_null(css_prov);
    gtk_style_context_remove_provider(c, GTK_STYLE_PROVIDER(css_prov));
}

/*---------------------------------------------------------------------------*/

color_t _oscontrol_from_gdkrgba(const GdkRGBA *gdkcolor)
{
    return color_rgba(
                (uint8_t)(gdkcolor->red * 255.),
                (uint8_t)(gdkcolor->green * 255.),
                (uint8_t)(gdkcolor->blue * 255.),
                (uint8_t)(gdkcolor->alpha * 255.));
}

/*---------------------------------------------------------------------------*/

void _oscontrol_to_gdkrgba(const color_t color, GdkRGBA *gdkcolor)
{
    real32_t r, g, b, a;
    color_get_rgbaf(color, &r, &g, &b, &a);
    gdkcolor->red = (double)r;
    gdkcolor->green = (double)g;
    gdkcolor->blue = (double)b;
    gdkcolor->alpha = (double)a;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_widget_color(GtkWidget *widget, const char_t *css_type, const color_t color, GtkCssProvider **css_provider)
{
    char_t html[16];
    String *css;
    color_to_html(color, html, sizeof(html));
    css = str_printf("%s {color:%s;}", css_type, html);
    _oscontrol_set_css_prov(widget, tc(css), css_provider);
    str_destroy(&css);
}

/*---------------------------------------------------------------------------*/

static __INLINE color_t i_color(const GdkRGBA *color)
{
    cassert_no_null(color);
    return color_rgba((uint8_t)(color->red * 255.), (uint8_t)(color->green * 255.), (uint8_t)(color->blue * 255.), (uint8_t)(color->alpha * 255.));
}

/*---------------------------------------------------------------------------*/

void _oscontrol_widget_get_color(GtkWidget *widget, color_t *color)
{
    GdkRGBA gcolor;
    GtkStyleContext *c = gtk_widget_get_style_context(widget);
    cassert_no_null(color);
    gtk_style_context_get_color(c, GTK_STATE_FLAG_NORMAL, &gcolor);
    *color = i_color(&gcolor);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_widget_bg_color(GtkWidget *widget, const char_t *css_type, const color_t color, GtkCssProvider **css_provider)
{
    char_t html[16];
    String *css;
    color_to_html(color, html, sizeof(html));
    css = str_printf("%s {background-color:%s;background-image:none;}", css_type, html);
    _oscontrol_set_css_prov(widget, tc(css), css_provider);
    str_destroy(&css);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_widget_border(GtkWidget *widget, const char_t *css_type, const color_t color, GtkCssProvider **css_provider)
{
    char_t html[16];
    String *css;
    color_to_html(color, html, sizeof(html));
    css = str_printf("%s {border-width:1px; border-style:solid; border-color:%s;}", css_type, html);
    _oscontrol_set_css_prov(widget, tc(css), css_provider);
    str_destroy(&css);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_font_px(const real32_t fsize, const uint32_t fstyle)
{
    if ((fstyle & ekFPOINTS) == ekFPOINTS)
    {
        PangoFontMap *fontmap = pango_cairo_font_map_get_default();
        real32_t dpi = (real32_t)pango_cairo_font_map_get_resolution((PangoCairoFontMap*)fontmap);
        return (uint32_t)(fsize / (dpi / 72.f));
    }
    else
    {
        return (uint32_t)fsize;
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _oscontrol_widget_font_size(GtkWidget *widget)
{
    PangoFontDescription *fdesc;
    GtkStyleContext *ctx = gtk_widget_get_style_context (widget);
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    real32_t dpi = (real32_t)pango_cairo_font_map_get_resolution((PangoCairoFontMap*)fontmap);
    real32_t fsize = 0;
    gtk_style_context_get(ctx, GTK_STATE_FLAG_NORMAL, "font", &fdesc, NULL);
    fsize = (real32_t)pango_font_description_get_size(fdesc);
    pango_font_description_free(fdesc);
    fsize = fsize * (dpi / 72.f) / (real32_t)PANGO_SCALE;
    return (uint32_t)fsize;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_widget_font(GtkWidget *widget, const char_t *css_type, const Font *font, GtkCssProvider **css_prov)
{
    PangoFontDescription *fdesc = (PangoFontDescription*)font_native(font);
    const char *family = pango_font_description_get_family(fdesc);
    uint32_t fstyle = font_style(font);
    uint32_t pxsize = i_font_px(font_size(font), fstyle);
    const char *italic = (fstyle & ekFITALIC) ? "italic" : "";
    const char *bold = (fstyle & ekFBOLD) ? "bold" : "";
    const char *decor = NULL;
    String *css = NULL;

#if GTK_CHECK_VERSION(3, 6, 0)
    if (fstyle & ekFUNDERLINE)
    {
        if (fstyle & ekFSTRIKEOUT)
            decor = "underline | line-through";
        else
            decor = "underline";
    }
    else if (fstyle & ekFSTRIKEOUT)
    {
        decor = "line-through";
    }
    else
    {
        decor = "none";
    }
#endif

    if (decor != NULL)
        css = str_printf("%s {font:%s %s %dpx \"%s\"; text-decoration-line:%s;}", css_type, italic, bold, pxsize, family, decor);
    else
        css = str_printf("%s {font:%s %s %dpx \"%s\";}", css_type, italic, bold, pxsize, family);

    _oscontrol_set_css_prov(widget, tc(css), css_prov);
    str_destroy(&css);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_css_sel(const guitype_t type)
{
    switch (type) {
    case ekGUI_COMPONENT_LABEL:
        return "label";
    case ekGUI_COMPONENT_BUTTON:
        return "button";
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_COMBOBOX:
        return "combobox";
    case ekGUI_COMPONENT_EDITBOX:
        return "entry";
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
    case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_CUSTOMVIEW:
    cassert_default();
    }

    return "";
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_font(OSControl *control, const Font *font, GtkCssProvider **css_prov)
{
    const char_t *css_type;
    cassert_no_null(control);
    css_type = i_css_sel(control->type);
    _oscontrol_widget_font(control->widget, css_type, font, css_prov);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_update_font(OSControl *control, Font **current_font, const Font *font)
{
    cassert_no_null(control);
    cassert_no_null(current_font);
    if (font_equals(*current_font, font) == FALSE)
    {
        font_destroy(current_font);
        *current_font = font_copy(font);
        _oscontrol_set_font(control, font, NULL);
    }
}

/*---------------------------------------------------------------------------*/

static GtkAlign i_align(const align_t align)
{
    switch(align) {
    case ekLEFT:
        return GTK_ALIGN_START;
    case ekCENTER:
        return GTK_ALIGN_CENTER;
    case ekRIGHT:
        return GTK_ALIGN_END;
    case ekJUSTIFY:
        return GTK_ALIGN_FILL;
    cassert_default();
    }
    return GTK_ALIGN_START;
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_halign(OSControl *control, const align_t align)
{
    GtkAlign a = i_align(align);
    cassert_no_null(control);
    gtk_widget_set_halign(control->widget, a);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_css_prov(GtkWidget *widget, const char_t *css, GtkCssProvider **css_prov)
{
    GtkCssProvider *p = gtk_css_provider_new();
    GtkStyleContext *c = gtk_widget_get_style_context(widget);
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(p), (gchar*)css, -1, NULL);
    gtk_style_context_add_provider(c, GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(p);
    ptr_assign(css_prov, p);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_css(GtkWidget *widget, const char_t *css)
{
    _oscontrol_set_css_prov(widget, css, NULL);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_text_bounds(const OSControl *control, PangoLayout *layout, const char_t *text, const Font *font, const real32_t refwidth, real32_t *width, real32_t *height)
{
    //StringSizeData data;
    int w, h;
    // Show label bounds
    //cassert(FALSE);

    if (layout == NULL)
    {
        const PangoFontDescription *fdesc = (PangoFontDescription*)font_native(font);
        pango_layout_set_font_description(kPANGO_LAYOUT, fdesc);
        pango_layout_set_wrap(kPANGO_LAYOUT, PANGO_WRAP_WORD);
        pango_layout_set_ellipsize(kPANGO_LAYOUT, PANGO_ELLIPSIZE_NONE);
        pango_layout_set_alignment(kPANGO_LAYOUT, PANGO_ALIGN_CENTER);
        pango_layout_set_width(kPANGO_LAYOUT, refwidth > 0.f ? (int)(refwidth * (real32_t)PANGO_SCALE): -1);
        pango_layout_set_text(kPANGO_LAYOUT, (const char*)text, -1);
        pango_layout_get_pixel_size(kPANGO_LAYOUT, &w, &h);
    }
    else
    {
        pango_layout_set_width(layout, refwidth > 0.f ? (int)(refwidth * (real32_t)PANGO_SCALE): -1);
        pango_layout_set_text(layout, (const char*)text, -1);
        pango_layout_get_pixel_size(layout, &w, &h);
    }

    *width = (real32_t)w;
    *height = (real32_t)h;
    unref(control);
    //data.layout = kPANGO_LAYOUT;
    //_osgui_text_bounds(&data, text, refwidth, width, height);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_visible(OSControl *control, const bool_t is_visible)
{
    cassert_no_null(control);
    if (is_visible == TRUE)
        gtk_widget_show(control->widget);
    else
        gtk_widget_hide(control->widget);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_set_enabled(OSControl *control, const bool_t is_enabled)
{
    cassert_no_null(control);
    gtk_widget_set_sensitive(control->widget, (gboolean)is_enabled);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_get_origin(const OSControl *control, real32_t *x, real32_t *y)
{
    GtkAllocation alloc;
    cassert_no_null(control);
    cassert_no_null(x);
    cassert_no_null(y);

#if GTK_CHECK_VERSION(3, 20, 0)
    gtk_widget_get_allocated_size(control->widget, &alloc, NULL);
#else
    gtk_widget_get_allocation(control->widget, &alloc);
#endif

    *x = (real32_t)alloc.x;
    *y = (real32_t)alloc.y;
}

//*---------------------------------------------------------------------------*/

void _oscontrol_get_size(const OSControl *control, real32_t *width, real32_t *height)
{
    GtkAllocation alloc;
    cassert_no_null(control);
    cassert_no_null(width);
    cassert_no_null(height);

#if GTK_CHECK_VERSION(3, 20, 0)
    gtk_widget_get_allocated_size(control->widget, &alloc, NULL);
#else
    gtk_widget_get_allocation(control->widget, &alloc);
#endif

    *width = (real32_t)alloc.width;
    *height = (real32_t)alloc.height;
}

//*---------------------------------------------------------------------------*/

void _oscontrol_set_position(OSControl *control, const int x, const int y)
{
    unref(control);
    unref(x);
    unref(y);
	cassert(FALSE);
}

//*---------------------------------------------------------------------------*/

void _oscontrol_set_frame(OSControl *control, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    GtkWidget *parent;
    //const gchar *type;
    const gchar *ptype;
    cassert_no_null(control);
	parent = gtk_widget_get_parent(control->widget);
    cassert_no_null(parent);
    //type = G_OBJECT_TYPE_NAME(control->widget);
	ptype = G_OBJECT_TYPE_NAME(parent);
	// GtkLayout inside GtkLayout
	// https://stackoverflow.com/questions/47879677/is-it-possible-to-put-gtklayouts-inside-gtklayout
	if (str_equ_c(ptype, "GtkLayout"))
	{
	    gtk_layout_move(GTK_LAYOUT(parent), control->widget, (gint)x, (gint)y);
	    gtk_widget_set_size_request(control->widget, (gint)width, (gint)height);
	}
	else
	{
	    cassert(str_equ_c(ptype, "GtkBox"));
        cassert(str_equ_c(G_OBJECT_TYPE_NAME(control->widget), "GtkLayout") || str_equ_c(G_OBJECT_TYPE_NAME(control->widget), "GtkScrolledWindow"));
	    cassert(control->type == ekGUI_COMPONENT_PANEL);
	    cassert(x == 0.f);
	    cassert(y == 0.f);
	    //gtk_widget_set_size_request(control->widget, (gint)width, (gint)height);
        //gtk_widget_set_size_request(control->widget, 0, 0);
        //gtk_layout_set_size(GTK_LAYOUT(control->widget), (gint)width, (gint)height);
    }
}

/*---------------------------------------------------------------------------*/

void _oscontrol_attach_to_parent(OSControl *control, GtkWidget *parent_widget)
{
    #if defined(__ASSERTS__)
    uint32_t c = i_num_children(GTK_CONTAINER(parent_widget));
    #endif
    cassert_no_null(control);
    cassert(str_equ_c(G_OBJECT_TYPE_NAME(parent_widget), "GtkLayout"));
    gtk_layout_put(GTK_LAYOUT(parent_widget), control->widget, 0, 0);
    cassert(i_num_children(GTK_CONTAINER(parent_widget)) == c + 1);
}

/*---------------------------------------------------------------------------*/

void _oscontrol_detach_from_parent(OSControl *control, GtkWidget *parent_widget)
{
    #if defined(__ASSERTS__)
    uint32_t c = i_num_children(GTK_CONTAINER(parent_widget));
    cassert(c > 0);
    #endif
    cassert_no_null(control);
    gtk_container_remove(GTK_CONTAINER(parent_widget), control->widget);
    cassert(i_num_children(GTK_CONTAINER(parent_widget)) == c - 1);
}

