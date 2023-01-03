/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osscroll.c
 *
 */

/* Windows scrolled view logic */

#include "osscroll.inl"
#include "osgui_win.inl"
#include "cassert.h"
#include "heap.h"
#include "ptr.h"

struct _osscroll_t
{
    HWND hwnd;
    HWND hscroll;
    HWND vscroll;
    int view_width;
    int view_height;
    int content_width;
    int content_height;
    int line_width;
    int line_height;
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

OSScroll *osscroll_create(HWND hwnd, const bool_t horizontal, const bool_t vertical)
{
    OSScroll *scroll = heap_new0(OSScroll);
    DWORD dwStyle = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    scroll->hwnd = hwnd;

    // The window has standard horizontal scrollbar
    if (dwStyle & WS_HSCROLL)
        scroll->hscroll = hwnd;
    else if (horizontal == TRUE)
        scroll->hscroll = i_create_scroll(SBS_HORZ, hwnd, 100, GetSystemMetrics(SM_CXHSCROLL));

    // The window has standard vertical scrollbar
    if (dwStyle & WS_VSCROLL)
        scroll->vscroll = hwnd;
    else if (vertical == TRUE)
        scroll->vscroll = i_create_scroll(SBS_VERT, hwnd, GetSystemMetrics(SM_CXVSCROLL), 100);

    cassert(scroll->hscroll != NULL || scroll->vscroll != NULL);
	return scroll;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_scroll(HWND scroll, HWND hwnd)
{
    HWND ret0 = NULL;
    BOOL ret1 = 0;
    cassert_unref(GetParent(scroll) == hwnd, hwnd);
    ret0 = SetParent(scroll, NULL);
    cassert_unref(ret0 == hwnd, ret0);
    ret1 = DestroyWindow(scroll);
    cassert_unref(ret1 != 0, ret1);
}

/*---------------------------------------------------------------------------*/

void osscroll_destroy(OSScroll **scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(*scroll);

    if ((*scroll)->hscroll != NULL && (*scroll)->hscroll != (*scroll)->hwnd)
        i_destroy_scroll((*scroll)->hscroll, (*scroll)->hwnd);

    if ((*scroll)->vscroll != NULL && (*scroll)->vscroll != (*scroll)->hwnd)
        i_destroy_scroll((*scroll)->vscroll, (*scroll)->hwnd);

    heap_delete(scroll, OSScroll);
}

/*---------------------------------------------------------------------------*/

static __INLINE int i_horbar(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(scroll->hscroll);
    return scroll->hscroll == scroll->hwnd ? SB_HORZ : SB_CTL;
}

/*---------------------------------------------------------------------------*/

static __INLINE int i_verbar(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    cassert_no_null(scroll->vscroll);
    return scroll->vscroll == scroll->hwnd ? SB_VERT : SB_CTL;
}

/*---------------------------------------------------------------------------*/

void osscroll_visible_area(OSScroll *scroll, int *x, int *y, int *width, int *height, int *total_width, int *total_height)
{
    cassert_no_null(scroll);
    cassert_no_null(x);
    cassert_no_null(y);
    cassert_no_null(width);
    cassert_no_null(height);
    if (scroll->hscroll != NULL)
    {
        *x = GetScrollPos(scroll->hscroll, i_horbar(scroll));
        *width = scroll->view_width;
        ptr_assign(total_width, scroll->content_width);
    }
    else
    {
        *x = 0;
        *width = scroll->view_width;
        ptr_assign(total_width, scroll->view_width);
    }

    if (scroll->vscroll != NULL)
    {
        *y = GetScrollPos(scroll->vscroll, i_verbar(scroll));
        *height = scroll->view_height;
        ptr_assign(total_height, scroll->content_height);
    }
    else
    {
        *y = 0;
        *height = scroll->view_height;
        ptr_assign(total_height, scroll->view_height);
    }
}

/*---------------------------------------------------------------------------*/

int osscroll_x_pos(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    if (scroll->hscroll != NULL)
        return GetScrollPos(scroll->hscroll, i_horbar(scroll));
    return 0;
}

/*---------------------------------------------------------------------------*/

int osscroll_y_pos(const OSScroll *scroll)
{
    cassert_no_null(scroll);
    if (scroll->vscroll != NULL)
        return GetScrollPos(scroll->vscroll, i_verbar(scroll));
    return 0;
}

/*---------------------------------------------------------------------------*/

int osscroll_bar_width(const OSScroll *scroll, const bool_t check_if_visible)
{
    if (check_if_visible == TRUE)
    {
        cassert_no_null(scroll);
        if (scroll->vscroll != NULL)
        {
            if (scroll->content_height > scroll->view_height)
                return GetSystemMetrics(SM_CXVSCROLL);
        }

        return 0;
    }
    else
    {
        return GetSystemMetrics(SM_CXVSCROLL);
    }
}

/*---------------------------------------------------------------------------*/

int osscroll_bar_height(const OSScroll *scroll, const bool_t check_if_visible)
{
    if (check_if_visible == TRUE)
    {
        cassert_no_null(scroll);
        if (scroll->hscroll != NULL)
        {
            if (scroll->content_width > scroll->view_width)
                return GetSystemMetrics(SM_CXHSCROLL);
        }

        return 0;
    }
    else
    {
        return GetSystemMetrics(SM_CXHSCROLL);
    }       
}

/*---------------------------------------------------------------------------*/

static int i_incr(HWND scroll, int nBar, int incr)
{
    SCROLLINFO si;
    int pos = 0;
    BOOL ok;
    
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL;
    ok = GetScrollInfo(scroll, nBar, &si);
    cassert_unref(ok != 0, ok);

    pos = si.nPos + incr;

    if (pos < 0)
        pos = 0;
    else if (pos > si.nMax - (int)si.nPage)
        pos = si.nMax - si.nPage;

    if (si.nPos != pos)
    {
        SetScrollPos(scroll, nBar, pos, TRUE);
        return si.nPos - pos;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

bool_t osscroll_wheel(OSScroll *scroll, WPARAM wParam, const bool_t update_children)
{
    cassert_no_null(scroll);
    if (scroll->vscroll != NULL && scroll->content_height > scroll->view_height)
    {
        int dY = scroll->line_height;
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
            dY = - scroll->line_height;

        dY = i_incr(scroll->vscroll, i_verbar(scroll), dY);

        if (dY != 0)
        {
            if (update_children == TRUE)
                ScrollWindowEx(scroll->hwnd, 0, dY, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);

            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

void osscroll_message(OSScroll *scroll, WPARAM wParam, UINT nMsg, const bool_t update_children)
{
    WORD lw = 0;
    cassert_no_null(scroll);
    lw = LOWORD(wParam);
    if (lw != SB_ENDSCROLL)
    {
        SCROLLINFO si;
        BOOL ok;
        BOOL redraw = FALSE;
        int current_pos, pos, max;
        int line_size = 0;
        HWND hwnd;
        int nBar = 0;

        if (nMsg == WM_HSCROLL)
        {
            line_size = scroll->line_width;
            hwnd = scroll->hscroll;
            nBar = i_horbar(scroll);
        }
        else
        {
            cassert(nMsg == WM_VSCROLL);
            line_size = scroll->line_height;
            hwnd = scroll->vscroll;
            nBar = i_verbar(scroll);
        }

        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        ok = GetScrollInfo(hwnd, nBar, &si);
        cassert(ok != 0);
        current_pos = si.nPos;
        pos = si.nPos;
        max = si.nMax - si.nPage;
        switch (lw) {
        case SB_TOP:
            pos = 0;
            break;

        case SB_BOTTOM:
            pos = max;
            break;

        case SB_LINEUP:
        case SB_PAGEUP:
            pos -= line_size;
            if (pos < 0)
                pos = 0;
            redraw = TRUE;
            break;

        case SB_LINEDOWN:
        case SB_PAGEDOWN:
            pos += line_size;
            if (pos > max)
                pos = max;
            redraw = TRUE;
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            pos = si.nTrackPos;
            break;

        cassert_default();
        }

        if (current_pos != pos)
        {
            SetScrollPos(hwnd, nBar, pos, TRUE);

            if (update_children == TRUE)
            {
                int dx = 0;
                int dy = 0;

                if (nMsg == WM_HSCROLL)
                    dx = current_pos - pos;
                else
                    dy = current_pos - pos;

                ScrollWindowEx(hwnd, dx, dy, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void osscroll_set(OSScroll *scroll, const int x, const int y, const bool_t update_children)
{
    int lx = x;
    int ly = y;

    cassert_no_null(scroll);

    if (lx != INT_MAX)
    {
        if (lx < 0)
            lx = 0;
        else if (lx > scroll->content_width - scroll->view_width)
            lx = scroll->content_width - scroll->view_width;
    }

    if (ly != INT_MAX)
    {
        if (ly < 0)
            ly = 0;
        else if (ly > scroll->content_height - scroll->view_height)
            ly = scroll->content_height - scroll->view_height;
    }

    if (update_children == TRUE)
    {
        int dx = 0;
        int dy = 0;

        if (lx != INT_MAX && scroll->hscroll != NULL)
        {
            int cx = GetScrollPos(scroll->hscroll, i_horbar(scroll));
            SetScrollPos(scroll->hscroll, i_horbar(scroll), lx, TRUE);
            dx = cx - lx;
        }

        if (ly != INT_MAX && scroll->vscroll != NULL)
        {
            int cy = GetScrollPos(scroll->vscroll, i_verbar(scroll));
            SetScrollPos(scroll->vscroll, i_verbar(scroll), ly, TRUE);
            dy = cy - ly;
        }

        ScrollWindowEx(scroll->hwnd, dx, dy, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    }
    else
    {
        if (lx != INT_MAX && scroll->hscroll != NULL)
            SetScrollPos(scroll->hscroll, i_horbar(scroll), lx, TRUE);

        if (ly != INT_MAX && scroll->vscroll != NULL)
            SetScrollPos(scroll->vscroll, i_verbar(scroll), ly, TRUE);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_limits(HWND hwnd, const int nBar, const int visible_size, const int total_size)
{
    // Scrollbar update
    if (visible_size < total_size)
    {
        SCROLLINFO si;
        int maxPos = total_size - visible_size;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        GetScrollInfo(hwnd, nBar, &si);
        si.nPage = (UINT)visible_size;
        si.nMin = 0;
        si.nMax = total_size - 1;

        cassert(maxPos >= 0);
        if (si.nPos > maxPos)
            si.nPos = maxPos;

        SetScrollInfo(hwnd, nBar, &si, TRUE);
        return TRUE;
    }
    // Scrollbar is not necessary
    else
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_POS;
        si.nPos = 0;
        SetScrollInfo(hwnd, nBar, &si, FALSE);
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_bars(OSScroll *scroll)
{
    cassert_no_null(scroll);

    if (scroll->hscroll != NULL)
    {
        int nBar = i_horbar(scroll);
        if (i_limits(scroll->hscroll, nBar, scroll->view_width, scroll->content_width) == TRUE)
        {
            BOOL ret = ShowScrollBar(scroll->hscroll, nBar, TRUE);
            cassert_unref(ret != 0, ret);

            if (nBar == SB_CTL)
            {
                ret = SetWindowPos(scroll->hscroll, NULL, 0, scroll->view_height, scroll->view_width, 0, SWP_NOZORDER);
                cassert_unref(ret != 0, ret);
            }
        }
        else
        {
            BOOL ret = ShowScrollBar(scroll->hscroll, nBar, FALSE);
            cassert_unref(ret != 0, ret);
        }
    }

    if (scroll->vscroll != NULL)
    {
        int nBar = i_verbar(scroll);
        if (i_limits(scroll->vscroll, nBar, scroll->view_height, scroll->content_height) == TRUE)
        {
            BOOL ret = ShowScrollBar(scroll->vscroll, nBar, TRUE);
            cassert_unref(ret != 0, ret);

            if (nBar == SB_CTL)
            {
                ret = SetWindowPos(scroll->vscroll, NULL, scroll->view_width, 0, 0, scroll->view_height, SWP_NOZORDER);
                cassert_unref(ret != 0, ret);
            }
        }
        else
        {
            BOOL ret = ShowScrollBar(scroll->vscroll, nBar, FALSE);
            cassert_unref(ret != 0, ret);
        }
    }
}

/*---------------------------------------------------------------------------*/

void osscroll_content_size(OSScroll *scroll, const real32_t width, const real32_t height, const real32_t line_width, const real32_t line_height)
{
    cassert_no_null(scroll);
    scroll->content_width = (int)width;
    scroll->content_height = (int)height;
    scroll->line_width = (int)line_width;
    scroll->line_height = (int)line_height;
    i_update_bars(scroll);
}

/*---------------------------------------------------------------------------*/

void osscroll_control_size(OSScroll *scroll, const real32_t width, const real32_t height)
{
    cassert_no_null(scroll);
    scroll->view_width = (int)width;
    scroll->view_height = (int)height;
    i_update_bars(scroll);
}

