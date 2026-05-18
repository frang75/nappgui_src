/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ostabs.c
 *
 */

/* Operating System native tabctrl */

#include "osgui_win.inl"
#include "ostabs_win.inl"
#include "oscontrol_win.inl"
#include "ospanel_win.inl"
#include "oswindow_win.inl"
#include "osimglist.inl"
#include "../ostabs.h"
#include "../ostabs.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _ostabs_t
{
    OSControl control;
    uint32_t flags;
    Font *font;
    OSImgList *image_list;
    Listener *OnSelect;
};

/*---------------------------------------------------------------------------*/

static const real32_t i_ICON_HEIGHT = 16;
static const real32_t i_TAB_VPADDING = 8;

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSTabs *tabs = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSTabs);
    cassert_no_null(tabs);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(tabs, OSControl)) == TRUE)
            break;
        return 0;

    default:
        break;
    }

    return CallWindowProc(tabs->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

OSTabs *ostabs_create(const uint32_t flags)
{
    OSTabs *tabs = NULL;
    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | TCS_HOTTRACK | TCS_SINGLELINE | TCS_TABS /* TCS_FLATBUTTONS*/;

    switch (tabs_get_pos(flags))
    {
    case ekTABS_LEFT:
        dwStyle |= TCS_VERTICAL;
        break;
    case ekTABS_TOP:
        break;
    case ekTABS_RIGHT:
        dwStyle |= TCS_VERTICAL | TCS_RIGHT;
        break;
    case ekTABS_BOTTOM:
        dwStyle |= TCS_BOTTOM;
        break;
    default:
        cassert_default(tabs_get_pos(flags));
    }

    tabs = heap_new0(OSTabs);
    tabs->flags = flags;
    tabs->control.type = ekGUI_TYPE_TABLIST;
    _oscontrol_init(cast(tabs, OSControl), PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY | CBES_EX_NOSIZELIMIT), dwStyle, WC_TABCONTROL, 10, 10, i_WndProc, kDEFAULT_PARENT_WINDOW);
    tabs->font = _osgui_create_default_font();
    tabs->control.tooltip_hwnd1 = tabs->control.hwnd;
    tabs->image_list = _osimglist_create((uint32_t)i_ICON_HEIGHT);
    _oscontrol_set_font(cast(tabs, OSControl), tabs->font);
    return tabs;
}

/*---------------------------------------------------------------------------*/

void ostabs_destroy(OSTabs **tabs)
{
    cassert_no_null(tabs);
    cassert_no_null(*tabs);
    font_destroy(&(*tabs)->font);
    listener_destroy(&(*tabs)->OnSelect);
    _osimglist_destroy(&(*tabs)->image_list);
    _oscontrol_destroy(&(*tabs)->control);
    heap_delete(tabs, OSTabs);
}

/*---------------------------------------------------------------------------*/

