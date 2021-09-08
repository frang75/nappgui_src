/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ossplit.c
 *
 */

/* Operating System split view */

#include "ossplit.h"
#include "osgui.inl"
#include "osgui_win.inl"
#include "oscontrol.inl"
#include "oslistener.inl"
#include "ospanel.inl"
#include "cassert.h"
#include "event.h"
#include "event.inl"
#include "heap.h"
#include "ptr.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _ossplit_t
{
    OSControl control;
    split_flag_t flags;
    RECT divrect;
    ViewListeners listeners;
};

/*---------------------------------------------------------------------------*/

static BOOL i_in_divider_rect(HWND hwnd, const RECT *divrect, const POINT *pt)
{
    if (PtInRect(divrect, *pt) == TRUE)
    {
        SetCapture(hwnd);
        return TRUE;
    }
    else
    {
        ReleaseCapture();
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSSplit *split = (OSSplit*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    static int IC = 0;
    cassert_no_null(split);

    switch (uMsg)
    {
        //case WM_PAINT:
        //{
        //    static int I = 0;
        //    BOOL ret;
        //    RECT frame, rect;
        //    HDC hdc;
        //    PAINTSTRUCT ps;
        //    ret = GetClientRect(hwnd, &frame);
	       // cassert(ret != 0);
        //    rect = frame;
        //    hdc = BeginPaint(split->control.hwnd, &ps);
        //    FillRect(hdc, &rect, CreateSolidBrush(RGB(255 / ((I%2)+1), 200, 200)));
        //    I+=1;
        //    EndPaint(hwnd, &ps);
        //}

        //break;

        case WM_SETCURSOR:
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(split->control.hwnd, &pt);
            if (i_in_divider_rect(split->control.hwnd, &split->divrect, &pt) == TRUE)
            {
                HCURSOR cursor = split_type(split->flags) == ekSPVERT ? kSIZING_VERTICAL_CURSOR : kSIZING_HORIZONTAL_CURSOR;
                SetCursor(cursor);
                return TRUE;
            }

            break;
        }

        case WM_NCHITTEST:
            return HTCLIENT;

        case WM_MOUSEMOVE:

            if (split->listeners.button == ekMLEFT)
            {
                POINTS point = MAKEPOINTS(lParam);
                oslistener_mouse_moved((OSControl*)split, wParam, (real32_t)point.x, (real32_t)point.y, NULL, &split->listeners);
                InvalidateRect(split->control.hwnd, NULL, FALSE);
            }
            else if (split->listeners.button == ENUM_MAX(mouse_t))
            {
                POINTS point;
                POINT pt;
                point = MAKEPOINTS(lParam);
                pt.x = point.x;
                pt.y = point.y;
                if (i_in_divider_rect(split->control.hwnd, &split->divrect, &pt) == TRUE)
                {
                    HCURSOR cursor = split_type(split->flags) == ekSPVERT ? kSIZING_VERTICAL_CURSOR : kSIZING_HORIZONTAL_CURSOR;
                    SetCursor(cursor);
                }
            }

            break;

        case WM_LBUTTONDOWN:
        {
            POINTS point;
            POINT pt;
            point = MAKEPOINTS(lParam);
            pt.x = point.x;
            pt.y = point.y;
            if (i_in_divider_rect(split->control.hwnd, &split->divrect, &pt) == TRUE)
            {
                cassert(FALSE);
                //oslistener_mouse_down(hwnd, ekMLEFT, &split->listeners);
                return 0;
            }

            break;
        }

        case WM_LBUTTONUP:

            if (split->listeners.button == ekMLEFT)
            {
                cassert(FALSE);
                //oslistener_mouse_up((OSControl*)split, lParam, ekMLEFT, &split->listeners);
                return 0;
            }

            break;
    }

    return CallWindowProc(split->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSSplit *ossplit_create(const split_flag_t flags)
{
    OSSplit *view = heap_new0(OSSplit);
    view->control.type = ekGUI_COMPONENT_SPLITVIEW;
    view->flags = flags;
    /* WS_EX_CONTROLPARENT: Recursive TabStop navigation over view children */
    _oscontrol_init((OSControl*)view, PARAM(dwExStyle, WS_EX_CONTROLPARENT | WS_EX_NOPARENTNOTIFY), PARAM(dwStyle, WS_CHILD | WS_CLIPSIBLINGS /*| WS_GROUP | WS_TABSTOP*/), L"static", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    oslistener_init(&view->listeners);
	return view;
}

/*---------------------------------------------------------------------------*/

void ossplit_destroy(OSSplit **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    cassert(_oscontrol_num_children((*view)->control.hwnd) == 0);
    oslistener_remove(&(*view)->listeners);
    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSSplit);
}

/*---------------------------------------------------------------------------*/

void ossplit_attach_control(OSSplit *view, OSControl *control)
{
    _oscontrol_attach_to_parent(control, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach_control(OSSplit *view, OSControl *control)
{
    _oscontrol_detach_from_parent(control, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_OnMoved(OSSplit *view, Listener *listener)
{
    cassert_no_null(view);
    cassert(FALSE);
    unref(listener);
    //listener_update(&view->listeners.OnMouseStartDrag, listener_copy(listener));
    //listener_update(&view->listeners.OnMouseDragging, listener_copy(listener));
    //listener_update(&view->listeners.OnMouseEndDrag, listener);
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
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_detach(OSSplit *view, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void ossplit_visible(OSSplit *view, const bool_t is_visible)
{
    _oscontrol_set_visible((OSControl*)view, is_visible);
}

/*---------------------------------------------------------------------------*/

void ossplit_enabled(OSSplit *view, const bool_t is_enabled)
{
    _oscontrol_set_enabled((OSControl*)view, is_enabled);
}

/*---------------------------------------------------------------------------*/

void ossplit_size(const OSSplit *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void ossplit_origin(const OSSplit *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void ossplit_frame(OSSplit *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
}
