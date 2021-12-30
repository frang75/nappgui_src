/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osview.cpp
 *
 */

/* Operating System native custom view */

#include "osview.h"
#include "osview.inl"
#include "osgui.inl"
#include "osgui_win.inl"
#include "oscontrol.inl"
#include "oswindow.inl"
#include "oslistener.inl"
#include "osscroll.inl"
#include "ospanel.inl"
#include "win/osstyleXP.inl"
#include "dctx.inl"
#include "draw.h"
#include "draw.inl"

#include "cassert.h"
#include "color.h"
#include "heap.h"
#include "event.h"
#include "osbs.h"
#include "ptr.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#include "nowarn.hxx"
#include <gdiplus.h>
#include <Commctrl.h>
#include "warn.hxx"

struct _osview_t
{
    OSControl control;
    OSPanel *parent_panel;
    OSScroll *scroll;
    DCtx *ctx;
    uint32_t flags;
    HBITMAP dbuffer;
    LONG dbuffer_width;
    LONG dbuffer_height;
    ViewListeners listeners;
    OSDraw osdraw;
    Listener *OnFocus;
    Listener *OnNotify;
};

/*---------------------------------------------------------------------------*/

#define i_color(c)\
    Gdiplus::Color((BYTE)(c), (BYTE)((c) >> 8), (BYTE)((c) >> 16))

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSView *view = (OSView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    cassert_no_null(view);
    cassert(view->control.type == ekGUI_COMPONENT_CUSTOMVIEW);
    cassert(view->control.hwnd == hwnd);

	switch(uMsg) {
    case WM_ERASEBKGND:
        return 1;

    case WM_PRINTCLIENT:
        cassert(FALSE);
        oslistener_draw((OSControl*)view, NULL, 0, 0, 0, 0, 0, 0, &view->listeners);
        return 0;

    case WM_NCPAINT:
        if (view->flags & ekBORDER)
        {
            // Very bad redraw effects. We leave the default border color.
            //RECT rect;
            //COLORREF color;
            //HPEN pen = NULL;
            //HRGN reg = NULL;
            //HDC hdc = NULL;
            //HGDIOBJ cpen = NULL;
            //GetWindowRect(hwnd, &rect);
            //color = GetSysColor(COLOR_ACTIVEBORDER);
            //pen = CreatePen(PS_INSIDEFRAME, 1, color);
            //reg = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
            //hdc = GetDCEx(hwnd, reg, DCX_WINDOW | DCX_INTERSECTRGN | DCX_CACHE);
            //cpen = SelectObject(hdc, pen);
            //Rectangle(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top);
            //SelectObject(hdc, cpen);
            //ReleaseDC(hwnd, hdc);
            //DeleteObject(reg);
            //DeleteObject(pen);
        }
        break;

    case WM_PAINT:
    {
        LONG_PTR edata;

        if (_oswindow_in_resizing(hwnd) == TRUE)
            return 0;

        // Extra data is defined in 'i_registry_view_class::cbWndExtra'
        // 0 means GDI/GDI+ based drawing
        edata = GetWindowLongPtr(hwnd, 0);
        if (edata == 0)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(view->control.hwnd, &ps);
            HDC memHdc = CreateCompatibleDC(hdc);

            if (view->ctx == NULL)
                view->ctx = dctx_create((void*)&view->osdraw);

            if (__TRUE_EXPECTED(view->dbuffer == NULL))
                view->dbuffer = CreateCompatibleBitmap(hdc, view->dbuffer_width, view->dbuffer_height);

            // Draw on an image back-buffer
            SelectObject(memHdc, view->dbuffer);

            // Can be called from OSView attached to OSSplit (not OSPanel)
            uint32_t background;
            if (view->parent_panel != NULL)
            {
                cassert(((OSControl*)view->parent_panel)->type == ekGUI_COMPONENT_PANEL);
                background = (uint32_t)_ospanel_background_color(view->parent_panel, hwnd);
            }
            else
            {
                background = (uint32_t)GetSysColor(COLOR_3DFACE);
            }

            Gdiplus::Graphics *graphics = new Gdiplus::Graphics(memHdc);

            // Don't delete --> ImageView-like controls with standard background
            if ((view->flags & ekNOERASE) == 0)
                graphics->Clear(i_color(background));

            int vx = 0;
            int vy = 0;
            int vwidth = view->dbuffer_width;
            int vheight = view->dbuffer_height;
            int twidth = vwidth;
            int theight = vheight;

            if (view->scroll != NULL)
                osscroll_visible_area(view->scroll, &vx, &vy, &vwidth, &vheight, &twidth, &theight);

            void *ctx[2];
            ctx[0] = graphics;
            ctx[1] = memHdc;
            dctx_set_gcontext(view->ctx, ctx, (uint32_t)vwidth, (uint32_t)vheight, -(real32_t)vx, -(real32_t)vy, background, TRUE/*(view->flags & ekCONTROL) ? FALSE : TRUE*/);
            oslistener_draw((OSControl*)view, view->ctx, (real32_t)twidth, (real32_t)theight, (real32_t)vx, (real32_t)vy, (real32_t)vwidth, (real32_t)vheight, &view->listeners);

            dctx_unset_gcontext(view->ctx);
            delete graphics;
            graphics = NULL;

            // Back buffer image to window
            BitBlt(hdc, 0, 0, view->dbuffer_width, view->dbuffer_height, memHdc, 0, 0, SRCCOPY);

            BOOL ok = DeleteDC(memHdc);
            cassert_unref(ok != 0, ok);

            EndPaint(hwnd, &ps);
        }
        // The window is rendered with other technology
        else
        {
            oslistener_draw((OSControl*)view, NULL, (real32_t)view->dbuffer_width, (real32_t)view->dbuffer_height, 0, 0, (real32_t)view->dbuffer_width, (real32_t)view->dbuffer_height, &view->listeners);
        }
    }

    return 0;

    // This Window must process mouse events
   // case WM_NCHITTEST:
		 //Automatic SetFocus for Mouse Wheel over view without click
   //     if (osbs_windows() <= ekWIN_8 && (view->listeners.OnWheel != NULL || view->vscroll != NULL))
   //         SetFocus(hwnd);
   //     return HTCLIENT;
//
    case WM_MOUSELEAVE:
        oslistener_mouse_exit((OSControl*)view, &view->listeners);
        return 0;

    case WM_MOUSEMOVE:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_moved((OSControl*)view, wParam, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_down((OSControl*)view, ekMLEFT, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        SetFocus(hwnd);
        return 0;
    }

    case WM_RBUTTONDOWN:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_down((OSControl*)view, ekMRIGHT, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_MBUTTONDOWN:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_down((OSControl*)view, ekMMIDDLE, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_LBUTTONUP:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_up((OSControl*)view, ekMLEFT, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_RBUTTONUP:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_up((OSControl*)view, ekMRIGHT, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_MBUTTONUP:
    {
        POINTS point = MAKEPOINTS(lParam);
        oslistener_mouse_up((OSControl*)view, ekMMIDDLE, (real32_t)point.x, (real32_t)point.y, view->scroll, &view->listeners);
        return 0;
    }

    case WM_MOUSEWHEEL:
        if (view->scroll != NULL)
        {
            if (osscroll_wheel(view->scroll, wParam, FALSE) == TRUE)
                InvalidateRect(view->control.hwnd, NULL, FALSE);
        }

        oslistener_whell((OSControl*)view, wParam, lParam, view->scroll, &view->listeners);
        break;

    case WM_SETFOCUS:
//        if (view->flags & ekBORDER)
//        {
//            SendMessage(hwnd, WM_NCPAINT, 1, 0);
////SetWindowPos(hwnd, 0, 0, 0, 0, 0,
////    SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
////        InvalidateRect(hwnd, NULL, FALSE);
//            //RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
//            //RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
//            //RedrawWindow(hwnd, NULL, NULL, RDW_NOFRAME | RDW_VALIDATE);
//        }
        if (view->OnFocus != NULL)
        {
            bool_t params = TRUE;
            listener_event(view->OnFocus, ekEVFOCUS, view, &params, NULL, OSView, bool_t, void);
        }
        break;

    case WM_KILLFOCUS:
//        if (view->flags & ekBORDER)
//        {
//            SendMessage(hwnd, WM_NCPAINT, 1, 0);
////SetWindowPos(hwnd, 0, 0, 0, 0, 0,
////    SWP_DRAWFRAME|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER);
////        InvalidateRect(hwnd, NULL, FALSE);
////            RedrawWindow(hwnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
//  //          RedrawWindow(hwnd, NULL, NULL, RDW_NOFRAME | RDW_VALIDATE);
//        }
        if (view->OnFocus != NULL)
        {
            bool_t params = FALSE;
            listener_event(view->OnFocus, ekEVFOCUS, view, &params, NULL, OSView, bool_t, void);
        }
        break;


        /*
    case WM_SETFOCUS:
        _oswindow_OnSetFocus((OSControl*)view);
        break;*/

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (oslistener_key_down((OSControl*)view, wParam, lParam, &view->listeners) == TRUE)
            return 0;
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (oslistener_key_up((OSControl*)view, wParam, lParam, &view->listeners) == TRUE)
            return 0;
        break;

    case WM_HSCROLL:
    case WM_VSCROLL:
        osscroll_message(view->scroll, wParam, uMsg, FALSE);
        InvalidateRect(view->control.hwnd, NULL, FALSE);
        return 0;	
    }

    return CallWindowProc(view->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSView *osview_create(const uint32_t flags)
{
    OSView *view = heap_new0(OSView);
    DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    if (flags & ekBORDER)
        dwStyle |= WS_BORDER;

    if (flags & ekHSCROLL)
        dwStyle |= WS_HSCROLL;

    if (flags & ekVSCROLL)
        dwStyle |= WS_VSCROLL;

    _oscontrol_init((OSControl*)view, PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), dwStyle, kVIEW_CLASS, 16, 16, i_WndProc, kDEFAULT_PARENT_WINDOW);
    view->control.type = ekGUI_COMPONENT_CUSTOMVIEW;
    view->flags = flags;

    // Extra data is defined in 'i_registry_view_class::cbWndExtra'
    // 0 means GDI/GDI+ based drawing
    SetWindowLongPtr(view->control.hwnd, 0, 0);

    if ((flags & ekHSCROLL) || (flags & ekVSCROLL))
        view->scroll = osscroll_create(view->control.hwnd, (flags & ekHSCROLL) ? TRUE : FALSE, (flags & ekVSCROLL) ? TRUE : FALSE);

    if (flags & ekCONTROL)
    {
        view->osdraw.button_theme = osstyleXP_OpenTheme(view->control.hwnd, L"BUTTON");
        view->osdraw.list_theme = osstyleXP_OpenTheme(view->control.hwnd, L"Explorer::ListView");
        // WinXP
        if (view->osdraw.list_theme == NULL)
            view->osdraw.list_theme = osstyleXP_OpenTheme(view->control.hwnd, L"ListView");
    }

    oslistener_init(&view->listeners);
    return view;
}

/*---------------------------------------------------------------------------*/

void osview_destroy(OSView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);

    oslistener_remove(&(*view)->listeners);
    listener_destroy(&(*view)->OnFocus);
    listener_destroy(&(*view)->OnNotify);

    if ((*view)->dbuffer != NULL)
    {
        BOOL ok = DeleteObject((*view)->dbuffer);
        cassert_unref(ok != 0, ok);
    }

    if ((*view)->ctx != NULL)
        dctx_destroy(&(*view)->ctx);

    if ((*view)->scroll != NULL)
        osscroll_destroy(&(*view)->scroll);

    if ((*view)->osdraw.button_theme != NULL)
    {
        osstyleXP_CloseTheme((*view)->osdraw.button_theme);
        (*view)->osdraw.button_theme = NULL;
    }

    if ((*view)->osdraw.list_theme != NULL)
    {
        osstyleXP_CloseTheme((*view)->osdraw.list_theme);
        (*view)->osdraw.list_theme = NULL;
    }

    _oscontrol_destroy(&(*view)->control);
    heap_delete(view, OSView);
}

/*---------------------------------------------------------------------------*/

void osview_OnDraw(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDraw, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnEnter(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnEnter, listener);
    view->listeners.is_mouse_inside = FALSE;
}

/*---------------------------------------------------------------------------*/

void osview_OnExit(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnExit, listener);
    view->listeners.is_mouse_inside = FALSE;
}

/*---------------------------------------------------------------------------*/

void osview_OnMoved(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnMoved, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDown, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnUp, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnClick(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnDrag(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnDrag, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnWheel(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnWheel, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyDown(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyDown, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnKeyUp(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->listeners.OnKeyUp, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnFocus(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osview_OnNotify(OSView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnNotify, listener);
}

/*---------------------------------------------------------------------------*/

void osview_scroll(OSView *view, const real32_t x, const real32_t y)
{
    cassert_no_null(view);
    osscroll_set(view->scroll, x, y);
}

/*---------------------------------------------------------------------------*/

void osview_scroll_get(const OSView *view, real32_t *x, real32_t *y)
{
    cassert_no_null(view);
    if (x != NULL)
        *x = (real32_t)osscroll_x_pos(view->scroll);

    if (y != NULL)
        *y = (real32_t)osscroll_y_pos(view->scroll);
}

/*---------------------------------------------------------------------------*/

void osview_scroller_size(const OSView* view, real32_t* width, real32_t* height)
{
    cassert_no_null(view);
    if (width != NULL)
        *width = (real32_t)osscroll_bar_width(view->scroll, TRUE);

    if (height != NULL)
        *height = (real32_t)osscroll_bar_height(view->scroll, TRUE);
}

/*---------------------------------------------------------------------------*/

void osview_content_size(OSView* view, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    cassert_no_null(view);
    osscroll_content_size(view->scroll, width, height, line_width, line_height);
}

/*---------------------------------------------------------------------------*/

real32_t osview_scale_factor(const OSView *view)
{
    unref(view);
    return 1;
}

/*---------------------------------------------------------------------------*/

void osview_set_need_display(OSView *view)
{
    cassert_no_null(view);
    InvalidateRect(view->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void *osview_get_native_view(const OSView *view)
{
    cassert_no_null(view);
    return (void*)view->control.hwnd;
}

/*---------------------------------------------------------------------------*/

void osview_attach(OSView *view, OSPanel *panel)
{
    cassert(view->parent_panel == NULL);
    view->parent_panel = panel;
    _ospanel_attach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void osview_detach(OSView *view, OSPanel *panel)
{
    cassert(view->parent_panel == panel);
    view->parent_panel = NULL;
    _ospanel_detach_control(panel, (OSControl*)view);
}

/*---------------------------------------------------------------------------*/

void osview_visible(OSView *view, const bool_t visible)
{
    _oscontrol_set_visible((OSControl*)view, visible);
}

/*---------------------------------------------------------------------------*/

void osview_enabled(OSView *view, const bool_t enabled)
{
   _oscontrol_set_enabled((OSControl*)view, enabled);
}

/*---------------------------------------------------------------------------*/

void osview_size(const OSView *view, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)view, width, height);
}

/*---------------------------------------------------------------------------*/

void osview_origin(const OSView *view, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)view, x, y);
}

/*---------------------------------------------------------------------------*/

void osview_frame(OSView *view, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    cassert_no_null(view);
    _oscontrol_set_frame((OSControl*)view, x, y, width, height);
    view->dbuffer_width = (LONG)width;
    view->dbuffer_height = (LONG)height;

    if (view->dbuffer != NULL)
    {
        DeleteObject(view->dbuffer);
        view->dbuffer = NULL;
    }

    if (view->scroll != NULL)
        osscroll_control_size(view->scroll, width, height);
}

/*---------------------------------------------------------------------------*/

void _osview_detach_and_destroy(OSView **view, OSPanel *panel)
{
    cassert_no_null(view);
    osview_detach(*view, panel);
    osview_destroy(view);
}