void ostabs_OnSelect(OSTabs *tabs, Listener *listener)
{
    cassert_no_null(tabs);
    listener_update(&tabs->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void ostabs_tooltip(OSTabs *tabs, const char_t *text)
{
    _oscontrol_tooltip(cast(tabs, OSControl), text);
}

/*---------------------------------------------------------------------------*/

void ostabs_font(OSTabs *tabs, const Font *font)
{
    cassert_no_null(tabs);
    _oscontrol_update_font(cast(tabs, OSControl), &tabs->font, font);
}

/*---------------------------------------------------------------------------*/

void ostabs_elem(OSTabs *tabs, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(tabs);
    if (op != ekCTRL_OP_DEL)
    {
        WString str;
        const WCHAR *wtext = _osgui_wstr_init(text, &str);
        UINT msg = 0;
        TCITEM tci = {0};
        int tpos = -1;

        switch (op)
        {
        case ekCTRL_OP_ADD:
            tpos = (int)SendMessage(tabs->control.hwnd, TCM_GETITEMCOUNT, 0, 0);
            msg = TCM_INSERTITEM;
            break;
        case ekCTRL_OP_INS:
            tpos = (int)index;
            msg = TCM_INSERTITEM;
            break;
        case ekCTRL_OP_SET:
            tpos = (int)index;
            msg = TCM_SETITEM;
            break;
        case ekCTRL_OP_DEL:
        default:
            cassert_default(op);
        }

        tci.mask = TCIF_IMAGE | TCIF_TEXT;
        tci.pszText = (LPWSTR)wtext;
        tci.iImage = _osimglist_index(tabs->image_list, tabs->control.hwnd, ekGUI_TYPE_TABLIST, image);
        SendMessage(tabs->control.hwnd, msg, (WPARAM)tpos, (LPARAM)&tci);
        _osgui_wstr_remove(&str);
    }
    else
    {
        LRESULT res = SendMessage(tabs->control.hwnd, TCM_DELETEITEM, (WPARAM)index, (LPARAM)0);
        cassert_unref(res != CB_ERR, res);
    }

    InvalidateRect(tabs->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void ostabs_selected(OSTabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    cassert(index < (uint32_t)SendMessage(tabs->control.hwnd, TCM_GETITEMCOUNT, 0, 0));
    SendMessage(tabs->control.hwnd, TCM_SETCURSEL, (WPARAM)index, 0);
}

/*---------------------------------------------------------------------------*/

uint32_t ostabs_get_selected(const OSTabs *tabs)
{
    cassert_no_null(tabs);
    return (uint32_t)SendMessage(tabs->control.hwnd, TCM_GETCURSEL, 0, 0);
}

/*---------------------------------------------------------------------------*/

void ostabs_bounds(const OSTabs *tabs, const real32_t length, real32_t *width, real32_t *height)
{
    real32_t twidth = 0;
    real32_t theight = 0;
    cassert_no_null(tabs);
    cassert_no_null(width);
    cassert_no_null(height);

    font_extents(tabs->font, "OO", -1.f, &twidth, &theight);

    if (theight < i_ICON_HEIGHT)
        theight = i_ICON_HEIGHT;

    theight += i_TAB_VPADDING;

    switch (tabs_get_pos(tabs->flags))
    {
    case ekTABS_LEFT:
    case ekTABS_RIGHT:
        *width = theight;
        *height = length;
        break;
    case ekTABS_TOP:
    case ekTABS_BOTTOM:
        *width = length;
        *height = theight;
        break;
    default:
        cassert_default(tabs_get_pos(tabs->flags));
    }
}

/*---------------------------------------------------------------------------*/

void ostabs_attach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(tabs, OSControl));
}

/*---------------------------------------------------------------------------*/

void ostabs_detach(OSTabs *tabs, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(tabs, OSControl));
}

/*---------------------------------------------------------------------------*/

void ostabs_visible(OSTabs *tabs, const bool_t visible)
{
    _oscontrol_set_visible(cast(tabs, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void ostabs_enabled(OSTabs *tabs, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(tabs, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void ostabs_size(const OSTabs *tabs, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(tabs, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void ostabs_origin(const OSTabs *tabs, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(tabs, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void ostabs_frame(OSTabs *tabs, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(tabs, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _ostabs_OnNotification(OSTabs *tabs, const NMHDR *nmhdr)
{
    cassert_no_null(tabs);
    cassert_no_null(nmhdr);
    if (nmhdr->code == TCN_SELCHANGE)
    {
        EvButton params;
        params.state = ekGUI_ON;
        params.index = (uint32_t)SendMessage(tabs->control.hwnd, TCM_GETCURSEL, (WPARAM)0, (LPARAM)0);
        cassert(params.index < (uint32_t)SendMessage(tabs->control.hwnd, TCM_GETITEMCOUNT, 0, 0));
        params.text = NULL;
        listener_event(tabs->OnSelect, ekGUI_EVENT_TABS, tabs, &params, NULL, OSTabs, EvButton, void);
    }
}
