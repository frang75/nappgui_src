/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.c
 *
 */

/* Operating System split view */

#include "ossplit_win.inl"
#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "oslistener.inl"
#include "ospanel_win.inl"
#include "../ossplit.h"
#include "../osgui.inl"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _ossplit_t
{
    OSControl control;
    uint32_t flags;
    RECT divrect;
    OSControl *child1;
    OSControl *child2;
    bool_t left_button;
    POINTS mouse_st;
    POINTS mouse_pos;
    Listener *OnDrag;
};

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSSplit *split = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSSplit);
    cassert_no_null(split);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_SETCURSOR:
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(split->control.hwnd, &pt);
        if (PtInRect(&split->divrect, pt) == TRUE)
        {
            HCURSOR cursor = split_get_type(split->flags) == ekSPLIT_VERT ? kSIZING_VERTICAL_CURSOR : kSIZING_HORIZONTAL_CURSOR;
            cassert(GetCapture() != split->control.hwnd);
            SetCapture(split->control.hwnd);
            SetCursor(cursor);
            return TRUE;
        }

        break;
    }

    case WM_NCHITTEST:
        return HTCLIENT;

    case WM_MOUSEMOVE:
        cassert(GetCapture() == split->control.hwnd);
        if (split->left_button == TRUE)
        {
            if (split->OnDrag != NULL)
            {
                EvMouse params;
                split->mouse_pos = MAKEPOINTS(lParam);
                params.x = (real32_t)split->mouse_pos.x;
                params.y = (real32_t)split->mouse_pos.y;
                params.lx = (real32_t)split->mouse_st.x;
                params.ly = (real32_t)split->mouse_st.y;
                params.button = ekGUI_MOUSE_LEFT;
                params.count = 0;
                params.modifiers = 0;
                params.tag = 0;
                listener_event(split->OnDrag, ekGUI_EVENT_DRAG, split, &params, NULL, OSSplit, EvMouse, void);
            }
        }
        else
        {
            POINTS point = MAKEPOINTS(lParam);
            POINT pt;
            pt.x = point.x;
            pt.y = point.y;
            if (PtInRect(&split->divrect, pt) == FALSE)
                ReleaseCapture();
        }

        return 0;

    case WM_LBUTTONDOWN:
    {
#if defined __ASSERTS__
        POINTS point;
        POINT pt;
        point = MAKEPOINTS(lParam);
        pt.x = point.x;
        pt.y = point.y;
        cassert(GetCapture() == split->control.hwnd);
        cassert(PtInRect(&split->divrect, pt) == TRUE);
#endif

        split->left_button = TRUE;
        split->mouse_st = MAKEPOINTS(lParam);
        return 0;
    }

    case WM_LBUTTONUP:
        split->left_button = FALSE;
        if (split->OnDrag != NULL)
        {
            EvMouse params;
            split->mouse_pos = MAKEPOINTS(lParam);
            params.x = (real32_t)split->mouse_pos.x;
            params.y = (real32_t)split->mouse_pos.y;
            params.lx = (real32_t)split->mouse_st.x;
            params.ly = (real32_t)split->mouse_st.y;
            params.button = ekGUI_MOUSE_LEFT;
            params.count = 0;
            params.modifiers = 0;
            params.tag = 0;
            listener_event(split->OnDrag, ekGUI_EVENT_UP, split, &params, NULL, OSSplit, EvMouse, void);
        }

        ReleaseCapture();
        return 0;

    default:
        break;
    }

    return CallWindowProc(split->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSSplit *ossplit_create(const uint32_t flags)
{
    OSSplit *view = heap_new0(OSSplit);
    view->control.type = ekGUI_TYPE_SPLITVIEW;
    view->flags = flags;
    /* WS_EX_CONTROLPARENT: Recursive TabStop navigation over view children */
    _oscontrol_init(cast(view, OSControl), PARAM(dwExStyle, WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY), PARAM(dwStyle, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN /*| WS_GROUP | WS_TABSTOP*/), L"static", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    return view;
}

/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    cassert(_oscontrol_num_children(*dcast(view, OSControl)) == 0);
    listener_destroy(&(*view)->OnDrag);
    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSSplit);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(control);
    if (view->child1 == NULL)
    {
        view->child1 = control;
    }
    else
    {
        cassert(view->child2 == NULL);
        view->child2 = control;
    }

    _oscontrol_attach_to_parent(control, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_detach_control(OSSplit *view, OSControl *control)
{
    cassert_no_null(control);
    if (view->child1 == control)
    {
        view->child1 = NULL;
    }
    else
    {
        cassert(view->child2 == control);
        view->child2 = NULL;
    }

    _oscontrol_detach_from_parent(control, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_OnDrag(OSSplit *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnDrag, listener);
}

/*---------------------------------------------------------------------------*/

void ossplit_track_area(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    view->divrect.left = (LONG)x;
    view->divrect.top = (LONG)y;
    view->divrect.right = view->divrect.left + (LONG)width;
    view->divrect.bottom = view->divrect.top + (LONG)height;
}

/*---------------------------------------------------------------------------*/

void ossplit_attach(OSSplit *view, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_detach(OSSplit *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(view, OSControl));
}

/*---------------------------------------------------------------------------*/

void ossplit_visible(OSSplit *view, const bool_t visible)
{
    _oscontrol_set_visible(cast(view, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ossplit_enabled(OSSplit *view, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(view, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(view, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ossplit_origin(const OSSplit *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(view, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ossplit_frame(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(view, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ossplit_children(OSSplit *view, OSControl **child1, OSControl **child2)
{
    cassert_no_null(view);
    cassert_no_null(child1);
    cassert_no_null(child2);
    *child1 = view->child1;
    *child2 = view->child2;
}
