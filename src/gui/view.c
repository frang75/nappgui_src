/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: view.c
 *
 */

/* Custom view */

#include "view.h"
#include "view.inl"
#include "vctrl.inl"
#include "cell.inl"
#include "component.inl"
#include "gui.inl"
#include "panel.inl"
#include <draw2d/guictx.h>
#include <geom2d/s2d.h>
#include <geom2d/v2d.h>
#include <core/heap.h>
#include <core/event.h>
#include <core/keybuf.h>
#include <core/objh.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

typedef struct _vlisteners_t VListeners;

struct _vlisteners_t
{
    Listener *OnDraw;
    Listener *OnOverlay;
    Listener *OnResize;
    Listener *OnEnter;
    Listener *OnExit;
    Listener *OnMoved;
    Listener *OnDown;
    Listener *OnUp;
    Listener *OnClick;
    Listener *OnDrag;
    Listener *OnWheel;
    Listener *OnKeyDown;
    Listener *OnKeyUp;
    Listener *OnFocus;
    Listener *OnResignFocus;
    Listener *OnAcceptFocus;
    Listener *OnScroll;
    KeyBuf *keybuf;
    FPtr_destroy func_destroy_data;
};

struct _view_t
{
    GuiComponent component;
    S2Df size;
    void *data;
    const VCtrlTbl *vtbl;
    VListeners *listeners;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_listeners(VListeners **listeners)
{
    cassert_no_null(listeners);
    cassert_no_null(*listeners);
    listener_destroy(&(*listeners)->OnDraw);
    listener_destroy(&(*listeners)->OnOverlay);
    listener_destroy(&(*listeners)->OnResize);
    listener_destroy(&(*listeners)->OnEnter);
    listener_destroy(&(*listeners)->OnExit);
    listener_destroy(&(*listeners)->OnMoved);
    listener_destroy(&(*listeners)->OnDown);
    listener_destroy(&(*listeners)->OnUp);
    listener_destroy(&(*listeners)->OnClick);
    listener_destroy(&(*listeners)->OnDrag);
    listener_destroy(&(*listeners)->OnWheel);
    listener_destroy(&(*listeners)->OnKeyDown);
    listener_destroy(&(*listeners)->OnKeyUp);
    listener_destroy(&(*listeners)->OnFocus);
    listener_destroy(&(*listeners)->OnResignFocus);
    listener_destroy(&(*listeners)->OnAcceptFocus);
    listener_destroy(&(*listeners)->OnScroll);
    heap_delete(listeners, VListeners);
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_DRAW);
    listener_pass_event(view->listeners->OnDraw, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnOverlay(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_OVERLAY);
    listener_pass_event(view->listeners->OnOverlay, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnEnter(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_ENTER);
    listener_pass_event(view->listeners->OnEnter, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_EXIT);
    listener_pass_event(view->listeners->OnExit, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnMoved(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_MOVED);
    listener_pass_event(view->listeners->OnMoved, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnDown(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_DOWN);
    listener_pass_event(view->listeners->OnDown, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnUp(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_UP);
    listener_pass_event(view->listeners->OnUp, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_CLICK);
    listener_pass_event(view->listeners->OnClick, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnDrag(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_DRAG);
    listener_pass_event(view->listeners->OnDrag, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnWheel(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_WHEEL);
    listener_pass_event(view->listeners->OnWheel, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyDown(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_KEYDOWN);
    if (view->listeners->keybuf != NULL)
    {
        const EvKey *p = event_params(e, EvKey);
        keybuf_OnDown(view->listeners->keybuf, p->key);
    }

    if (view->listeners->OnKeyDown != NULL)
        listener_pass_event(view->listeners->OnKeyDown, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnKeyUp(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    cassert(event_type(e) == ekGUI_EVENT_KEYUP);
    if (view->listeners->keybuf != NULL)
    {
        const EvKey *p = event_params(e, EvKey);
        keybuf_OnUp(view->listeners->keybuf, p->key);
    }

    if (view->listeners->OnKeyUp != NULL)
        listener_pass_event(view->listeners->OnKeyUp, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    if (view->listeners->keybuf != NULL)
        keybuf_clear(view->listeners->keybuf);
    listener_pass_event(view->listeners->OnFocus, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnResignFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    listener_pass_event(view->listeners->OnResignFocus, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnAcceptFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    listener_pass_event(view->listeners->OnAcceptFocus, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnScroll(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    listener_pass_event(view->listeners->OnScroll, e, view, View);
}

/*---------------------------------------------------------------------------*/

static View *i_create(const uint32_t flags, const VCtrlTbl *vtbl)
{
    const GuiCtx *context = guictx_get_current();
    void *ositem = context->func_create[ekGUI_TYPE_CUSTOMVIEW](flags);

    if (ositem != NULL)
    {
        View *view = obj_new0(View);
        view->size = s2df(128, 128);

        if (vtbl != NULL)
            view->vtbl = vtbl;
        else
            view->listeners = heap_new0(VListeners);

        _component_init(&view->component, context, PARAM(type, ekGUI_TYPE_CUSTOMVIEW), &ositem);
        return view;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

View *view_create(void)
{
    return i_create(0, NULL);
}

/*---------------------------------------------------------------------------*/

View *view_scroll(void)
{
    return i_create(ekVIEW_HSCROLL | ekVIEW_VSCROLL, NULL);
}

/*---------------------------------------------------------------------------*/

View *view_custom(const bool_t scroll, const bool_t border)
{
    uint32_t flags = 0;
    if (scroll == TRUE)
        flags |= ekVIEW_HSCROLL | ekVIEW_VSCROLL;
    if (border == TRUE)
        flags |= ekVIEW_BORDER;
    return i_create(flags, NULL);
}

/*---------------------------------------------------------------------------*/

View *_view_create(const uint32_t flags)
{
    return i_create(flags, NULL);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(View *view)
{
    cassert_no_null(view);
    if (view->data != NULL)
    {
        if (view->vtbl != NULL)
        {
            cassert_no_nullf(view->vtbl->func_destroy_data);
            view->vtbl->func_destroy_data(&view->data);
        }
        else
        {
            cassert_no_null(view->listeners);
            cassert_no_nullf(view->listeners->func_destroy_data);
            view->listeners->func_destroy_data(&view->data);
        }
    }
}

/*---------------------------------------------------------------------------*/

void view_data_imp(View *view, void **data, FPtr_destroy func_destroy_data)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    i_destroy_data(view);
    view->data = ptr_dget_no_null(data, void);
    view->listeners->func_destroy_data = func_destroy_data;
}

/*---------------------------------------------------------------------------*/

void *view_get_data_imp(const View *view)
{
    cassert_no_null(view);
    return view->data;
}

/*---------------------------------------------------------------------------*/

void _view_destroy(View **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    i_destroy_data(*view);
    ptr_destopt(i_destroy_listeners, &(*view)->listeners, VListeners);
    _component_destroy_imp(&(*view)->component);
    _gui_delete_transition(*view, View);
    obj_delete(view, View);
}

/*---------------------------------------------------------------------------*/

void view_size(View *view, const S2Df size)
{
    cassert_no_null(view);
    view->size = size;
}

/*---------------------------------------------------------------------------*/

void view_OnDraw(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnDraw, listener, i_OnDraw, view->component.context->func_view_OnDraw, View);
}

/*---------------------------------------------------------------------------*/

void view_OnOverlay(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnOverlay, listener, i_OnOverlay, view->component.context->func_view_OnOverlay, View);
}

/*---------------------------------------------------------------------------*/

void view_OnSize(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    listener_update(&view->listeners->OnResize, listener);
}

/*---------------------------------------------------------------------------*/

void view_OnEnter(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnEnter, listener, i_OnEnter, view->component.context->func_view_OnEnter, View);
}

/*---------------------------------------------------------------------------*/

void view_OnExit(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnExit, listener, i_OnExit, view->component.context->func_view_OnExit, View);
}

/*---------------------------------------------------------------------------*/

void view_OnMove(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnMoved, listener, i_OnMoved, view->component.context->func_view_OnMoved, View);
}

/*---------------------------------------------------------------------------*/

void view_OnDown(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnDown, listener, i_OnDown, view->component.context->func_view_OnDown, View);
}

/*---------------------------------------------------------------------------*/

void view_OnUp(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnUp, listener, i_OnUp, view->component.context->func_view_OnUp, View);
}

/*---------------------------------------------------------------------------*/

void view_OnClick(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnClick, listener, i_OnClick, view->component.context->func_view_OnClick, View);
}

/*---------------------------------------------------------------------------*/

void view_OnDrag(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnDrag, listener, i_OnDrag, view->component.context->func_view_OnDrag, View);
}

/*---------------------------------------------------------------------------*/

void view_OnWheel(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnWheel, listener, i_OnWheel, view->component.context->func_view_OnWheel, View);
}

/*---------------------------------------------------------------------------*/

void view_OnKeyDown(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnKeyDown, listener, i_OnKeyDown, view->component.context->func_view_OnKeyDown, View);
    if (view->listeners->keybuf != NULL && view->listeners->OnKeyDown == NULL)
        view->component.context->func_view_OnKeyDown(view->component.ositem, listener(view, i_OnKeyDown, View));
}

/*---------------------------------------------------------------------------*/

void view_OnKeyUp(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnKeyUp, listener, i_OnKeyUp, view->component.context->func_view_OnKeyUp, View);
    if (view->listeners->keybuf != NULL && view->listeners->OnKeyUp == NULL)
        view->component.context->func_view_OnKeyUp(view->component.ositem, listener(view, i_OnKeyUp, View));
}

/*---------------------------------------------------------------------------*/

void view_OnFocus(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnFocus, listener, i_OnFocus, view->component.context->func_view_OnFocus, View);
}

/*---------------------------------------------------------------------------*/

void view_OnResignFocus(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnResignFocus, listener, i_OnResignFocus, view->component.context->func_view_OnResignFocus, View);
}

/*---------------------------------------------------------------------------*/

void view_OnAcceptFocus(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnAcceptFocus, listener, i_OnAcceptFocus, view->component.context->func_view_OnAcceptFocus, View);
}

/*---------------------------------------------------------------------------*/

void view_OnScroll(View *view, Listener *listener)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    component_update_listener(view, &view->listeners->OnScroll, listener, i_OnScroll, view->component.context->func_view_OnScroll, View);
}

/*---------------------------------------------------------------------------*/

void view_allow_tab(View *view, const bool_t allow)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    view->component.context->func_view_allow_key(view->component.ositem, ekKEY_TAB, (uint32_t)allow);
}

