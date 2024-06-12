/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: webview.c
 *
 */

/* Web view */

#include "webview.h"
#include "webview.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/guictx.h>
#include <geom2d/s2d.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _webview_t
{
    GuiComponent component;
    S2Df size;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

static void i_OnFocus(WebView *view, Event *e)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
        listener_pass_event(view->OnFocus, e, view, WebView);
}

/*---------------------------------------------------------------------------*/

WebView *webview_create(void)
{
    const GuiCtx *context = guictx_get_current();
    WebView *view = obj_new0(WebView);
    void *ositem = NULL;
    cassert_no_null(context);
    ositem = context->func_create[ekGUI_TYPE_WEBVIEW](ekTEXT_FLAG);
    view->size = s2df(256, 144);
    _component_init(&view->component, context, PARAM(type, ekGUI_TYPE_WEBVIEW), &ositem);
    context->func_web_OnFocus(view->component.ositem, obj_listener(view, i_OnFocus, WebView));
    return view;
}

/*---------------------------------------------------------------------------*/

void _webview_destroy(WebView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    _component_destroy_imp(&(*view)->component);
    listener_destroy(&(*view)->OnFocus);
    obj_delete(view, WebView);
}

/*---------------------------------------------------------------------------*/

void webview_OnFocus(WebView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void webview_size(WebView *view, const S2Df size)
{
    cassert_no_null(view);
    view->size = size;
}

/*---------------------------------------------------------------------------*/

void webview_navigate(WebView *view, const char_t *url)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_web_command);
    view->component.context->func_web_command(view->component.ositem, (enum_t)ekGUI_WEB_NAVIGATE, (const void *)url, NULL);
}

/*---------------------------------------------------------------------------*/

void webview_back(WebView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_web_command);
    view->component.context->func_web_command(view->component.ositem, (enum_t)ekGUI_WEB_BACK, NULL, NULL);
}

/*---------------------------------------------------------------------------*/

void webview_forward(WebView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_web_command);
    view->component.context->func_web_command(view->component.ositem, (enum_t)ekGUI_WEB_FORWARD, NULL, NULL);
}

/*---------------------------------------------------------------------------*/

void _webview_dimension(WebView *view, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(view);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        *dim0 = view->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = view->size.height;
    }
}
