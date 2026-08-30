/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osscroll.c
 *
 */

/* Operating System native scrollbar */

#include "osscroll_win.inl"
#include "osgui_win.inl"
#include "oswindow_win.inl"
#include "../osscroll.inl"
#include <core/heap.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

struct _osscroll_t
{
    int type;
    HWND hwnd;
};

/*---------------------------------------------------------------------------*/

static HWND i_create_scroll(DWORD type, HWND hwnd, int width, int height)
{
    return CreateWindowEx(
        0, L"SCROLLBAR", NULL,
        WS_CHILD | WS_CLIPSIBLINGS | type,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        hwnd,
        (HMENU)NULL,
        _osgui_instance(),
        NULL);
}

/*---------------------------------------------------------------------------*/

OSScroll *_osscroll_horizontal(OSControl *control)
{
    OSScroll *scroll = heap_new(OSScroll);
    DWORD dwStyle = 0;
    cassert_no_null(control);
    dwStyle = (DWORD)GetWindowLong(control->hwnd, GWL_STYLE);

    /* The window has standard horizontal scrollbar */
    if (dwStyle & WS_HSCROLL)
    {
        scroll->type = SBS_HORZ;
        scroll->hwnd = control->hwnd;
    }
    else
    {
        scroll->type = SB_CTL;
        scroll->hwnd = i_create_scroll((DWORD)scroll->type, control->hwnd, 100, _osgui_system_metrics_for_dpi(SM_CXHSCROLL, USER_DEFAULT_SCREEN_DPI));
    }

    return scroll;
}

/*---------------------------------------------------------------------------*/

OSScroll *_osscroll_vertical(OSControl *control)
{
    OSScroll *scroll = heap_new(OSScroll);
    DWORD dwStyle = 0;
    cassert_no_null(control);
    dwStyle = (DWORD)GetWindowLong(control->hwnd, GWL_STYLE);

    /* The window has standard vertical scrollbar */
    if (dwStyle & WS_VSCROLL)
    {
        scroll->type = SBS_VERT;
        scroll->hwnd = control->hwnd;
    }
    else
    {
        scroll->type = SB_CTL;
        scroll->hwnd = i_create_scroll(SBS_VERT, control->hwnd, _osgui_system_metrics_for_dpi(SM_CXVSCROLL, USER_DEFAULT_SCREEN_DPI), 100);
    }

    return scroll;
}

/*---------------------------------------------------------------------------*/