/*---------------------------------------------------------------------------*/

void view_keybuf(View *view, KeyBuf *buffer)
{
    cassert_no_null(view);
    cassert_no_null(view->listeners);
    view->listeners->keybuf = buffer;

    if (view->listeners->keybuf != NULL && view->listeners->OnKeyDown == NULL)
        view->component.context->func_view_OnKeyDown(view->component.ositem, listener(view, i_OnKeyDown, View));

    if (view->listeners->keybuf != NULL && view->listeners->OnKeyUp == NULL)
        view->component.context->func_view_OnKeyUp(view->component.ositem, listener(view, i_OnKeyUp, View));
}

/*---------------------------------------------------------------------------*/

const char_t *_view_subtype(const View *view)
{
    cassert_no_null(view);
    if (view->vtbl != NULL)
    {
        cassert(str_empty_c(view->vtbl->type) == FALSE);
        return view->vtbl->type;
    }
    else
    {
        return "View";
    }
}

/*---------------------------------------------------------------------------*/

void _view_locale(View *view)
{
    cassert_no_null(view);
    if (view->vtbl != NULL && view->vtbl->func_locale != NULL)
        view->vtbl->func_locale(cast(view, void));
}

/*---------------------------------------------------------------------------*/

void _view_natural(View *view, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(view);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (view->vtbl != NULL && view->vtbl->func_natural != NULL)
    {
        view->vtbl->func_natural(cast(view, void), di, dim0, dim1);
        if (di == 0)
        {
            view->size.width = *dim0;
        }
        else
        {
            cassert(di == 1);
            view->size.height = *dim1;
        }
    }

    if (di == 0)
    {
        *dim0 = view->size.width;
    }
    else
    {
        cassert(di == 1);
        *dim1 = view->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _view_OnResize(View *view, const S2Df *size)
{
    cassert_no_null(view);
    cassert_no_null(size);
    if (view->listeners != NULL && view->listeners->OnResize != NULL)
    {
        EvSize p;
        p.width = size->width;
        p.height = size->height;
        listener_event(view->listeners->OnResize, ekGUI_EVENT_RESIZE, view, &p, NULL, View, EvSize, void);
    }
    else if (view->vtbl != NULL && view->vtbl->OnResize != NULL)
    {
        EvSize p;
        p.width = size->width;
        p.height = size->height;
        listener_handler(view->vtbl->OnResize, view->data, ekGUI_EVENT_RESIZE, view, &p, NULL, View, EvSize, void);
    }

    view->component.context->func_view_set_need_display(view->component.ositem);
}

/*---------------------------------------------------------------------------*/

void _view_empty(View *view)
{
    cassert_no_null(view);
    if (view->vtbl != NULL && view->vtbl->func_empty != NULL)
        view->vtbl->func_empty(view);
}

/*---------------------------------------------------------------------------*/

void _view_uint32(View *view, const uint32_t value)
{
    cassert_no_null(view);
    if (view->vtbl != NULL && view->vtbl->func_uint32 != NULL)
        view->vtbl->func_uint32(view, value);
}

/*---------------------------------------------------------------------------*/

void _view_image(View *view, const Image *image)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    if (view->vtbl != NULL && view->vtbl->func_image != NULL)
        view->vtbl->func_image(view, image);
}

/*---------------------------------------------------------------------------*/

void view_get_size(const View *view, S2Df *size)
{
    cassert_no_null(view);
    cassert_no_null(size);
    view->component.context->func_get_size[ekGUI_TYPE_CUSTOMVIEW](view->component.ositem, &size->width, &size->height);
}

/*---------------------------------------------------------------------------*/

void view_content_size(View *view, const S2Df size, const S2Df line)
{
    cassert_no_null(view);
    view->component.context->func_view_content_size(view->component.ositem, size.width, size.height, line.width, line.height);
}

/*---------------------------------------------------------------------------*/

void view_scroll_x(View *view, const real32_t pos)
{
    cassert_no_null(view);
    view->component.context->func_view_scroll(view->component.ositem, pos, -1);
}

/*---------------------------------------------------------------------------*/

void view_scroll_y(View *view, const real32_t pos)
{
    cassert_no_null(view);
    view->component.context->func_view_scroll(view->component.ositem, -1, pos);
}

/*---------------------------------------------------------------------------*/

void view_scroll_size(const View *view, real32_t *width, real32_t *height)
{
    cassert_no_null(view);
    view->component.context->func_view_scroller_size(view->component.ositem, width, height);
}

/*---------------------------------------------------------------------------*/

void view_scroll_visible(View *view, const bool_t horizontal, const bool_t vertical)
{
    cassert_no_null(view);
    view->component.context->func_view_scroller_visible(view->component.ositem, horizontal, vertical);
}

/*---------------------------------------------------------------------------*/

void view_viewport(const View *view, V2Df *pos, S2Df *size)
{
    cassert_no_null(view);
    if (pos != NULL)
        view->component.context->func_view_scroll_get(view->component.ositem, &pos->x, &pos->y);

    if (size != NULL)
        view->component.context->func_get_size[ekGUI_TYPE_CUSTOMVIEW](view->component.ositem, &size->width, &size->height);
}

/*---------------------------------------------------------------------------*/

void view_point_scale(const View *view, real32_t *scale)
{
    cassert_no_null(view);
    cassert_no_null(scale);
    *scale = view->component.context->func_view_scale_factor(view->component.ositem);
}

/*---------------------------------------------------------------------------*/

void view_update(View *view)
{
    cassert_no_null(view);
    view->component.context->func_view_set_need_display(view->component.ositem);
}

/*---------------------------------------------------------------------------*/

void *view_native(View *view)
{
    /* Get the native view */
    cassert_no_null(view);
    return view->component.context->func_view_get_native_view(view->component.ositem);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlDraw(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_DRAW);
    listener_pass_handler(view->vtbl->OnDraw, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlOverlay(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_OVERLAY);
    listener_pass_handler(view->vtbl->OnOverlay, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlEnter(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_ENTER);
    listener_pass_handler(view->vtbl->OnEnter, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlExit(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_EXIT);
    listener_pass_handler(view->vtbl->OnExit, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlMoved(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_MOVED);
    listener_pass_handler(view->vtbl->OnMoved, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlDown(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_DOWN);
    listener_pass_handler(view->vtbl->OnDown, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlUp(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_UP);
    listener_pass_handler(view->vtbl->OnUp, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlClick(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_CLICK);
    listener_pass_handler(view->vtbl->OnClick, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlDrag(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_DRAG);
    listener_pass_handler(view->vtbl->OnDrag, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlWheel(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_WHEEL);
    listener_pass_handler(view->vtbl->OnWheel, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlKeyDown(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_KEYDOWN);
    listener_pass_handler(view->vtbl->OnKeyDown, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlKeyUp(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_KEYUP);
    listener_pass_handler(view->vtbl->OnKeyUp, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_FOCUS);
    listener_pass_handler(view->vtbl->OnFocus, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlResignFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_FOCUS_RESIGN);
    listener_pass_handler(view->vtbl->OnResignFocus, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlAcceptFocus(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_FOCUS_ACCEPT);
    listener_pass_handler(view->vtbl->OnAcceptFocus, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

static void i_OnCtrlScroll(View *view, Event *e)
{
    cassert_no_null(view);
    cassert_no_null(view->vtbl);
    cassert(event_type(e) == ekGUI_EVENT_SCROLL);
    listener_pass_handler(view->vtbl->OnScroll, view->data, e, view, View);
}

/*---------------------------------------------------------------------------*/

View *_vctrl_create_imp(const uint32_t flags, const VCtrlTbl *tbl, void *data, const char_t *type)
{
    View *view = i_create(flags, tbl);
    cassert_no_null(tbl);
    cassert_no_null(data);
    unref(type);
    view->data = data;

    if (tbl->OnDraw != NULL)
        view->component.context->func_view_OnDraw(view->component.ositem, listener(view, i_OnCtrlDraw, View));

    if (tbl->OnOverlay != NULL)
        view->component.context->func_view_OnOverlay(view->component.ositem, listener(view, i_OnCtrlOverlay, View));

    if (tbl->OnEnter != NULL)
        view->component.context->func_view_OnEnter(view->component.ositem, listener(view, i_OnCtrlEnter, View));

    if (tbl->OnExit != NULL)
        view->component.context->func_view_OnExit(view->component.ositem, listener(view, i_OnCtrlExit, View));

    if (tbl->OnMoved != NULL)
        view->component.context->func_view_OnMoved(view->component.ositem, listener(view, i_OnCtrlMoved, View));

    if (tbl->OnDown != NULL)
        view->component.context->func_view_OnDown(view->component.ositem, listener(view, i_OnCtrlDown, View));

    if (tbl->OnUp != NULL)
        view->component.context->func_view_OnUp(view->component.ositem, listener(view, i_OnCtrlUp, View));

    if (tbl->OnClick != NULL)
        view->component.context->func_view_OnClick(view->component.ositem, listener(view, i_OnCtrlClick, View));

    if (tbl->OnDrag != NULL)
        view->component.context->func_view_OnDrag(view->component.ositem, listener(view, i_OnCtrlDrag, View));

    if (tbl->OnWheel != NULL)
        view->component.context->func_view_OnWheel(view->component.ositem, listener(view, i_OnCtrlWheel, View));

    if (tbl->OnKeyDown != NULL)
        view->component.context->func_view_OnKeyDown(view->component.ositem, listener(view, i_OnCtrlKeyDown, View));

    if (tbl->OnKeyUp != NULL)
        view->component.context->func_view_OnKeyUp(view->component.ositem, listener(view, i_OnCtrlKeyUp, View));

    if (tbl->OnFocus != NULL)
        view->component.context->func_view_OnFocus(view->component.ositem, listener(view, i_OnCtrlFocus, View));

    if (tbl->OnResignFocus != NULL)
        view->component.context->func_view_OnResignFocus(view->component.ositem, listener(view, i_OnCtrlResignFocus, View));

    if (tbl->OnAcceptFocus != NULL)
        view->component.context->func_view_OnAcceptFocus(view->component.ositem, listener(view, i_OnCtrlAcceptFocus, View));

    if (tbl->OnScroll != NULL)
        view->component.context->func_view_OnScroll(view->component.ositem, listener(view, i_OnCtrlScroll, View));

    return view;
}

/*---------------------------------------------------------------------------*/

Cell *_vctrl_cell(View *view)
{
    cassert_no_null(view);
    return _component_cell(&view->component);
}

/*---------------------------------------------------------------------------*/

void _vctrl_add_transition(View *view, Listener *listener)
{
    cassert_no_null(view);
    _gui_add_transition(view, listener, View);
}

/*---------------------------------------------------------------------------*/

void _vctrl_delete_transition(View *view)
{
    _gui_delete_transition(view, View);
}
