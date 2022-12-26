/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osedit.c
 *
 */

/* Operating System edit box */

#include "osedit.h"
#include "osedit.inl"
#include "osgui.inl"
#include "osgui_win.inl"
#include "oscontrol.inl"
#include "ospanel.inl"
#include "oswindow.inl"
#include "cassert.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "ptr.h"
#include "strings.h"
#include "unicode.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _osedit_t 
{
    OSControl control;
    uint32_t flags;
    Font *font;
    bool_t launch_event;
    COLORREF color;
    COLORREF bgcolor;
    HBRUSH bgbrush;
    RECT border;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

static void i_launch_change_event(const OSEdit *edit)
{
    cassert_no_null(edit);
    if (edit->OnChange != NULL)
    {
        char_t *edit_text;
        uint32_t tsize;
        EvText params;
        edit_text = _oscontrol_get_text((const OSControl*)edit, &tsize);
        params.text = (const char_t*)edit_text;
        listener_event(edit->OnChange, ekGUI_EVENT_TXTCHANGE, edit, &params, NULL, OSEdit, EvText, void);
        heap_free((byte_t**)&edit_text, tsize, "OSControlGetText");
    }
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSEdit *edit = (OSEdit*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    LRESULT res;
    cassert_no_null(edit);  

    switch (uMsg) {
	case WM_ERASEBKGND:
        return 1;

    case WM_NCCALCSIZE:
        return _osgui_nccalcsize(hwnd, wParam, lParam, TRUE, &edit->border);

    case WM_NCPAINT:
        return _osgui_ncpaint(hwnd, &edit->border);

	case WM_PAINT:
        if (_oswindow_in_resizing(hwnd) == TRUE)
            return 0;
        break;

    case WM_SETFOCUS:
        if (edit->OnFocus != NULL)
        {
            bool_t params = TRUE;
            listener_event(edit->OnFocus, ekGUI_EVENT_FOCUS, edit, &params, NULL, OSEdit, bool_t, void);
        }

        if (BIT_TEST(edit->flags, ekEDIT_AUTOSEL) == TRUE)
            SendMessage(hwnd,EM_SETSEL, 0, -1);
        break;

    case WM_KILLFOCUS:
        if (edit->launch_event == TRUE && IsWindowEnabled(edit->control.hwnd))
            i_launch_change_event(edit);

        if (edit->OnFocus != NULL)
        {
            bool_t params = FALSE;
            listener_event(edit->OnFocus, ekGUI_EVENT_FOCUS, edit, &params, NULL, OSEdit, bool_t, void);
        }
        break;
    }

    res = CallWindowProc(edit->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);

    if (uMsg == WM_LBUTTONDOWN)
    {
        if (BIT_TEST(edit->flags, ekEDIT_AUTOSEL) == TRUE)
            SendMessage(hwnd,EM_SETSEL, 0, -1);
    }

    return res;
}

/*---------------------------------------------------------------------------*/

static DWORD i_flags(const edit_flag_t flags)
{
    if (edit_get_type(flags) == ekEDIT_MULTI)
        return ES_MULTILINE | ES_AUTOVSCROLL;
    else
        return ES_AUTOHSCROLL;
}

/*---------------------------------------------------------------------------*/

OSEdit *osedit_create(const uint32_t flags)
{
    OSEdit *edit = heap_new0(OSEdit);
    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | _oscontrol_halign(ekLEFT) | i_flags(flags);
    edit->control.type = ekGUI_TYPE_EDITBOX;
    edit->flags = flags;
    _oscontrol_init((OSControl*)edit, PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY /*| WS_EX_CLIENTEDGE*/), dwStyle, L"edit", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    edit->font = _osgui_create_default_font();
    edit->launch_event = TRUE;
    _oscontrol_set_font((OSControl*)edit, edit->font);
    return edit;
}

/*---------------------------------------------------------------------------*/

void osedit_destroy(OSEdit **edit)
{
    cassert_no_null(edit);
    cassert_no_null(*edit);
    font_destroy(&(*edit)->font);
    listener_destroy(&(*edit)->OnFilter);
    listener_destroy(&(*edit)->OnChange);
    listener_destroy(&(*edit)->OnFocus);
    if ((*edit)->bgbrush != NULL)
        DeleteObject((*edit)->bgbrush);
    _oscontrol_destroy(&(*edit)->control);
    heap_delete(edit, OSEdit);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFilter(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnChange(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_OnFocus(OSEdit *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void osedit_text(OSEdit *edit, const char_t *text)
{
	cassert_no_null(edit);
    edit->launch_event = FALSE;
    _oscontrol_set_text((OSControl*)edit, text);
    edit->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void osedit_font(OSEdit *edit, const Font *font)
{
    cassert_no_null(edit);
    _oscontrol_update_font((OSControl*)edit, &edit->font, font);
}

/*---------------------------------------------------------------------------*/

void osedit_tooltip(OSEdit *edit, const char_t *text)
{
    _oscontrol_set_tooltip((OSControl*)edit, text);
}

/*---------------------------------------------------------------------------*/

void osedit_align(OSEdit *edit, const align_t align)
{
    DWORD dwStyle = 0;
    cassert_no_null(edit);
    dwStyle = (DWORD)GetWindowLongPtr(edit->control.hwnd, GWL_STYLE);
    dwStyle &= ~SS_LEFT;
    dwStyle &= ~SS_CENTER;
    dwStyle &= ~SS_RIGHT;
    dwStyle |= _oscontrol_halign(align);
    SetWindowLongPtr(edit->control.hwnd, GWL_STYLE, dwStyle);
}

/*---------------------------------------------------------------------------*/

void osedit_passmode(OSEdit *edit, const bool_t passmode)
{
    cassert_no_null(edit);
    if (passmode == TRUE)
    {
        wchar_t pchar = L'\x2022';
        SendMessage(edit->control.hwnd, EM_SETPASSWORDCHAR, (WPARAM)pchar, (LPARAM)0);
    }
    else
    {
        SendMessage(edit->control.hwnd, EM_SETPASSWORDCHAR, (WPARAM)0, (LPARAM)0);
    }
}

/*---------------------------------------------------------------------------*/

void osedit_editable(OSEdit *edit, const bool_t is_editable)
{
    LRESULT res = 0;
    cassert_no_null(edit);
    res = SendMessage(edit->control.hwnd, EM_SETREADONLY, (WPARAM)!is_editable, (LPARAM)0);
    cassert_unref(res != 0, res);
}

/*---------------------------------------------------------------------------*/

void osedit_autoselect(OSEdit *edit, const bool_t autoselect)
{
    cassert_no_null(edit);
    if (autoselect == TRUE)
        BIT_SET(edit->flags, ekEDIT_AUTOSEL);
    else
        BIT_CLEAR(edit->flags, ekEDIT_AUTOSEL);
}

/*---------------------------------------------------------------------------*/

void osedit_color(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->color = _oscontrol_colorref(color);
}

/*---------------------------------------------------------------------------*/

void osedit_bgcolor(OSEdit *edit, const color_t color)
{
    cassert_no_null(edit);
    _oscontrol_update_brush(color, &edit->bgbrush, &edit->bgcolor);
}

/*---------------------------------------------------------------------------*/

void osedit_bounds(const OSEdit *edit, const real32_t refwidth, const uint32_t lines, real32_t *width, real32_t *height)
{
    register uint32_t padding = 0;
    cassert_no_null(edit);
    cassert_no_null(width);
    cassert_no_null(height);
    cassert(lines > 0);

    _oscontrol_text_bounds((OSControl*)edit, "O", edit->font, -1.f, width, height);
    padding = (uint32_t)((.3f * *height) + .5f);
    if (padding % 2 == 1)
        padding += 1;

    if (lines > 1)
    {
        register uint32_t i;
        char_t text[256] = "";
        cassert(edit_get_type(edit->flags) == ekEDIT_MULTI);
        cassert(lines < 100);
        for (i = 0; i < lines - 1; ++i)
            str_cat_c(text, 256, "O\n");
        str_cat_c(text, 256, "O");
        _oscontrol_text_bounds((OSControl*)edit, text, edit->font, -1.f, width, height);
    }

    *width = refwidth;
    *height += (real32_t)padding;
    cassert(*width == 100);
}

/*---------------------------------------------------------------------------*/

void osedit_attach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_attach_control(panel, (OSControl*)edit);
}

/*---------------------------------------------------------------------------*/

void osedit_detach(OSEdit *edit, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)edit);
}

/*---------------------------------------------------------------------------*/

void osedit_visible(OSEdit *edit, const bool_t visible)
{
    _oscontrol_set_visible((OSControl*)edit, visible);
}

/*---------------------------------------------------------------------------*/

void osedit_enabled(OSEdit *edit, const bool_t enabled)
{
    _oscontrol_set_enabled((OSControl*)edit, enabled);
}

/*---------------------------------------------------------------------------*/

void osedit_size(const OSEdit *edit, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)edit, width, height);
}

/*---------------------------------------------------------------------------*/

void osedit_origin(const OSEdit *edit, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)edit, x, y);
}