void _osscroll_destroy(OSScroll **scroll, OSControl *control)
{
    cassert_no_null(scroll);
    cassert_no_null(*scroll);
    cassert_no_null(control);
    if ((*scroll)->hwnd != control->hwnd)
    {
        HWND ret0 = NULL;
        BOOL ret1 = 0;
        cassert(GetParent((*scroll)->hwnd) == control->hwnd);
        ret0 = SetParent((*scroll)->hwnd, NULL);
        cassert_unref(ret0 == control->hwnd, ret0);
        ret1 = DestroyWindow((*scroll)->hwnd);
        cassert_unref(ret1 != 0, ret1);
    }

    heap_delete(scroll, OSScroll);
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_pos(const OSScroll *scroll)
{
    return (uint32_t)GetScrollPos(scroll->hwnd, scroll->type);
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_trackpos(const OSScroll *scroll)
{
    SCROLLINFO si;
    BOOL ok;
    cassert_no_null(scroll);
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_TRACKPOS;
    ok = GetScrollInfo(scroll->hwnd, scroll->type, &si);
    cassert_unref(ok != 0, ok);
    return (uint32_t)si.nTrackPos;
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_bar_width(const OSScroll *scroll)
{
    unref(scroll);
    return (uint32_t)_osgui_system_metrics_for_dpi(SM_CXVSCROLL, USER_DEFAULT_SCREEN_DPI);
}

/*---------------------------------------------------------------------------*/

uint32_t _osscroll_bar_height(const OSScroll *scroll)
{
    unref(scroll);
    return (uint32_t)_osgui_system_metrics_for_dpi(SM_CXHSCROLL, USER_DEFAULT_SCREEN_DPI);
}

/*---------------------------------------------------------------------------*/

void _osscroll_set_pos(OSScroll *scroll, const uint32_t pos)
{
    cassert_no_null(scroll);
    SetScrollPos(scroll->hwnd, scroll->type, (int)pos, TRUE);
}

/*---------------------------------------------------------------------------*/

void _osscroll_visible(OSScroll *scroll, const bool_t visible)
{
    BOOL ret;
    cassert_no_null(scroll);
    ret = ShowScrollBar(scroll->hwnd, scroll->type, visible);
    cassert_unref(ret != 0, ret);
}

/*---------------------------------------------------------------------------*/

void _osscroll_config(OSScroll *scroll, const uint32_t pos, const uint32_t max, const uint32_t page)
{
    SCROLLINFO si;
    cassert_no_null(scroll);
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nPage = (UINT)page;
    si.nMin = 0;
    si.nMax = (int)max;
    si.nPos = (int)pos;
    SetScrollInfo(scroll->hwnd, scroll->type, &si, FALSE);
}

/*---------------------------------------------------------------------------*/

void _osscroll_frame(OSScroll *scroll, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
{
    cassert_no_null(scroll);
    if (scroll->type == SB_CTL)
    {
        /* 'x'/'y'/'width'/'height' arrive in logical screen points - scaled to real pixels here, right before the raw SetWindowPos() call */
        real32_t scale = (real32_t)_osgui_dpi_for_window(scroll->hwnd) / (real32_t)USER_DEFAULT_SCREEN_DPI;
        int32_t px = (int32_t)bmath_roundf((real32_t)x * scale);
        int32_t py = (int32_t)bmath_roundf((real32_t)y * scale);
        int32_t pwidth = (int32_t)bmath_roundf((real32_t)width * scale);
        int32_t pheight = (int32_t)bmath_roundf((real32_t)height * scale);
        /* The control-owner scrollbars are automatically positioned  */
        BOOL ret = SetWindowPos(scroll->hwnd, NULL, (int)px, (int)py, (int)pwidth, (int)pheight, SWP_NOZORDER);
        cassert_unref(ret != 0, ret);
    }
}

/*---------------------------------------------------------------------------*/

void _osscroll_control_scroll(OSControl *control, const int32_t incr_x, const int32_t incr_y)
{
    real32_t scale;
    int32_t pincr_x = 0, pincr_y = 0;
    cassert_no_null(control);
    scale = _oswindow_scale(control->window);

    if (incr_x != 0)
    {
        int32_t new_x = (int32_t)GetScrollPos(control->hwnd, SB_HORZ);
        int32_t old_x = new_x + incr_x;
        pincr_x = (int32_t)bmath_roundf((real32_t)old_x * scale) - (int32_t)bmath_roundf((real32_t)new_x * scale);
    }

    if (incr_y != 0)
    {
        int32_t new_y = (int32_t)GetScrollPos(control->hwnd, SB_VERT);
        int32_t old_y = new_y + incr_y;
        pincr_y = (int32_t)bmath_roundf((real32_t)old_y * scale) - (int32_t)bmath_roundf((real32_t)new_y * scale);
    }

    ScrollWindowEx(control->hwnd, (int)pincr_x, (int)pincr_y, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
}

/*---------------------------------------------------------------------------*/

gui_scroll_t _osscroll_event(WPARAM wParam)
{
    WORD lw = LOWORD(wParam);
    switch (lw)
    {
    case SB_TOP:
        return ekGUI_SCROLL_BEGIN;
    case SB_BOTTOM:
        return ekGUI_SCROLL_END;
    case SB_LINEUP:
        return ekGUI_SCROLL_STEP_LEFT;
    case SB_LINEDOWN:
        return ekGUI_SCROLL_STEP_RIGHT;
    case SB_PAGEUP:
        return ekGUI_SCROLL_PAGE_LEFT;
    case SB_PAGEDOWN:
        return ekGUI_SCROLL_PAGE_RIGHT;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        return ekGUI_SCROLL_THUMB;
    case SB_ENDSCROLL:
        break;
    default:
        cassert_default(lw);
    }

    return ENUM_MAX(gui_scroll_t);
}
