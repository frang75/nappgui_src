/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: ospanel.c
 *
 */

/* Operating System native panel */

#include "osgui_gtk.inl"
#include "ospanel_gtk.inl"
#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ossplit_gtk.inl"
#include "../ospanel.h"
#include "../ospanel.inl"
#include "../osgui.inl"
#include "../oscontrol.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/arrst.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>

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
    real32_t twidth;
    String *text;
};

struct _ospanel_t
{
    OSControl control;
    OSControl *capture;
    GtkWidget *layout;
    GtkWidget *scroll;
    GtkAdjustment *hadjust;
    GtkAdjustment *vadjust;
    real32_t width;
    real32_t height;
    ArrSt(Area) *areas;
};

DeclSt(Area);
#define i_red(rgba) (((double)((uint8_t)((rgba) >> 0))) / 255.)
#define i_green(rgba) (((double)((uint8_t)((rgba) >> 8))) / 255.)
#define i_blue(rgba) (((double)((uint8_t)((rgba) >> 16))) / 255.)
#define i_alpha(rgba) (((double)((uint8_t)((rgba) >> 24))) / 255.)
static real32_t i_GROUP_TITLE_OFFSET = 8;
static real32_t i_GROUP_TITLE_CLEAN = 3;

/*---------------------------------------------------------------------------*/

static void i_remove_area(Area *area)
{
    cassert_no_null(area);
    str_destopt(&area->text);
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSPanel *panel)
{
    cassert_no_null(panel);
    unref(widget);

    if (panel->areas == NULL)
        return FALSE;

    cairo_save(cr);

    if (panel->hadjust != NULL)
    {
        gdouble x = gtk_adjustment_get_value(panel->hadjust);
        gdouble y = gtk_adjustment_get_value(panel->vadjust);
        cairo_translate(cr, -(double)x, -(double)y);
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

        if (area->text != NULL)
        {
            GtkStyleContext *context = gtk_widget_get_style_context(widget);
            gtk_style_context_save(context);
            gtk_style_context_add_class(context, GTK_STYLE_CLASS_FRAME);
            gtk_style_context_set_state(context, GTK_STATE_FLAG_NORMAL);
            gtk_render_frame(context, cr, (gdouble)area->x, (gdouble)area->y, (gdouble)area->w, (gdouble)area->h);
            gtk_style_context_restore(context);

            if (str_empty(area->text) == FALSE)
            {
                PangoLayout *layout = gtk_widget_create_pango_layout(widget, tc(area->text));
                Font *font = _osgui_create_default_font();
                const PangoFontDescription *fdesc = cast(font_native(font), PangoFontDescription);
                real32_t mwidth = area->w - 2 * i_GROUP_TITLE_OFFSET;

                pango_layout_set_font_description(layout, fdesc);
                pango_layout_set_width(layout, (int)mwidth * PANGO_SCALE);
                pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);

                if (area->twidth < 0)
                    font_extents(font, tc(area->text), -1, &area->twidth, NULL);

                /* Erase the border line */
                {
                    real32_t ewidth = area->twidth < mwidth ? area->twidth : mwidth;
                    gdouble x = (gdouble)(area->x + i_GROUP_TITLE_OFFSET - i_GROUP_TITLE_CLEAN);
                    gdouble y = (gdouble)area->y;
                    gdouble w = (gdouble)(ewidth + i_GROUP_TITLE_CLEAN * 2);

                    if (area->skcolor != kCOLOR_TRANSPARENT)
                    {
                        cairo_rectangle(cr, x, y, w, 1);
                        cairo_fill(cr);
                    }
                    else
                    {
                        GtkStyleContext *bcontext = _osglobals_button_context();
                        gtk_render_background(bcontext, cr, x, y - 2, w, 4);
                    }
                }

                /* Draw the text */
                {
                    real32_t height = font_height(font);
                    real32_t tx = area->x + i_GROUP_TITLE_OFFSET;
                    real32_t ty = area->y;
                    ty -= height / 2;
                    gtk_render_layout(context, cr, (gdouble)tx, (gdouble)ty, layout);
                }

                font_destroy(&font);
                g_object_unref(layout);
            }
        }
    arrst_end()

    cairo_restore(cr);
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSPanel *panel)
{
    unref(widget);

    if (panel->capture != NULL)
    {
        if (panel->capture->type == ekGUI_TYPE_SPLITVIEW)
            _ossplit_OnPress((OSSplit *)panel->capture, event);

        return TRUE;
    }
    /* The handler will be called before the default handler of the signal.
    This is the default behaviour */
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
    GtkWidget *top = widget;
    GtkWidget *focus = widget;

    panel->layout = widget;
    g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(i_OnDraw), panel);
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), panel);

    if (flags & ekPANEL_HSCROLL || flags & ekPANEL_VSCROLL)
    {
        GtkPolicyType hpolicy = GTK_POLICY_NEVER;
        GtkPolicyType vpolicy = GTK_POLICY_NEVER;

        if (flags & ekPANEL_HSCROLL)
            hpolicy = GTK_POLICY_AUTOMATIC;

        if (flags & ekPANEL_VSCROLL)
            vpolicy = GTK_POLICY_AUTOMATIC;

        panel->scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_widget_show(panel->layout);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(panel->scroll), hpolicy, vpolicy);
        gtk_container_add(GTK_CONTAINER(panel->scroll), panel->layout);

        /* A parent widget can "capture" the mouse */
        {
            GtkWidget *vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(panel->scroll));
            GtkWidget *hscroll = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(panel->scroll));
            g_signal_connect(G_OBJECT(vscroll), "button-press-event", G_CALLBACK(i_OnPressed), panel);
            g_signal_connect(G_OBJECT(hscroll), "button-press-event", G_CALLBACK(i_OnPressed), panel);
            g_signal_connect(G_OBJECT(panel->scroll), "button-press-event", G_CALLBACK(i_OnPressed), panel);
        }

        panel->hadjust = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(panel->scroll));
        panel->vadjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(panel->scroll));
        top = panel->scroll;
        focus = panel->scroll;
    }

    /* Creating the frame (border) view */
    if (flags & ekPANEL_BORDER)
    {
        GtkWidget *frame = gtk_frame_new(NULL);
        cassert(gtk_widget_get_has_window(frame) == FALSE);
        gtk_container_add(GTK_CONTAINER(frame), top);
        gtk_widget_show(top);
        top = frame;
    }

    _oscontrol_init(&panel->control, ekGUI_TYPE_PANEL, top, focus, TRUE);
    g_object_set_data(G_OBJECT(panel->layout), "OSControl", &panel->control);
    return panel;
}

