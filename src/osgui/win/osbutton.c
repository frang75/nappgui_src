/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
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
#include "osdrawctrl_win.inl"
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
#include <core/strings.h>
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
    gui_pos_t image_pos;
    uint16_t id;
    vkey_t key;
    Font *font;
    Image *image;
    String *text;
    HBITMAP dbuffer;
    real32_t twidth;
    real32_t theight;
    LONG width;
    LONG height;
    uint32_t hpadding;
    uint32_t vpadding;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

static HWND i_LAST_FOCUS = NULL;
static double i_LAST_FOCUS_TIME = 0.;
static real32_t i_BUTTON_IMAGE_SEP = 4.f;
static real32_t i_BUTTON_DEF_PADDING = 8.f;
#define i_TIME_SEC(microseconds) ((real64_t)microseconds / 1000000.)

/*---------------------------------------------------------------------------*/

static bool_t i_is_flat_button(const uint32_t flags)
{
    return (bool_t)(button_get_type(flags) == ekBUTTON_FLAT || button_get_type(flags) == ekBUTTON_FLATGLE);
}

/*---------------------------------------------------------------------------*/

static bool_t i_draw_flat_text(const OSButton *button)
{
    cassert_no_null(button);
    cassert(i_is_flat_button(button->flags) == TRUE);
    return (bool_t)(str_empty(button->text) == FALSE && button->image_pos != ekGUI_POS_NONE);
}

/*---------------------------------------------------------------------------*/

