/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscombo.c
 *
 */

/* Operating System native combo box */

#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "oscombo_win.inl"
#include "ospanel_win.inl"
#include "oswindow_win.inl"
#include "osimglist.inl"
#include "../oscombo.h"
#include "../oscombo.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _oscombo_t
{
    OSControl control;
    uint32_t flags;
    Font *font;
    bool_t launch_event;
    bool_t with_initial_size;
    uint32_t list_num_elems;
    HWND combo_hwnd;
    HWND edit_hwnd;
    WNDPROC def_combo_proc;
    WNDPROC def_edit_proc;
    OSImgList *image_list;
    DWORD cursor_st;
    DWORD cursor_ed;
    COLORREF color;
    COLORREF bgcolor;
    HBRUSH bgbrush;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
    Listener *OnSelect;
};

/*---------------------------------------------------------------------------*/

static HBRUSH i_background_color(const OSCombo *combo, COLORREF *color)
{
    cassert_no_null(combo);
    if (combo->bgbrush != NULL)
    {
        *color = combo->bgcolor & 0x00FFFFFF;
        return combo->bgbrush;
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSCombo *combo = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSCombo);
    cassert_no_null(combo);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(combo, OSControl)) == TRUE)
            break;
        return 0;

    case WM_CTLCOLOREDIT:
    {
        HBRUSH defbrush = (HBRUSH)CallWindowProc(combo->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
        COLORREF bgcolor = UINT32_MAX;
        HBRUSH brush = i_background_color(combo, &bgcolor);
        return (LRESULT)_oscontrol_ctl_color_edit((HDC)wParam, combo->color, bgcolor, brush, defbrush);
    }

    default:
        break;
    }

    return CallWindowProc(combo->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_ComboWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSCombo *combo = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSCombo);
    cassert_no_null(combo);

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(combo, OSControl)) == TRUE)
            break;
        return 0;
    default:
        break;
    }

    return combo->def_combo_proc(hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_get_cursor_pos(HWND hwnd)
{
    DWORD start;
    SendMessage(hwnd, CB_GETEDITSEL, (WPARAM)&start, (LPARAM)NULL);
    return (uint32_t)start;
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_EditWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSCombo *combo = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSCombo);
    DWORD cursor_st = 0, cursor_ed = 0;
    LRESULT res = 0;
    cassert_no_null(combo);

    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(combo, OSControl)) == TRUE)
            break;
        return 0;

    case WM_SETFOCUS:
        SendMessage(combo->control.hwnd, CB_GETEDITSEL, (WPARAM)&cursor_st, (LPARAM)&cursor_ed);
        break;

    default:
        break;
    }

    res = CallWindowProc(combo->def_edit_proc, hwnd, uMsg, wParam, lParam);

    if (uMsg == WM_SETFOCUS)
        SendMessage(combo->edit_hwnd, EM_SETSEL, cursor_st, cursor_ed);

    return res;
}

/*---------------------------------------------------------------------------*/

