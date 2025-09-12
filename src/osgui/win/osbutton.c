/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.c
 *
 */

/* Operating System native button */

#include "osbutton_win.inl"
#include "osgui_win.inl"
#include "oscontrol_win.inl"
#include "ospanel_win.inl"
#include "oswindow_win.inl"
#include "osimg.inl"
#include "osstyleXP.inl"
#include "../osbutton.h"
#include "../osbutton.inl"
#include "../osgui.inl"
#include <draw2d/font.h>
#include <draw2d/image.h>
#include <core/event.h>
#include <core/heap.h>
#include <osbs/osbs.h>
#include <osbs/btime.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _osbutton_t
{
    OSControl control;
    uint32_t flags;
    bool_t is_default;
    bool_t can_focus;
    uint16_t id;
    vkey_t key;
    Font *font;
    Image *image;
    uint32_t hpadding;
    uint32_t vpadding;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

static HWND i_LAST_FOCUS = NULL;
static double i_LAST_FOCUS_TIME = 0.;
#define i_TIME_SEC(microseconds) ((real64_t)microseconds / 1000000.)

/*---------------------------------------------------------------------------*/

static void i_draw_flat_button(HWND hwnd, const Image *image)
{
    HDC hdc = NULL;
    PAINTSTRUCT ps;
    RECT rect;
    RECT border;
    BOOL withXP_style = FALSE;
    BOOL enabled;
    cassert_no_null(hwnd);
    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rect);
    border = rect;

    withXP_style = _osstyleXP_OpenThemeData(hwnd, L"TOOLBAR");
    enabled = IsWindowEnabled(hwnd);

    if (withXP_style == TRUE)
    {
        int state = 0;
        if (enabled == FALSE)
        {
            state = TS_DISABLED;
        }
        else if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        {
            state = TS_PRESSED;
        }
        else if (_osgui_hit_test(hwnd) == TRUE)
        {
            if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
                state = TS_PRESSED;
            else
                state = TS_HOT;
        }
        else
        {
            state = TS_NORMAL;
        }

        /* WindowsXP draws nothing in TS_NORMAL (doesn't erase background) */
        if (osbs_windows() > ekWIN_XP3 || state != TS_NORMAL)
        {
            _osstyleXP_DrawThemeBackground(hwnd, hdc, TP_BUTTON, state, TRUE, &rect, &border);
        }
        else
        {
            HBRUSH brush = GetSysColorBrush(COLOR_BTNFACE);
            FillRect(hdc, &rect, brush);
        }
    }
    else
    {
        UINT state = DFCS_BUTTONPUSH;
        if (enabled == FALSE)
        {
            state |= DFCS_INACTIVE;
        }
        else if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        {
            state |= DFCS_PUSHED;
        }
        else if (_osgui_hit_test(hwnd) == TRUE)
        {
            if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
                state |= DFCS_PUSHED;
            else
                state |= DFCS_HOT;
        }

        _osstyleXP_DrawNonThemedButtonBackground(hwnd, hdc, FALSE, state, &rect, &border);
    }

    if (image != NULL)
    {
        uint32_t width = image_width(image);
        uint32_t height = image_height(image);
        uint32_t offset_x = (uint32_t)(rect.right - rect.left - (LONG)width) / 2;
        uint32_t offset_y = (uint32_t)(rect.bottom - rect.top - (LONG)height) / 2;
        _osimg_draw(image, hdc, UINT32_MAX, (real32_t)offset_x, (real32_t)offset_y, (real32_t)width, (real32_t)height, !enabled);
    }

    if (withXP_style == TRUE)
        _osstyleXP_CloseThemeData();

    {
        BOOL ok = EndPaint(hwnd, &ps);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSButton *button = cast(GetWindowLongPtr(hwnd, GWLP_USERDATA), OSButton);
    cassert_no_null(button);

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
        if (button_get_type(button->flags) == ekBUTTON_FLAT || button_get_type(button->flags) == ekBUTTON_FLATGLE)
        {
            i_draw_flat_button(button->control.hwnd, button->image);
            if (GetFocus() == button->control.hwnd)
                _oscontrol_draw_focus(hwnd, 3, 3, 3, 3);
        }
        else
        {
            CallWindowProc(button->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
        }

        return 0;

    case WM_SETFOCUS:
        if (button->can_focus == FALSE)
            return 0;

        {
            if (button_get_type(button->flags) == ekBUTTON_RADIO)
            {
                uint64_t microseconds;
                microseconds = btime_now();
                i_LAST_FOCUS = button->control.hwnd;
                i_LAST_FOCUS_TIME = i_TIME_SEC(microseconds);
            }
            else
            {
                i_LAST_FOCUS = NULL;
            }
        }
        break;

    case WM_KILLFOCUS:
        /* GTNAP fix for double '_osbutton_command' call */
        if (button->is_default == TRUE)
            return 0;
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        if (_oswindow_mouse_down(cast(button, OSControl)) == TRUE)
            break;
        return 0;

    default:
        break;
    }

    return CallWindowProc(button->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static DWORD i_button_skin(const button_flag_t flags)
{
    switch (flags)
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_FLAT:
        return BS_PUSHBUTTON;
    case ekBUTTON_CHECK2:
    case ekBUTTON_FLATGLE:
        return BS_AUTOCHECKBOX;
    case ekBUTTON_CHECK3:
        return BS_AUTO3STATE;
    case ekBUTTON_RADIO:
        return BS_RADIOBUTTON;

    case ekBUTTON_TYPE:
    default:
        cassert_default(flags);
    }

    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

static DWORD i_button_halign(const align_t text_align)
{
    switch (text_align)
    {
    case ekLEFT:
        return BS_LEFT;
    case ekCENTER:
    case ekJUSTIFY:
        return BS_CENTER;
    case ekRIGHT:
        return BS_RIGHT;
    default:
        cassert_default(text_align);
    }

    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

static DWORD i_style(const button_flag_t flags, const align_t align)
{
    DWORD osskin = 0;
    DWORD oshalign = 0;
    DWORD dwStyle = 0;
    osskin = i_button_skin(flags);
    oshalign = i_button_halign(align);
    dwStyle = WS_CHILD | WS_CLIPSIBLINGS | osskin | oshalign;
    return dwStyle;
}

/*---------------------------------------------------------------------------*/

OSButton *osbutton_create(const uint32_t flags)
{
    OSButton *button = heap_new0(OSButton);
    button->control.type = ekGUI_TYPE_BUTTON;
    button->flags = flags;
    button->is_default = FALSE;
    button->can_focus = TRUE;
    button->hpadding = UINT32_MAX;
    button->vpadding = UINT32_MAX;
    button->key = ENUM_MAX(vkey_t);
    button->id = _osgui_unique_child_id();

    _oscontrol_init(cast(button, OSControl), PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), i_style(flags, ekCENTER), L"button", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);

    if (_osbutton_text_allowed(flags) == TRUE)
    {
        button->font = _osgui_create_default_font();
        _oscontrol_set_font(cast(button, OSControl), button->font);
    }

    return button;
}

/*---------------------------------------------------------------------------*/

void osbutton_destroy(OSButton **button)
{
    cassert_no_null(button);
    cassert_no_null(*button);
    ptr_destopt(font_destroy, &(*button)->font, Font);
    ptr_destopt(image_destroy, &(*button)->image, Image);
    listener_destroy(&(*button)->OnClick);
    _oscontrol_destroy(&(*button)->control);
    heap_delete(button, OSButton);
}

/*---------------------------------------------------------------------------*/

void osbutton_OnClick(OSButton *button, Listener *listener)
{
    cassert_no_null(button);
    listener_update(&button->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void osbutton_text(OSButton *button, const char_t *text)
{
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    _oscontrol_set_text(cast(button, OSControl), text);

    /* Update key accelerator from text */
    {
        vkey_t key = _osgui_vkey_from_text(text);

        if (button->key == ENUM_MAX(vkey_t))
        {
            if (key != ENUM_MAX(vkey_t))
                _osgui_add_accelerator(FVIRTKEY | FALT, kVIRTUAL_KEY[key], button->id, button->control.hwnd);
        }
        else
        {
            if (key != ENUM_MAX(vkey_t))
                _osgui_change_accelerator(FVIRTKEY | FALT, kVIRTUAL_KEY[key], button->id);
            else
                _osgui_remove_accelerator(button->id);
        }

        button->key = key;
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_tooltip(OSButton *button, const char_t *text)
{
    _oscontrol_set_tooltip(cast(button, OSControl), text);
}

/*---------------------------------------------------------------------------*/

static void i_set_image(HWND hwnd, const Image *image)
{
    HBITMAP hbitmap = _osimg_hbitmap(image, 0);
    BOOL ok = FALSE;
    SendMessage(hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbitmap);
    ok = DeleteObject(hbitmap);
    cassert_unref(ok != 0, ok);
}

/*---------------------------------------------------------------------------*/

void osbutton_font(OSButton *button, const Font *font)
{
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    _oscontrol_update_font(cast(button, OSControl), &button->font, font);
}

/*---------------------------------------------------------------------------*/

void osbutton_align(OSButton *button, const align_t align)
{
    DWORD dwStyle = 0;
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    dwStyle = i_style(button->flags, align);
    SetWindowLongPtr(button->control.hwnd, GWL_STYLE, dwStyle);
}

/*---------------------------------------------------------------------------*/

void osbutton_image(OSButton *button, const Image *image)
{
    cassert_no_null(button);
    cassert(_osbutton_image_allowed(button->flags) == TRUE);
    ptr_destopt(image_destroy, &button->image, Image);
    if (button_get_type(button->flags) == ekBUTTON_PUSH)
    {
        if (image != NULL)
        {
            button->image = image_copy(image);
            i_set_image(button->control.hwnd, button->image);
        }
        else
        {
            SendMessage(button->control.hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)NULL);
        }
    }
    else
    {
        cassert(button_get_type(button->flags) == ekBUTTON_FLAT || button_get_type(button->flags) == ekBUTTON_FLATGLE);
        button->image = image_copy(image);
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_state(OSButton *button, const gui_state_t state)
{
    cassert_no_null(button);
    cassert(button->control.type == ekGUI_TYPE_BUTTON);
    if (button->flags == ekBUTTON_CHECK2 || button->flags == ekBUTTON_RADIO || button->flags == ekBUTTON_FLATGLE)
    {
        DWORD cstate = 0;
        switch (state)
        {
        case ekGUI_ON:
        case ekGUI_MIXED:
            cstate = BST_CHECKED;
            break;
        case ekGUI_OFF:
            cstate = BST_UNCHECKED;
            break;
        default:
            cassert_default(state);
        }

        SendMessage(button->control.hwnd, BM_SETCHECK, cstate, (LPARAM)0);
    }
    else if (button->flags == ekBUTTON_CHECK3)
    {
        DWORD cstate = 0;
        switch (state)
        {
        case ekGUI_ON:
            cstate = BST_CHECKED;
            break;
        case ekGUI_OFF:
            cstate = BST_UNCHECKED;
            break;
        case ekGUI_MIXED:
            cstate = BST_INDETERMINATE;
            break;
        default:
            cassert_default(state);
        }

        SendMessage(button->control.hwnd, BM_SETCHECK, cstate, (LPARAM)0);
    }
}

/*---------------------------------------------------------------------------*/

static gui_state_t i_get_state(const button_flag_t flags, HWND hwnd)
{
    switch (button_get_type(flags))
    {
    case ekBUTTON_PUSH:
    case ekBUTTON_FLAT:
        return ekGUI_ON;

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
    case ekBUTTON_FLATGLE:
    {
        LRESULT state = SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
        if (state == BST_CHECKED)
        {
            return ekGUI_ON;
        }
        else if (state == BST_UNCHECKED)
        {
            return ekGUI_OFF;
        }
        else
        {
            cassert(state == BST_INDETERMINATE);
            return ekGUI_MIXED;
        }
    }

    default:
        cassert_default(button_get_type(flags));
    }

    return ENUM_MAX(gui_state_t);
}

/*---------------------------------------------------------------------------*/

gui_state_t osbutton_get_state(const OSButton *button)
{
    cassert_no_null(button);
    return i_get_state(button->flags, button->control.hwnd);
}

/*---------------------------------------------------------------------------*/

void osbutton_hpadding(OSButton *button, const real32_t padding)
{
    cassert_no_null(button);
    if (padding >= 0)
        button->hpadding = (uint32_t)padding;
    else
        button->hpadding = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void osbutton_vpadding(OSButton *button, const real32_t padding)
{
    cassert_no_null(button);
    if (padding >= 0)
        button->vpadding = (uint32_t)padding;
    else
        button->vpadding = UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void osbutton_bounds(const OSButton *button, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height)
{
    cassert_no_null(button);
    cassert_no_null(width);
    cassert_no_null(height);

    switch (button_get_type(button->flags))
    {
    case ekBUTTON_PUSH:
    {
        real32_t fheight;
        real32_t woff, hoff;
        font_extents(button->font, text, -1.f, width, &fheight);
        font_extents(button->font, "O", -1.f, &woff, &hoff);

        if (button->hpadding == UINT32_MAX)
        {
            *width += 3 * woff;
        }
        else
        {
            if (button->hpadding < woff)
                *width += woff;
            else
                *width += (real32_t)button->hpadding;
        }

        /* Image width */
        if (refwidth > 0.f)
        {
            *width += refwidth;
            *width += (real32_t)(2 * GetSystemMetrics(SM_CXEDGE));
        }

        /* Image is higher than text */
        if (refheight > fheight)
            *height = refheight;
        else
            *height = fheight;

        if (button->vpadding == UINT32_MAX)
        {
            *height = bmath_ceilf(1.5f * *height) + 2.f;
        }
        else
        {
            uint32_t padding = button->vpadding;

            if (*height == fheight)
                padding += 2;

            if (padding % 2 == 1)
                padding += 1;

            *height += (real32_t)padding;
        }

        break;
    }

    case ekBUTTON_CHECK2:
    case ekBUTTON_CHECK3:
    case ekBUTTON_RADIO:
        font_extents(button->font, text, -1.f, width, height);
        *width += (real32_t)GetSystemMetrics(SM_CXMENUCHECK);
        *width += (real32_t)GetSystemMetrics(SM_CXEDGE);
        *height = (real32_t)GetSystemMetrics(SM_CYMENUCHECK);
        break;

    case ekBUTTON_FLAT:
    case ekBUTTON_FLATGLE:
        if (button->hpadding == UINT32_MAX)
            *width = (real32_t)(uint32_t)((refwidth * 1.5f) + .5f);
        else
            *width = refwidth + (real32_t)button->hpadding;

        if (button->vpadding == UINT32_MAX)
            *height = (real32_t)(uint32_t)((refheight * 1.5f) + .5f);
        else
            *height = refheight + (real32_t)button->vpadding;
        break;

    default:
        cassert_default(button_get_type(button->flags));
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_attach(OSButton *button, OSPanel *panel)
{
    _ospanel_attach_control(panel, cast(button, OSControl));
}

/*---------------------------------------------------------------------------*/

void osbutton_detach(OSButton *button, OSPanel *panel)
{
    _ospanel_detach_control(panel, cast(button, OSControl));
}

/*---------------------------------------------------------------------------*/

void osbutton_visible(OSButton *button, const bool_t visible)
{
    _oscontrol_set_visible(cast(button, OSControl), visible);
}

/*---------------------------------------------------------------------------*/

void osbutton_enabled(OSButton *button, const bool_t enabled)
{
    _oscontrol_set_enabled(cast(button, OSControl), enabled);
    if (button->flags == ekBUTTON_FLAT || button->flags == ekBUTTON_FLATGLE)
        InvalidateRect(button->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void osbutton_size(const OSButton *button, real32_t *width, real32_t *height)
{
    _oscontrol_get_size(cast_const(button, OSControl), width, height);
}

/*---------------------------------------------------------------------------*/

void osbutton_origin(const OSButton *button, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin(cast_const(button, OSControl), x, y);
}

/*---------------------------------------------------------------------------*/

void osbutton_frame(OSButton *button, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame(cast(button, OSControl), x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _osbutton_command(OSButton *button, WPARAM wParam, const bool_t restore_focus)
{
    cassert_no_null(button);
    if (HIWORD(wParam) == BN_CLICKED)
    {
        if (button->flags == ekBUTTON_RADIO)
        {
            /* This avoid the BN_CLICKED event in RadioButtons after receive the focus. */
            if (i_LAST_FOCUS == button->control.hwnd)
            {
                uint64_t microseconds;
                real64_t current_time;
                microseconds = btime_now();
                i_LAST_FOCUS = button->control.hwnd;
                current_time = i_TIME_SEC(microseconds);
                if (current_time - i_LAST_FOCUS_TIME < 0.01)
                    return;
            }

            SendMessage(button->control.hwnd, BM_SETCHECK, BST_CHECKED, (LPARAM)0);
        }

        if (IsWindowEnabled(button->control.hwnd) && button->OnClick != NULL)
        {
            EvButton params;
            params.index = 0;
            params.state = i_get_state(button->flags, button->control.hwnd);
            params.text = NULL;
            listener_event(button->OnClick, ekGUI_EVENT_BUTTON, button, &params, NULL, OSButton, EvButton, void);
        }

        _oswindow_release_transient_focus(cast(button, OSControl));
        unref(restore_focus);
    }
}

/*---------------------------------------------------------------------------*/

void _osbutton_toggle(OSButton *button)
{
    cassert_no_null(button);
    if (button->flags == ekBUTTON_CHECK2 || button->flags == ekBUTTON_CHECK3)
    {
        gui_state_t state = i_get_state(button->flags, button->control.hwnd);
        if (state == ekGUI_ON || state == ekGUI_MIXED)
            state = ekGUI_OFF;
        else
            state = ekGUI_ON;

        osbutton_state(button, state);
    }
}

/*---------------------------------------------------------------------------*/

void _osbutton_set_can_focus(OSButton *button, const bool_t can_focus)
{
    cassert_no_null(button);
    button->can_focus = can_focus;
}

/*---------------------------------------------------------------------------*/

void _osbutton_set_default(OSButton *button, const bool_t is_default)
{
    cassert_no_null(button);
    if (button_get_type(button->flags) == ekBUTTON_PUSH)
    {
        LONG style = GetWindowLong(button->control.hwnd, GWL_STYLE);

        if (is_default == TRUE)
            style |= BS_DEFPUSHBUTTON;
        else
            style &= ~BS_DEFPUSHBUTTON;

        SetWindowLong(button->control.hwnd, GWL_STYLE, style);
        InvalidateRect(button->control.hwnd, NULL, TRUE);
        button->is_default = is_default;
    }
    else
    {
        button->is_default = FALSE;
    }
}
