/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osupdown.c
 *
 */

/* Operating System native updown */

#include "osupdown_win.inl"
#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "ospanel_win.inl"
#include "oswindow_win.inl"
#include "../osupdown.h"
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#include <sewer/nowarn.hxx>
#include <Commctrl.h>
#include <sewer/warn.hxx>

struct _osupdown_t
{
    OSControl control;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSUpDown *updown = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSUpDown);
    cassert_no_null(updown);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(updown, OSControl)) == TRUE)
            break;
        return 0;

    default:
        break;
    }

    return CallWindowProc(updown->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSUpDown *osupdown_create(const uint32_t flags)
{
    OSUpDown *updown = NULL;
    DWORD dwStyle = 0;
    unref(flags);
    updown = heap_new0(OSUpDown);
    updown->control.type = ekGUI_TYPE_UPDOWN;
    dwStyle = WS_CHILD | WS_CLIPSIBLINGS | UDS_ARROWKEYS;
    _oscontrol_init(cast(updown, OSControl), PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), dwStyle, UPDOWN_CLASS, 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    updown->control.tooltip_hwnd1 = updown->control.hwnd;
    _oscontrol_set_frame(cast(updown, OSControl), 0, 0, 20, 20);
    return updown;
}

/*---------------------------------------------------------------------------*/

void osupdown_destroy(OSUpDown **updown)
{
    cassert_no_null(updown);
    cassert_no_null(*updown);
    listener_destroy(&(*updown)->OnClick);
    _oscontrol_destroy(&(*updown)->control);
    heap_delete(updown, OSUpDown);
}

/*---------------------------------------------------------------------------*/

void osupdown_OnClick(OSUpDown *updown, Listener *listener)
{
    cassert_no_null(updown);
    listener_update(&updown->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void osupdown_tooltip(OSUpDown *updown, const char_t *text)
{
    _oscontrol_tooltip(cast(updown, OSControl), text);
}

/*---------------------------------------------------------------------------*/

void osupdown_attach(OSUpDown *updown, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(updown, OSControl));
}

/*---------------------------------------------------------------------------*/

void osupdown_detach(OSUpDown *updown, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(updown, OSControl));
}

/*---------------------------------------------------------------------------*/

void osupdown_visible(OSUpDown *updown, const bool_t visible)
{
    _oscontrol_set_visible(cast(updown, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osupdown_enabled(OSUpDown *updown, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(updown, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void osupdown_size(const OSUpDown *updown, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(updown, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osupdown_origin(const OSUpDown *updown, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(updown, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osupdown_frame(OSUpDown *updown, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(updown, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _osupdown_OnNotification(OSUpDown *updown, const NMHDR *nmhdr, LPARAM lParam)
{
    cassert_no_null(updown);
    cassert_no_null(nmhdr);
    if (nmhdr->code == UDN_DELTAPOS)
    {
        if (IsWindowEnabled(updown->control.hwnd) && updown->OnClick != NULL)
        {
            NMUPDOWN *lpnmud = cast(lParam, NMUPDOWN);
            EvButton params;
            params.text = "";
            params.state = ekGUI_ON;
            if (lpnmud->iDelta < 0)
                params.index = 0;
            else
                params.index = 1;
            listener_event(updown->OnClick, ekGUI_EVENT_UPDOWN, updown, &params, NULL, OSUpDown, EvButton, void);
        }
    }
}