/*---------------------------------------------------------------------------*/

void osedit_frame(OSEdit *edit, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)edit, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _osedit_detach_and_destroy(OSEdit **edit, OSPanel *panel)
{
    cassert_no_null(edit);
    osedit_detach(*edit, panel);
    osedit_destroy(edit);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_get_cursor_pos(HWND hwnd)
{
    DWORD start;
    SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM)NULL);
    return (uint32_t)start;
}

/*---------------------------------------------------------------------------*/

static void i_set_cursor_pos(HWND hwnd, const uint32_t pos)
{
    SendMessage(hwnd, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
}

/*---------------------------------------------------------------------------*/

void _osedit_command(OSEdit *edit, WPARAM wParam)
{
    cassert_no_null(edit);
    /*EN_UPDATE Cambiar*/
    if (HIWORD(wParam) == EN_CHANGE)
    {
        if (edit->launch_event == TRUE && IsWindowEnabled(edit->control.hwnd) && edit->OnFilter != NULL)
        {
            char_t *edit_text;
            uint32_t tsize;
            EvText params;
            EvTextFilter result;
            edit_text = _oscontrol_get_text((const OSControl*)edit, &tsize);
            params.text = (const char_t*)edit_text;
            params.cpos = i_get_cursor_pos(edit->control.hwnd);
            result.apply = FALSE;
            result.text[0] = '\0';
            result.cpos = UINT32_MAX;
            listener_event(edit->OnFilter, ekGUI_EVENT_TXTFILTER, edit, &params, &result, OSEdit, EvText, EvTextFilter);
            heap_free((byte_t**)&edit_text, tsize, "OSControlGetText");

            if (result.apply == TRUE)
            {
                bool_t prev = edit->launch_event;
                edit->launch_event = FALSE;
                _oscontrol_set_text(&edit->control, result.text);
                edit->launch_event = prev;
            }

            if (result.cpos != UINT32_MAX)
                i_set_cursor_pos(edit->control.hwnd, result.cpos);
            else
                i_set_cursor_pos(edit->control.hwnd, params.cpos);
        }
    }
}

/*---------------------------------------------------------------------------*/

COLORREF _osedit_color(const OSEdit *edit)
{
    cassert_no_null(edit);
    return edit->color;
}

/*---------------------------------------------------------------------------*/

HBRUSH _osedit_background_color(const OSEdit *edit, COLORREF *color)
{
    cassert_no_null(edit);
    if (edit->bgbrush != NULL)
    {
        *color = edit->bgcolor & 0x00FFFFFF;
        return edit->bgbrush;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

void _osedit_kill_focus(const OSEdit *edit)
{
    i_launch_change_event(edit);
}
