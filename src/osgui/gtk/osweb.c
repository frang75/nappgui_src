/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: osweb.c
 *
 */

/* Operating System native webview */

#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "ossplit_gtk.inl"
#include "osweb_gtk.inl"
#include "oswindow_gtk.inl"
#include "../osweb.h"
#include "../osweb.inl"
#include <core/event.h>
#include <core/heap.h>
#include <osbs/dlib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef void WebKitWebView;
typedef struct _webkitapi_t WebKitApi;

typedef GtkWidget *(*FPtr_webkit_web_view_new)(void);
typedef void (*FPtr_webkit_web_view_load_uri)(WebKitWebView *web_view, const gchar *uri);
typedef void (*FPtr_webkit_web_view_go_back)(WebKitWebView *web_view);
typedef void (*FPtr_webkit_web_view_go_forward)(WebKitWebView *web_view);

struct _webkitapi_t
{
    DLib *dlib;
    FPtr_webkit_web_view_new web_view_new;
    FPtr_webkit_web_view_load_uri web_view_load_uri;
    FPtr_webkit_web_view_go_back web_view_go_back;
    FPtr_webkit_web_view_go_forward web_view_go_forward;
};

struct _osweb_t
{
    OSControl control;
    OSControl *capture;
    Listener *OnFocus;
    WebKitWebView *webView;
};

/*---------------------------------------------------------------------------*/

static bool_t i_WEBKIT_INIT = FALSE;
static WebKitApi i_WEBKIT;
static const char_t *i_NO_WEBKIT_TEXT = "Web content not available. WebKitGTK is not installed.";
static const gint i_NO_WEBKIT_MARGIN = 10;

/*---------------------------------------------------------------------------*/

static DLib *i_webkit_dlib(void)
{
    DLib *dlib = dlib_open(NULL, "webkit2gtk-4.1", "0");
    if (dlib == NULL)
        dlib = dlib_open(NULL, "webkit2gtk-4.0", "37");
    if (dlib == NULL)
        dlib = dlib_open(NULL, "webkit2gtk-3.0", "25");
    return dlib;
}

/*---------------------------------------------------------------------------*/