/*---------------------------------------------------------------------------*/

void ospanel_destroy(OSPanel **panel)
{
    cassert_no_null(panel);
    cassert_no_null(*panel);

    if ((*panel)->areas != NULL)
        arrst_destroy(&(*panel)->areas, i_remove_area, Area);

    _oscontrol_destroy(*dcast(panel, OSControl));
    heap_delete(panel, OSPanel);
}

/*---------------------------------------------------------------------------*/

void ospanel_area(OSPanel *panel, void *obj, const char_t *group, const color_t bgcolor, const color_t skcolor, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
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
        arrst_end()

        if (area == NULL)
        {
            area = arrst_new0(panel->areas, Area);
            area->obj = obj;
        }

        area->x = x;
        area->y = y;
        area->w = width;
        area->h = height;
        area->bgcolor = bgcolor;
        area->skcolor = skcolor;
        str_upd(&area->text, group);
        area->twidth = -1;
    }
    else
    {
        if (panel->areas != NULL)
            arrst_clear(panel->areas, i_remove_area, Area);
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_scroll_get(const OSPanel *panel, real32_t *x, real32_t *y)
{
    cassert_no_null(panel);
    if (x != NULL)
    {
        if (panel->hadjust != NULL)
            *x = (real32_t)gtk_adjustment_get_value(panel->hadjust);
        else
            *x = 0;
    }

    if (y != NULL)
    {
        if (panel->vadjust != NULL)
            *y = (real32_t)gtk_adjustment_get_value(panel->vadjust);
        else
            *y = 0;
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_scroller_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    bool_t overlay = FALSE;

    cassert_no_null(panel);

#if GTK_CHECK_VERSION(3, 16, 0)
    {
        gboolean overlay_setting = TRUE;
        g_object_get(gtk_settings_get_default(), "gtk-overlay-scrolling", &overlay_setting, NULL);
        overlay = (bool_t)overlay_setting;
    }
#endif

    if (width != NULL)
    {
        if (overlay == TRUE)
        {
            *width = 0;
        }
        else
        {
            GtkWidget *vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(panel->scroll));
            gint minimum = 0, natural = 0;
            gtk_widget_get_preferred_width(vscroll, &minimum, &natural);
            *width = (real32_t)natural;
        }
    }

    if (height != NULL)
    {
        if (overlay == TRUE)
        {
            *height = 0;
        }
        else
        {
            GtkWidget *hscroll = gtk_scrolled_window_get_hscrollbar(GTK_SCROLLED_WINDOW(panel->scroll));
            gint minimum = 0, natural = 0;
            gtk_widget_get_preferred_height(hscroll, &minimum, &natural);
            *height = (real32_t)natural;
        }
    }
}

/*---------------------------------------------------------------------------*/

void ospanel_content_size(OSPanel *panel, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    real32_t cwidth = width;
    real32_t cheight = height;
    cassert_no_null(panel);
    cassert(panel->layout != NULL);
    cassert(panel->hadjust != NULL && panel->vadjust != NULL);
    unref(line_width);
    unref(line_height);

    {
        GtkPolicyType hpolicy = GTK_POLICY_NEVER, vpolicy = GTK_POLICY_NEVER;
        gtk_scrolled_window_get_policy(GTK_SCROLLED_WINDOW(panel->scroll), &hpolicy, &vpolicy);

        if (hpolicy == GTK_POLICY_AUTOMATIC && vpolicy == GTK_POLICY_AUTOMATIC)
        {
            bool_t with_hscroll = (bool_t)(width > panel->width);
            bool_t with_vscroll = (bool_t)(height > panel->height);
            real32_t barw = 0, barh = 0;
            ospanel_scroller_size(panel, &barw, &barh);

            if (with_hscroll == FALSE && with_vscroll == TRUE && barw > 0)
            {
                if (width > panel->width - barw)
                    with_hscroll = TRUE;
            }

            if (with_vscroll == FALSE && with_hscroll == TRUE && barh > 0)
            {
                if (height > panel->height - barh)
                    with_vscroll = TRUE;
            }

            if (with_hscroll == TRUE && with_vscroll == TRUE)
            {
                cwidth += barw;
                cheight += barh;
            }
        }
    }

    gtk_layout_set_size(GTK_LAYOUT(panel->layout), (guint)cwidth, (guint)cheight);
}

/*---------------------------------------------------------------------------*/

void ospanel_display(OSPanel *panel)
{
    cassert_no_null(panel);
    gtk_widget_queue_draw(panel->control.widget);
}

/*---------------------------------------------------------------------------*/

void ospanel_scroll_update(OSPanel *panel)
{
    GtkWidget *toplevel = NULL;
    GtkWidget *focus = NULL;

    cassert_no_null(panel);
    cassert_no_null(panel->scroll);

    toplevel = gtk_widget_get_toplevel(panel->scroll);
    if (GTK_IS_WINDOW(toplevel) == TRUE)
        focus = gtk_window_get_focus(GTK_WINDOW(toplevel));

    gtk_widget_hide(panel->scroll);
    gtk_widget_show(panel->scroll);

    while (gtk_events_pending() == TRUE)
        gtk_main_iteration();

    if (focus != NULL)
        gtk_widget_grab_focus(focus);
}

/*---------------------------------------------------------------------------*/

void ospanel_attach(OSPanel *panel, OSPanel *parent_panel)
{
    _ospanel_attach_control(parent_panel, cast(panel, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospanel_detach(OSPanel *panel, OSPanel *parent_panel)
{
    _ospanel_detach_control(parent_panel, cast(panel, OSControl));
}

/*---------------------------------------------------------------------------*/

void ospanel_visible(OSPanel *panel, const bool_t visible)
{
    _oscontrol_set_visible(cast(panel, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ospanel_enabled(OSPanel *panel, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(panel, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ospanel_size(const OSPanel *panel, real32_t *width, real32_t *height)
{
    cassert_no_null(panel);
    cassert_no_null(width);
    cassert_no_null(height);
    /* Ubuntu 16 and lower, the main panel gtk_widget_get_allocation() can be wrong */
    *width = panel->width;
    *height = panel->height;
}

/*---------------------------------------------------------------------------*/

void ospanel_origin(const OSPanel *panel, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(panel, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ospanel_frame(OSPanel *panel, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(panel);
    _oscontrol_set_frame(cast(panel, OSControl), x, y, width, height);
    panel->width = width;
    panel->height = height;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_child(GtkWidget *widget, gpointer data)
{
    OSPanel *panel = cast(data, OSPanel);
    OSControl *control = cast(g_object_get_data(G_OBJECT(widget), "OSControl"), OSControl);
    _oscontrol_detach_and_destroy(&control, panel);
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
    _oscontrol_attach_to_parent(control, panel->layout);
}

/*---------------------------------------------------------------------------*/

void _ospanel_detach_control(OSPanel *panel, OSControl *control)
{
    cassert_no_null(panel);
    _oscontrol_detach_from_parent(control, panel->layout);
}

/*---------------------------------------------------------------------------*/

void _ospanel_set_capture(OSPanel *panel, OSControl *control)
{
    cassert_no_null(panel);
    panel->capture = control;
}

/*---------------------------------------------------------------------------*/

void _ospanel_release_capture(OSPanel *panel)
{
    cassert_no_null(panel);
    panel->capture = NULL;
}

/*---------------------------------------------------------------------------*/

bool_t _ospanel_with_scroll(const OSPanel *panel)
{
    cassert_no_null(panel);
    return (bool_t)(panel->hadjust != NULL);
}

/*---------------------------------------------------------------------------*/

void _ospanel_scroll(OSPanel *panel, const int32_t x, const int32_t y)
{
    cassert_no_null(panel);
    if (panel->hadjust != NULL && x != INT32_MAX)
        gtk_adjustment_set_value(panel->hadjust, (gdouble)x);

    if (panel->vadjust != NULL && y != INT32_MAX)
        gtk_adjustment_set_value(panel->vadjust, (gdouble)y);
}

/*---------------------------------------------------------------------------*/

void _ospanel_scroll_frame(const OSPanel *panel, OSFrame *rect)
{
    real32_t w, h;
    cassert_no_null(panel);
    cassert_no_null(rect);
    if (panel->hadjust != NULL)
        rect->left = (int32_t)gtk_adjustment_get_value(panel->hadjust);
    else
        rect->left = 0;

    if (panel->vadjust != NULL)
        rect->top = (int32_t)gtk_adjustment_get_value(panel->vadjust);
    else
        rect->top = 0;

    _oscontrol_get_size(&panel->control, &w, &h);
    rect->right = rect->left + (int32_t)w;
    rect->bottom = rect->top + (int32_t)h;
}