static void i_flat_content_size(const OSButton *button, const real32_t imgw, const real32_t imgh, const real32_t textw, const real32_t texth, real32_t *cwidth, real32_t *cheight)
{
    const bool_t draw_text = i_draw_flat_text(button);
    cassert_no_null(button);
    cassert_no_null(cwidth);
    cassert_no_null(cheight);

    if (imgw > 0.f && draw_text == TRUE)
    {
        switch (button->image_pos)
        {
        case ekGUI_POS_LEFT:
        case ekGUI_POS_RIGHT:
            *cwidth = imgw + i_BUTTON_IMAGE_SEP + textw;
            *cheight = imgh > texth ? imgh : texth;
            break;

        case ekGUI_POS_TOP:
        case ekGUI_POS_BOTTOM:
            *cwidth = imgw > textw ? imgw : textw;
            *cheight = imgh + i_BUTTON_IMAGE_SEP + texth;
            break;

        case ekGUI_POS_NONE:
        default:
            cassert_default(button->image_pos);
        }
    }
    else if (imgw > 0.f)
    {
        *cwidth = imgw;
        *cheight = imgh;
    }
    else
    {
        *cwidth = textw;
        *cheight = texth;
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_flat_button(OSButton *button, const Image *image)
{
    HWND hwnd = NULL;
    HDC hdc = NULL;
    HDC memHdc = NULL;
    PAINTSTRUCT ps;
    BOOL enabled;

    cassert_no_null(button);

    /* Drawing context */
    hwnd = button->control.hwnd;
    hdc = BeginPaint(hwnd, &ps);
    memHdc = CreateCompatibleDC(hdc);

    if (button->dbuffer == NULL)
        button->dbuffer = CreateCompatibleBitmap(hdc, button->width, button->height);

    SelectObject(memHdc, button->dbuffer);
    enabled = IsWindowEnabled(hwnd);

    /* Button background */
    {
        HTHEME theme = _osstyleXP_OpenTheme(hwnd, L"TOOLBAR");
        RECT rect;
        RECT border;

        rect.left = 0;
        rect.top = 0;
        rect.right = button->width;
        rect.bottom = button->height;
        border = rect;

        if (theme != NULL)
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

            if (osbs_windows() > ekWIN_XP3)
            {
                _osstyleXP_DrawThemeBackground(theme, hwnd, memHdc, TP_BUTTON, state, TRUE, &rect, &border);
            }
            else
            {
                HBRUSH brush = GetSysColorBrush(COLOR_BTNFACE);
                FillRect(memHdc, &rect, brush);
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

            _osstyleXP_DrawNonThemedButtonBackground(hwnd, memHdc, FALSE, state, &rect, &border);
        }

        if (theme != NULL)
            _osstyleXP_CloseTheme(theme);
    }

    /* Button content */
    {
        real32_t imgw = 0.f, imgh = 0.f, imgs = 0.f;
        real32_t txtw = 0.f, txth = 0.f;
        real32_t img_x = 0.f, img_y = 0.f;
        real32_t txt_x = 0.f, txt_y = 0.f;

        if (image != NULL)
        {
            imgw = (real32_t)image_width(image);
            imgh = (real32_t)image_height(image);
            imgs = i_BUTTON_IMAGE_SEP;
        }

        if (i_draw_flat_text(button) == TRUE)
        {
            if (button->twidth < 0.f)
                font_extents(button->font, tc(button->text), -1.f, &button->twidth, &button->theight);
            txtw = button->twidth;
            txth = button->theight;
        }

        /* Text/image positioning */
        {
            real32_t cwidth, cheight;
            real32_t origin_x, origin_y;
            i_flat_content_size(button, imgw, imgh, txtw, txth, &cwidth, &cheight);
            origin_x = ((real32_t)button->width - cwidth) / 2.f;
            origin_y = ((real32_t)button->height - cheight) / 2.f;

            switch (button->image_pos)
            {
            case ekGUI_POS_LEFT:
                img_x = origin_x;
                img_y = origin_y + (cheight - imgh) / 2.f;
                txt_x = origin_x + imgw + imgs;
                txt_y = origin_y + (cheight - txth) / 2.f;
                break;

            case ekGUI_POS_RIGHT:
                img_x = origin_x + txth + imgs;
                img_y = origin_y + (cheight - imgh) / 2.f;
                txt_x = origin_x;
                txt_y = origin_y + (cheight - txth) / 2.f;
                break;

            case ekGUI_POS_TOP:
                img_x = origin_x + (cwidth - imgw) / 2.f;
                img_y = origin_y;
                txt_x = origin_x + (cwidth - txtw) / 2.f;
                txt_y = origin_y + imgh + imgs;
                break;

            case ekGUI_POS_BOTTOM:
                img_x = origin_x + (cwidth - imgw) / 2.f;
                img_y = origin_y + txth + imgs;
                txt_x = origin_x + (cwidth - txtw) / 2.f;
                txt_y = origin_y;
                break;

            case ekGUI_POS_NONE:
                img_x = origin_x + (cwidth - imgw) / 2.f;
                img_y = origin_y + (cheight - imgh) / 2.f;
                txt_x = 0.f;
                txt_y = 0.f;
                break;

            default:
                cassert_default(button->image_pos);
            }
        }

        if (imgw > 0.f)
            _osimg_draw(image, memHdc, UINT32_MAX, img_x, img_y, imgw, imgh, !enabled);

        if (txtw > 0.f)
        {
            HGDIOBJ old_font = SelectObject(memHdc, (HFONT)font_native(button->font));
            COLORREF color = GetSysColor(enabled ? COLOR_BTNTEXT : COLOR_GRAYTEXT);
            SetBkMode(memHdc, TRANSPARENT);
            _osdrawctrl_gdi_text(memHdc, NULL, tc(button->text), (int32_t)txt_x, (int32_t)txt_y, ekLEFT, ekELLIPEND, -1, color, ekCTRL_STATE_NORMAL);
            SelectObject(memHdc, old_font);
        }
    }

    BitBlt(hdc, 0, 0, button->width, button->height, memHdc, 0, 0, SRCCOPY);
    DeleteDC(memHdc);

    {
        BOOL ok = EndPaint(hwnd, &ps);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

static void i_update_focus_show(const OSButton *button)
{
    cassert_no_null(button);
    if ((button_get_type(button->flags) == ekBUTTON_CHECK2 || button_get_type(button->flags) == ekBUTTON_CHECK3 || button_get_type(button->flags) == ekBUTTON_RADIO) && GetWindowTextLength(button->control.hwnd) > 0)
    {
        SendMessage(button->control.hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEFOCUS), 0);
    }
    else
    {
        SendMessage(button->control.hwnd, WM_UPDATEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
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

    case WM_ENABLE:
        /* Avoid flickering in flatbuttons when enabled/disabled */
        if (i_is_flat_button(button->flags) == TRUE)
        {
            LRESULT res = 0;
            SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
            res = CallWindowProc(button->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
            SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
            return res;
        }
        break;

    case WM_PAINT:
        if (i_is_flat_button(button->flags) == TRUE)
        {
            i_draw_flat_button(button, button->image);
            if (GetFocus() == button->control.hwnd)
                _oscontrol_draw_focus(hwnd, 3, 3, 3, 3);
            return 0;
        }
        break;

    case WM_SETFOCUS:
        if (button->can_focus == FALSE)
            return 0;

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
        i_update_focus_show(button);
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
    button->image_pos = ekGUI_POS_NONE;
    button->twidth = -1.f;
    button->theight = -1.f;
    button->hpadding = UINT32_MAX;
    button->vpadding = UINT32_MAX;
    button->key = ENUM_MAX(vkey_t);
    button->id = _osgui_unique_child_id();

    _oscontrol_init(cast(button, OSControl), PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), i_style(flags, ekCENTER), L"button", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);
    button->control.tooltip_hwnd1 = button->control.hwnd;

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
    str_destopt(&(*button)->text);

    if ((*button)->dbuffer != NULL)
    {
        DeleteObject((*button)->dbuffer);
        (*button)->dbuffer = NULL;
    }

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
    cassert_no_null(text);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);

    if (i_is_flat_button(button->flags) == TRUE)
    {
        str_upd(&button->text, text);
        InvalidateRect(button->control.hwnd, NULL, FALSE);
        button->twidth = -1.f;
        button->theight = -1.f;
    }
    else
    {
        _oscontrol_set_text(cast(button, OSControl), text);
    }

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
    _oscontrol_tooltip(cast(button, OSControl), text);
}

/*---------------------------------------------------------------------------*/

void osbutton_font(OSButton *button, const Font *font)
{
    cassert_no_null(button);
    cassert(_osbutton_text_allowed(button->flags) == TRUE);
    _oscontrol_update_font(cast(button, OSControl), &button->font, font);
    button->twidth = -1.f;
    button->theight = -1.f;
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

static void i_set_image(HWND hwnd, const Image *image)
{
    HBITMAP hbitmap = _osimg_hbitmap(image, 0);
    BOOL ok = FALSE;
    SendMessage(hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbitmap);
    ok = DeleteObject(hbitmap);
    cassert_unref(ok != 0, ok);
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
        cassert(i_is_flat_button(button->flags));
        button->image = ptr_copyopt(image_copy, image, Image);
        InvalidateRect(button->control.hwnd, NULL, FALSE);
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_image_pos(OSButton *button, const gui_pos_t pos)
{
    cassert_no_null(button);
    cassert(i_is_flat_button(button->flags) == TRUE);
    button->image_pos = pos;
    InvalidateRect(button->control.hwnd, NULL, FALSE);
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

    if (i_is_flat_button(button->flags) == TRUE)
        InvalidateRect(button->control.hwnd, NULL, FALSE);
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
    {
        real32_t textw = 0.f;
        real32_t texth = 0.f;
        real32_t cwidth = 0.f;
        real32_t cheight = 0.f;
        const bool_t draw_text = (bool_t)(str_empty_c(text) == FALSE && button->image_pos != ekGUI_POS_NONE);

        if (draw_text == TRUE)
            font_extents(button->font, text, -1.f, &textw, &texth);

        i_flat_content_size(button, refwidth, refheight, textw, texth, &cwidth, &cheight);

        if (button->hpadding == UINT32_MAX)
            *width = cwidth + (real32_t)(uint32_t)((refwidth * .5f) + .5f);
        else
            *width = cwidth + (real32_t)button->hpadding;

        if (button->vpadding == UINT32_MAX)
            *height = cheight + (real32_t)(uint32_t)((refheight * .5f) + .5f);
        else
            *height = cheight + (real32_t)button->vpadding;
        break;
    }

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
    cassert_no_null(button);
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
    cassert_no_null(button);
    _oscontrol_set_frame(cast(button, OSControl), x, y, width, height);

    if (button_get_type(button->flags) == ekBUTTON_FLAT || button_get_type(button->flags) == ekBUTTON_FLATGLE)
    {
        LONG lwidth = (LONG)width;
        LONG lheight = (LONG)height;
        if (lwidth != button->width || lheight != button->height)
        {
            button->width = lwidth;
            button->height = lheight;
            if (button->dbuffer != NULL)
            {
                DeleteObject(button->dbuffer);
                button->dbuffer = NULL;
            }
        }
    }
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