static bool_t i_webkit_procs(WebKitApi *api)
{
    cassert_no_null(api);
    api->web_view_new = dlib_proc(api->dlib, "webkit_web_view_new", FPtr_webkit_web_view_new);
    api->web_view_load_uri = dlib_proc(api->dlib, "webkit_web_view_load_uri", FPtr_webkit_web_view_load_uri);
    api->web_view_go_back = dlib_proc(api->dlib, "webkit_web_view_go_back", FPtr_webkit_web_view_go_back);
    api->web_view_go_forward = dlib_proc(api->dlib, "webkit_web_view_go_forward", FPtr_webkit_web_view_go_forward);
    if (api->web_view_new == NULL)
        return FALSE;
    if (api->web_view_load_uri == NULL)
        return FALSE;
    if (api->web_view_go_back == NULL)
        return FALSE;
    if (api->web_view_go_forward == NULL)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_webkit_init(void)
{
    if (i_WEBKIT_INIT == FALSE)
    {
        bmem_zero(&i_WEBKIT, WebKitApi);
        i_WEBKIT.dlib = i_webkit_dlib();
        if (i_WEBKIT.dlib != NULL)
        {
            if (i_webkit_procs(&i_WEBKIT) == FALSE)
            {
                dlib_close(&i_WEBKIT.dlib);
                bmem_zero(&i_WEBKIT, WebKitApi);
            }
        }

        i_WEBKIT_INIT = TRUE;
    }
}

/*---------------------------------------------------------------------------*/

void _osweb_finish(void)
{
    if (i_WEBKIT.dlib != NULL)
        dlib_close(&i_WEBKIT.dlib);
    bmem_zero(&i_WEBKIT, WebKitApi);
    i_WEBKIT_INIT = FALSE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnPressed(GtkWidget *widget, GdkEventButton *event, OSWeb *view)
{
    unref(widget);
    if (_oswindow_mouse_down(cast(view, OSControl)) == TRUE)
    {
        if (view->capture != NULL)
        {
            if (view->capture->type == ekGUI_TYPE_SPLITVIEW)
            {
                _ossplit_OnPress(cast(view->capture, OSSplit), event);
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

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static gboolean i_OnDraw(GtkWidget *widget, cairo_t *cr, OSWeb *view)
{
    gint width = gtk_widget_get_allocated_width(widget);
    gint height = gtk_widget_get_allocated_height(widget);
    gint text_width = width - 2 * i_NO_WEBKIT_MARGIN;
    unref(view);
    if (text_width > 0)
    {
        GtkStyleContext *context = gtk_widget_get_style_context(widget);
        GtkStateFlags state = gtk_style_context_get_state(context);
        PangoLayout *layout = gtk_widget_create_pango_layout(widget, i_NO_WEBKIT_TEXT);
        GdkRGBA color;
        gint lwidth = 0;
        gint lheight = 0;
        gtk_style_context_get_color(context, state, &color);
        pango_layout_set_width(layout, text_width * PANGO_SCALE);
        pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
        pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
        pango_layout_get_pixel_size(layout, &lwidth, &lheight);
        gdk_cairo_set_source_rgba(cr, &color);
        cairo_move_to(cr, (double)i_NO_WEBKIT_MARGIN, (double)((height - lheight) / 2));
        pango_cairo_show_layout(cr, layout);
        g_object_unref(layout);
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

OSWeb *osweb_create(const uint32_t flags)
{
    OSWeb *view = heap_new0(OSWeb);
    GtkWidget *widget = NULL;
    unref(flags);
    i_webkit_init();

    if (i_WEBKIT.dlib != NULL)
    {
        widget = i_WEBKIT.web_view_new();
        view->webView = cast(widget, WebKitWebView);
    }
    else
    {
        widget = gtk_drawing_area_new();
        view->webView = NULL;
        g_signal_connect(G_OBJECT(widget), "draw", G_CALLBACK(i_OnDraw), view);
    }

    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(i_OnPressed), view);
    _oscontrol_init(&view->control, ekGUI_TYPE_WEBVIEW, widget, widget, TRUE);
    return view;
}

/*---------------------------------------------------------------------------*/

void osweb_destroy(OSWeb **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    listener_destroy(&(*view)->OnFocus);
    _oscontrol_destroy(*dcast(view, OSControl));
    heap_delete(view, OSWeb);
}

/*---------------------------------------------------------------------------*/

void osweb_OnFocus(OSWeb *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osweb_command(OSWeb *view, const gui_web_t cmd, const void *param, void *result)
{
    cassert_no_null(view);
    unref(result);
    if (view->webView == NULL)
        return;

    switch (cmd)
    {
    case ekGUI_WEB_NAVIGATE:
        i_WEBKIT.web_view_load_uri(view->webView, cast_const(param, gchar));
        break;

    case ekGUI_WEB_BACK:
        i_WEBKIT.web_view_go_back(view->webView);
        break;

    case ekGUI_WEB_FORWARD:
        i_WEBKIT.web_view_go_forward(view->webView);
        break;

    default:
        cassert_default(cmd);
    }
}

/*---------------------------------------------------------------------------*/

void osweb_scroller_visible(OSWeb *view, const bool_t horizontal, const bool_t vertical)
{
    unref(view);
    unref(horizontal);
    unref(vertical);
}

/*---------------------------------------------------------------------------*/

void osweb_set_need_display(OSWeb *view)
{
    unref(view);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void osweb_clipboard(OSWeb *view, const clipboard_t clipboard)
{
    unref(view);
    unref(clipboard);
}

/*---------------------------------------------------------------------------*/

void osweb_attach(OSWeb *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osweb_detach(OSWeb *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void osweb_visible(OSWeb *view, const bool_t visible)
{
    cassert_no_null(view);
    _oscontrol_set_visible(cast(view, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osweb_enabled(OSWeb *view, const bool_t enabled)
{
    cassert_no_null(view);
    _oscontrol_set_enabled(cast(view, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osweb_size(const OSWeb *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osweb_origin(const OSWeb *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(view, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osweb_frame(OSWeb *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    _oscontrol_set_frame(cast(view, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

GtkWidget *_osweb_focus_widget(OSWeb *view)
{
    cassert_no_null(view);
    return view->control.widget;
}

/*---------------------------------------------------------------------------*/

void _osweb_set_capture(OSWeb *view, OSControl *control)
{
    cassert_no_null(view);
    view->capture = control;
}

/*---------------------------------------------------------------------------*/

void _osweb_release_capture(OSWeb *view)
{
    cassert_no_null(view);
    view->capture = NULL;
}
