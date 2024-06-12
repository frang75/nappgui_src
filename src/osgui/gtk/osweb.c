/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osweb.c
 *
 */

/* Operating System native webview */

#include "osweb.h"
#include "osweb.inl"
#include "osglobals_gtk.inl"
#include "oscontrol_gtk.inl"
#include "ospanel_gtk.inl"
#include "ossplit_gtk.inl"
#include "osweb_gtk.inl"
#include "oswindow_gtk.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

typedef struct _webimpt_t WebImp;

#if defined(NAPPGUI_WEB_SUPPORT)
#include <sewer/nowarn.hxx>
#include <webkit2/webkit2.h>
#include <sewer/warn.hxx>

struct _webimpt_t
{
    WebKitWebView *webView;
};

#else

struct _webimpt_t
{
    uint32_t dummy;
};

#endif

struct _osweb_t
{
    OSControl control;
    OSControl *capture;
    Listener *OnFocus;
    WebImp web;
};

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

OSWeb *osweb_create(const uint32_t flags)
{
    OSWeb *view = heap_new0(OSWeb);
    GtkWidget *widget = NULL;
    unref(flags);

#if defined(NAPPGUI_WEB_SUPPORT)
    widget = webkit_web_view_new();
    view->web.webView = WEBKIT_WEB_VIEW(widget);
#else
    widget = gtk_drawing_area_new();
#endif

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
#if defined(NAPPGUI_WEB_SUPPORT)
    cassert_no_null(view);
    unref(result);
    switch (cmd)
    {
    case ekGUI_WEB_NAVIGATE:
        webkit_web_view_load_uri(view->web.webView, cast_const(param, gchar));
        break;

    case ekGUI_WEB_BACK:
        webkit_web_view_go_back(view->web.webView);
        break;

    case ekGUI_WEB_FORWARD:
        webkit_web_view_go_forward(view->web.webView);
        break;

        cassert_default();
    }
#else
    unref(view);
    unref(cmd);
    unref(param);
    unref(result);
#endif
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