OSCombo *oscombo_create(const uint32_t flags)
{
    OSCombo *combo = heap_new0(OSCombo);
    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | CBS_DROPDOWN | CBS_AUTOHSCROLL;
    DWORD dwExStyle = WS_EX_NOPARENTNOTIFY | CBES_EX_NOSIZELIMIT;
    combo->control.type = ekGUI_TYPE_COMBOBOX;
    combo->flags = flags;
    combo->with_initial_size = FALSE;
    combo->list_num_elems = 5;
    _oscontrol_init(cast(combo, OSControl), dwExStyle, dwStyle, WC_COMBOBOXEX, 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    combo->font = _osgui_create_default_font();
    combo->launch_event = TRUE;
    combo->combo_hwnd = (HWND)SendMessage(combo->control.hwnd, CBEM_GETCOMBOCONTROL, (WPARAM)0, (LPARAM)0);
    combo->edit_hwnd = (HWND)SendMessage(combo->control.hwnd, CBEM_GETEDITCONTROL, (WPARAM)0, (LPARAM)0);
    combo->def_combo_proc = (WNDPROC)SetWindowLongPtr(combo->combo_hwnd, GWLP_WNDPROC, (LONG_PTR)i_ComboWndProc);
    combo->def_edit_proc = (WNDPROC)SetWindowLongPtr(combo->edit_hwnd, GWLP_WNDPROC, (LONG_PTR)i_EditWndProc);
    combo->control.tooltip_hwnd1 = combo->combo_hwnd;
    combo->control.tooltip_hwnd2 = combo->edit_hwnd;
    SetWindowLongPtr(combo->combo_hwnd, GWLP_USERDATA, (LONG_PTR)combo);
    SetWindowLongPtr(combo->edit_hwnd, GWLP_USERDATA, (LONG_PTR)combo);
    combo->image_list = _osimglist_create(16);
    _oscontrol_set_font(cast(combo, OSControl), combo->font);
    return combo;
}

/*---------------------------------------------------------------------------*/

void oscombo_destroy(OSCombo **combo)
{
    cassert_no_null(combo);
    cassert_no_null(*combo);
    font_destroy(&(*combo)->font);
    listener_destroy(&(*combo)->OnFilter);
    listener_destroy(&(*combo)->OnChange);
    listener_destroy(&(*combo)->OnFocus);
    listener_destroy(&(*combo)->OnSelect);
    _oscontrol_destroy_brush(&(*combo)->bgbrush);
    _osimglist_destroy(&(*combo)->image_list);
    _oscontrol_destroy(&(*combo)->control);
    heap_delete(combo, OSCombo);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFilter(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&combo->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnChange(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&combo->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnFocus(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&combo->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_OnSelect(OSCombo *combo, Listener *listener)
{
    cassert_no_null(combo);
    listener_update(&combo->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

void oscombo_text(OSCombo *combo, const char_t *text)
{
    cassert_no_null(combo);
    cassert(combo->launch_event == TRUE);
    combo->launch_event = FALSE;
    _oscontrol_set_text(cast(combo, OSControl), text);
    combo->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

void oscombo_tooltip(OSCombo *combo, const char_t *text)
{
    _oscontrol_tooltip(cast(combo, OSControl), text);
}

/*---------------------------------------------------------------------------*/

void oscombo_font(OSCombo *combo, const Font *font)
{
    cassert_no_null(combo);
    _oscontrol_update_font(cast(combo, OSControl), &combo->font, font);
}

/*---------------------------------------------------------------------------*/

void oscombo_align(OSCombo *combo, const align_t align)
{
    DWORD dwStyle = 0;
    cassert_no_null(combo);
    dwStyle = (DWORD)GetWindowLongPtr(combo->edit_hwnd, GWL_STYLE);
    dwStyle &= ~(DWORD)ES_LEFT;
    dwStyle &= ~(DWORD)ES_CENTER;
    dwStyle &= ~(DWORD)ES_RIGHT;
    dwStyle |= _oscontrol_es_halign(align);
    SetWindowLongPtr(combo->edit_hwnd, GWL_STYLE, dwStyle);
}

/*---------------------------------------------------------------------------*/

void oscombo_passmode(OSCombo *combo, const bool_t passmode)
{
    cassert_no_null(combo);
    if (passmode == TRUE)
    {
        wchar_t pchar = L'\x2022';
        SendMessage(combo->edit_hwnd, EM_SETPASSWORDCHAR, (WPARAM)pchar, (LPARAM)0);
    }
    else
    {
        SendMessage(combo->edit_hwnd, EM_SETPASSWORDCHAR, (WPARAM)0, (LPARAM)0);
    }
}

/*---------------------------------------------------------------------------*/

void oscombo_editable(OSCombo *combo, const bool_t is_editable)
{
    LRESULT res = 0;
    cassert_no_null(combo);
    res = SendMessage(combo->edit_hwnd, EM_SETREADONLY, (WPARAM)!is_editable, (LPARAM)0);
    cassert_unref(res != 0, res);
}

/*---------------------------------------------------------------------------*/

void oscombo_autoselect(OSCombo *combo, const bool_t autoselect)
{
    cassert_no_null(combo);
    if (autoselect == TRUE)
        BIT_SET(combo->flags, ekCOMBO_AUTOSEL);
    else
        BIT_CLEAR(combo->flags, ekCOMBO_AUTOSEL);
}

/*---------------------------------------------------------------------------*/

void oscombo_select(OSCombo *combo, const int32_t start, const int32_t end)
{
    int32_t platform_st, platform_ed;
    cassert_no_null(combo);
    _osgui_select_text(start, end, &platform_st, &platform_ed);
    SendMessage(combo->edit_hwnd, EM_SETSEL, (WPARAM)platform_st, (LPARAM)platform_ed);
}

/*---------------------------------------------------------------------------*/

void oscombo_color(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    combo->color = _oscontrol_colorref(color);
    InvalidateRect(combo->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void oscombo_bgcolor(OSCombo *combo, const color_t color)
{
    cassert_no_null(combo);
    _oscontrol_update_brush(color, &combo->bgbrush, &combo->bgcolor);
}

/*---------------------------------------------------------------------------*/

static int i_img_index(HWND hwnd, OSImgList *imglist, const Image *image)
{
    int index = 0;

    if (image != NULL)
    {
        /* Image exits in list --> rehuse */
        index = _osimglist_find(imglist, image);

        /* Check for unused image --> replace */
        if (index == -1)
        {
            uint32_t num_images = _osimglist_num_elems(imglist);
            if (num_images > 0)
            {
                uint32_t i = 0, num_elems = (uint32_t)SendMessage(hwnd, CB_GETCOUNT, 0, 0);
                bool_t *exists = heap_new_n0(num_images, bool_t);

                for (i = 0; i < num_elems; ++i)
                {
                    COMBOBOXEXITEM cbbi = {0};
                    LRESULT res;
                    cbbi.iItem = (INT_PTR)i;
                    cbbi.mask = CBEIF_IMAGE;
                    res = SendMessage(hwnd, CBEM_GETITEM, (WPARAM)0, (LPARAM)&cbbi);
                    cassert_unref(res != 0, res);
                    exists[cbbi.iImage] = TRUE;
                }

                for (i = 1; i < num_images && index == -1; ++i)
                {
                    if (exists[i] == FALSE)
                    {
                        index = (int)i;
                        _osimglist_replace(imglist, index, image);
                    }
                }

                heap_delete_n(&exists, num_images, bool_t);
            }
        }

        /* Add a new image */
        if (index == -1)
        {
            uint8_t result = 0;
            index = _osimglist_add(imglist, image, &result);
            if (result == HIMAGELIST_CREATED)
            {
                HIMAGELIST hlist = _osimglist_hlist(imglist);
                HIMAGELIST previous = (HIMAGELIST)SendMessage(hwnd, CBEM_SETIMAGELIST, 0, (LPARAM)hlist);
                cassert_unref(previous == NULL, previous);
            }
        }
    }

    return index;
}

/*---------------------------------------------------------------------------*/

void oscombo_elem(OSCombo *combo, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(combo);
    _oscombo_elem(combo->control.hwnd, combo->image_list, op, index, text, image);

    /* If the editBox has not previous text, take the first element text */
    if (SendMessage(combo->edit_hwnd, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0) == 0)
    {
        if (SendMessage(combo->control.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) == -1)
            SendMessage(combo->control.hwnd, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    }

    InvalidateRect(combo->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void oscombo_list_height(OSCombo *combo, const uint32_t num_elems)
{
    cassert_no_null(combo);
    combo->list_num_elems = num_elems;
    if (combo->with_initial_size == TRUE)
        _oscombo_set_list_height(combo->control.hwnd, combo->combo_hwnd, _osimglist_height(combo->image_list), combo->list_num_elems);
}

/*---------------------------------------------------------------------------*/

void oscombo_selected(OSCombo *combo, const uint32_t index)
{
    cassert_no_null(combo);
    cassert(combo->launch_event == TRUE);
    combo->launch_event = FALSE;
    if (index != UINT32_MAX)
    {
        LRESULT ret = SendMessage(combo->control.hwnd, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
        cassert_unref(ret == (LRESULT)index, ret);
    }
    else
    {
        LRESULT ret = SendMessage(combo->control.hwnd, CB_SETCURSEL, (WPARAM)-1, (LPARAM)0);
        cassert_unref(ret == (LRESULT)CB_ERR, ret);
    }
    combo->launch_event = TRUE;
}

/*---------------------------------------------------------------------------*/

uint32_t oscombo_get_selected(const OSCombo *combo)
{
    LRESULT res = 0;
    cassert_no_null(combo);
    res = SendMessage(combo->control.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    return (res != CB_ERR) ? (uint32_t)res : UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void oscombo_bounds(const OSCombo *combo, const real32_t refwidth, real32_t *width, real32_t *height)
{
    long button_height = 0;
    cassert_no_null(combo);
    cassert_no_null(width);
    cassert_no_null(height);
    button_height = (14 * HIWORD(GetDialogBaseUnits())) / 8;
    *width = refwidth;
    *height = (real32_t)(button_height - 4);
}

/*---------------------------------------------------------------------------*/

void oscombo_clipboard(OSCombo *combo, const clipboard_t clipboard)
{
    cassert_no_null(combo);
    _oscontrol_clipboard(combo->edit_hwnd, clipboard);
}

/*---------------------------------------------------------------------------*/

void oscombo_attach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(combo, OSControl));
}

/*---------------------------------------------------------------------------*/

void oscombo_detach(OSCombo *combo, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(combo, OSControl));
}

/*---------------------------------------------------------------------------*/

void oscombo_visible(OSCombo *combo, const bool_t visible)
{
    _oscontrol_set_visible(cast(combo, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void oscombo_enabled(OSCombo *combo, const bool_t enabled)
{
    cassert(GetFocus() != combo->control.hwnd);
    _oscontrol_set_enabled(cast(combo, OSControl), enabled);
}

/*---------------------------------------------------------------------------*/

void oscombo_size(const OSCombo *combo, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(combo, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void oscombo_origin(const OSCombo *combo, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(combo, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void oscombo_frame(OSCombo *combo, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(combo, OSControl), x, y, width, height);
    _oscombo_set_list_height(combo->control.hwnd, combo->combo_hwnd, _osimglist_height(combo->image_list), combo->list_num_elems);
    combo->with_initial_size = TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_set_cursor_pos(HWND hwnd, const uint32_t pos)
{
    /* CB_SETEDITSEL is not working fine */
    SendMessage(hwnd, EM_SETSEL, (WPARAM)pos, (LPARAM)pos);
}

/*---------------------------------------------------------------------------*/

void _oscombo_command(OSCombo *combo, WPARAM wParam)
{
    cassert_no_null(combo);
    if (HIWORD(wParam) == CBN_EDITCHANGE)
    {
        if (combo->launch_event == TRUE && IsWindowEnabled(combo->control.hwnd) && combo->OnFilter != NULL)
        {
            char_t *combo_text;
            uint32_t tsize;
            EvText params;
            EvTextFilter result;
            combo_text = _oscontrol_get_text(cast_const(combo, OSControl), &tsize, NULL);
            params.text = cast_const(combo_text, char_t);
            params.cpos = i_get_cursor_pos(combo->control.hwnd);
            params.len = INT32_MAX;
            result.apply = FALSE;
            result.text[0] = '\0';
            result.cpos = UINT32_MAX;
            listener_event(combo->OnFilter, ekGUI_EVENT_TXTFILTER, combo, &params, &result, OSCombo, EvText, EvTextFilter);
            heap_free(dcast(&combo_text, byte_t), tsize, "OSControlGetText");

            if (result.apply == TRUE)
            {
                bool_t prev = combo->launch_event;
                combo->launch_event = FALSE;
                _oscontrol_set_text(&combo->control, result.text);
                combo->launch_event = prev;
            }

            if (result.cpos != UINT32_MAX)
                i_set_cursor_pos(combo->edit_hwnd, result.cpos);
            else
                i_set_cursor_pos(combo->edit_hwnd, params.cpos);
        }
    }
    else if (HIWORD(wParam) == CBN_SELCHANGE)
    {
        if (IsWindowEnabled(combo->control.hwnd) && combo->OnSelect != NULL)
        {
            EvButton params;
            params.state = ekGUI_ON;
            params.index = (uint16_t)SendMessage(combo->control.hwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
            params.text = NULL;
            listener_event(combo->OnSelect, ekGUI_EVENT_BUTTON, combo, &params, NULL, OSCombo, EvButton, void);
        }
    }
}

/*---------------------------------------------------------------------------*/

HWND _oscombo_focus_widget(OSCombo *combo)
{
    cassert_no_null(combo);
    return combo->edit_hwnd;
}

/*---------------------------------------------------------------------------*/

void _oscombo_elem(HWND hwnd, OSImgList *imglist, const ctrl_op_t op, const uint32_t index, const char_t *text, const Image *image)
{
    if (op != ekCTRL_OP_DEL)
    {
        WString str;
        const WCHAR *wtext = _osgui_wstr_init(text, &str);
        UINT msg = 0;
        COMBOBOXEXITEM cbbi = {0};

        switch (op)
        {
        case ekCTRL_OP_ADD:
            cbbi.iItem = -1;
            msg = CBEM_INSERTITEM;
            break;
        case ekCTRL_OP_INS:
            cbbi.iItem = (INT_PTR)index;
            msg = CBEM_INSERTITEM;
            break;
        case ekCTRL_OP_SET:
            cbbi.iItem = (INT_PTR)index;
            msg = CBEM_SETITEM;
            break;
        case ekCTRL_OP_DEL:
        default:
            cassert_default(op);
        }

        cbbi.mask = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
        cbbi.pszText = (LPWSTR)wtext;
        cbbi.iImage = i_img_index(hwnd, imglist, image);
        cbbi.iSelectedImage = cbbi.iImage;
        SendMessage(hwnd, msg, (WPARAM)0, (LPARAM)&cbbi);
        _osgui_wstr_remove(&str);
    }
    else
    {
        LRESULT res = SendMessage(hwnd, CBEM_DELETEITEM, (WPARAM)index, (LPARAM)0);
        cassert_unref(res != CB_ERR, res);
    }
}

/*---------------------------------------------------------------------------*/

void _oscombo_set_list_height(HWND hwnd, HWND combo_hwnd, const uint32_t image_height, uint32_t num_elems)
{
    uint32_t height = (uint32_t)((14 * HIWORD(GetDialogBaseUnits())) / 8) - 4;
    uint32_t line_height = (uint32_t)SendMessage(hwnd, CB_GETITEMHEIGHT, (WPARAM)0, (LPARAM)0);
    RECT rect;
    GetClientRect(hwnd, &rect);

    if (image_height != UINT32_MAX)
    {
        if (line_height < image_height)
            line_height = image_height;
    }

    height += num_elems * line_height;
    height += 2;
    SetWindowPos(combo_hwnd, NULL, 0, 0, rect.right - rect.left, (int)height, SWP_NOMOVE | SWP_NOZORDER);
}

/*---------------------------------------------------------------------------*/

bool_t _oscombo_resign_focus(const OSCombo *combo)
{
    bool_t lost_focus = TRUE;
    if (combo->OnChange != NULL)
    {
        char_t *combo_text = NULL;
        uint32_t tsize = 0;
        uint32_t nchars = 0;
        EvText params;
        combo_text = _oscontrol_get_text(cast_const(combo, OSControl), &tsize, &nchars);
        params.text = cast_const(combo_text, char_t);
        params.cpos = i_get_cursor_pos(combo->control.hwnd);
        params.len = (int32_t)nchars;
        listener_event(combo->OnChange, ekGUI_EVENT_TXTCHANGE, combo, &params, &lost_focus, OSCombo, EvText, bool_t);
        heap_free(dcast(&combo_text, byte_t), tsize, "OSControlGetText");
    }

    return lost_focus;
}

/*---------------------------------------------------------------------------*/

void _oscombo_focus(OSCombo *combo, const bool_t focus)
{
    cassert_no_null(combo);
    if (combo->OnFocus != NULL)
    {
        bool_t params = focus;
        listener_event(combo->OnFocus, ekGUI_EVENT_FOCUS, combo, &params, NULL, OSCombo, bool_t, void);
    }

    if (focus == TRUE)
    {
        /* CB_SETEDITSEL is not working fine */
        if (BIT_TEST(combo->flags, ekCOMBO_AUTOSEL) == TRUE)
            SendMessage(combo->edit_hwnd, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
        else
            SendMessage(combo->edit_hwnd, EM_SETSEL, combo->cursor_st, combo->cursor_ed);
    }
    else
    {
        SendMessage(combo->control.hwnd, CB_GETEDITSEL, (WPARAM)&combo->cursor_st, (LPARAM)&combo->cursor_ed);
    }
}
