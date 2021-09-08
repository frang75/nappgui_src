/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbutton.c
 *
 */

/* Operating System native button */

#include "osbutton.h"
#include "osbutton.inl"
#include "osgui.inl"
#include "osgui_win.inl"
#include "win/osimage.inl"
#include "oscontrol.inl"
#include "osstyleXP.inl"
#include "osdrawctrl.inl"
#include "ospanel.inl"
#include "oswindow.inl"

#include "cassert.h"
#include "bmath.h"
#include "btime.h"
#include "event.h"
#include "font.h"
#include "font.inl"
#include "heap.h"
#include "image.h"
#include "ptr.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

struct _osbutton_t
{
    OSControl control;
    button_flag_t flags;
    bool_t def;
    Font *font;
    Image *image;
    Listener *OnClick;
};

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

    withXP_style = osstyleXP_OpenThemeData(hwnd, L"TOOLBAR");
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

        osstyleXP_DrawThemeBackground(hwnd, hdc, TP_BUTTON, state, TRUE, &rect, &border);
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

        osstyleXP_DrawNonThemedButtonBackground(hwnd, hdc, FALSE, state, &rect, &border);
    }

    if (image != NULL)
    {
        uint32_t width = image_width(image);
        uint32_t height = image_height(image);
        uint32_t offset_x = (rect.right - rect.left - width) / 2;
        uint32_t offset_y = (rect.bottom - rect.top - height) / 2;
        osimage_draw(image, hdc, UINT32_MAX, (real32_t)offset_x, (real32_t)offset_y, (real32_t)width, (real32_t)height, !enabled);
    }

    if (withXP_style == TRUE)
        osstyleXP_CloseThemeData();

    {
        BOOL ok = EndPaint(hwnd, &ps);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_header_button(HWND hwnd, const Font *font, const Image *image)
{
    HDC hdc = NULL;
    PAINTSTRUCT ps;
    RECT rect;
    BOOL enabled;
    HFONT hfont;
    WCHAR text[WCHAR_BUFFER_SIZE];
    int state = 0;
    cassert_no_null(hwnd);
    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rect);
    enabled = IsWindowEnabled(hwnd);

    if (enabled == FALSE)
    {
        state = HIS_NORMAL;
    }
    else if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
    {
        state = HIS_PRESSED;
    }
    else if (_osgui_hit_test(hwnd) == TRUE)
    {
        if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
            state = HIS_PRESSED;
        else
            state = HIS_HOT;
    }
    else
    {
        state = HIS_NORMAL;
    }

    hfont = (HFONT)font_native(font);
    SendMessage(hwnd, WM_GETTEXT, (WPARAM)WCHAR_BUFFER_SIZE, (LPARAM)text);

    osdrawctrl_header(hwnd, hdc, hfont, &rect, state, text, ekRIGHT, image);

    {
        BOOL ok = EndPaint(hwnd, &ps);
        cassert_unref(ok != 0, ok);
    }
}

/*---------------------------------------------------------------------------*/

static LRESULT CALLBACK i_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    OSButton *button = (OSButton*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    cassert_no_null(button);

    switch (uMsg) {
	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		if (_oswindow_in_resizing(hwnd) == TRUE)
			return 0;

		if (button_type(button->flags) == ekBTFLAT
			|| button_type(button->flags) == ekBTFLATGLE)
		{
			i_draw_flat_button(button->control.hwnd, button->image);
			if (GetFocus() == button->control.hwnd)
				_oscontrol_draw_focus(hwnd, 3, 3, 3, 3);
		}
        else if (button_type(button->flags) == ekBTHEADER)
        {
			i_draw_header_button(button->control.hwnd, button->font, button->image);
			if (GetFocus() == button->control.hwnd)
				_oscontrol_draw_focus(hwnd, 3, 3, 3, 3);
        }
		else
		{
			CallWindowProc(button->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
		}

		return 0;

	case WM_SETFOCUS:
		if (button_type(button->flags) == ekBTRADIO)
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

		break;
    }

    return CallWindowProc(button->control.def_wnd_proc, hwnd, uMsg, wParam, lParam);
}

/*---------------------------------------------------------------------------*/

static DWORD i_button_skin(const button_flag_t flags)
{
    switch (flags)
    {
        case ekBTPUSH:
        case ekBTFLAT:
        case ekBTHEADER:
            return BS_PUSHBUTTON;
        case ekBTCHECK2:
        case ekBTFLATGLE:
            return BS_AUTOCHECKBOX;
        case ekBTCHECK3:
            return BS_AUTO3STATE;
        case ekBTRADIO:
            return BS_RADIOBUTTON;

        case ekBTTYPE:
        cassert_default();
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
        cassert_default();
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

OSButton *osbutton_create(const button_flag_t flags)
{
    OSButton *button = heap_new0(OSButton);
    button->control.type = ekGUI_COMPONENT_BUTTON;
    button->flags = flags;
    button->def = FALSE;

    _oscontrol_init((OSControl*)button, PARAM(dwExStyle, WS_EX_NOPARENTNOTIFY), i_style(flags, ekCENTER), L"button", 0, 0, i_WndProc, kDEFAULT_PARENT_WINDOW);

    if (_osgui_button_text_allowed(flags) == TRUE)
    {
        button->font = _osgui_create_default_font();
        _oscontrol_set_font((OSControl*)button, button->font);
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
    cassert(_osgui_button_text_allowed(button->flags) == TRUE);
    _oscontrol_set_text((OSControl*)button, text);
}

/*---------------------------------------------------------------------------*/

void osbutton_tooltip(OSButton *button, const char_t *text)
{
    _oscontrol_set_tooltip((OSControl*)button, text);
}

/*---------------------------------------------------------------------------*/

static void i_set_image(HWND hwnd, const Image *image)
{
    HBITMAP hbitmap = osimage_hbitmap(image, 0);
    BOOL ok = FALSE;
    SendMessage(hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbitmap);
    ok = DeleteObject(hbitmap);
    cassert_unref(ok != 0, ok);
}

/*---------------------------------------------------------------------------*/

void osbutton_font(OSButton *button, const Font *font)
{
    cassert_no_null(button);
    cassert(_osgui_button_text_allowed(button->flags) == TRUE);
    _oscontrol_update_font((OSControl*)button, &button->font, font);
    if (button_type(button->flags) == ekBTPUSH && button->image != NULL)
    {
        Image *image = _osgui_scale_image(button->image, button->font);
        image_destroy(&button->image);
        button->image = image;
        i_set_image(button->control.hwnd, button->image);
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_align(OSButton *button, const align_t align)
{
    DWORD dwStyle = 0;
    cassert_no_null(button);
    cassert(_osgui_button_text_allowed(button->flags) == TRUE);
    dwStyle = i_style(button->flags, align);
    SetWindowLongPtr(button->control.hwnd, GWL_STYLE, dwStyle);
}

/*---------------------------------------------------------------------------*/

void osbutton_image(OSButton *button, const Image *image)
{
    cassert_no_null(button);
    cassert(_osgui_button_image_allowed(button->flags) == TRUE);
    ptr_destopt(image_destroy, &button->image, Image);
    if (button_type(button->flags) == ekBTPUSH)
    {
        if (image != NULL)
        {
            button->image = _osgui_scale_image(image, button->font);
            i_set_image(button->control.hwnd, button->image);
        }
        else
        {
            SendMessage(button->control.hwnd, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)NULL);
        }
    }
    else
    {
        cassert(button_type(button->flags) == ekBTFLAT
            || button_type(button->flags) == ekBTFLATGLE);
        button->image = image_copy(image);
    }
}

/*---------------------------------------------------------------------------*/

//enum gui_position_t
//{
//    ekGUI_POSITION_TOP          = 1,
//    ekGUI_POSITION_BOTTOM       = 4,
//    ekGUI_POSITION_LEFT         = 8,
//    ekGUI_POSITION_RIGHT        = 32
//};

//void osbutton_set_image_position(OSButton *button, const enum gui_position_t position);
//void osbutton_set_image_position(OSButton *button, const enum gui_position_t position)
//{
//    unref(button);
//    cassert(_osgui_button_image_allowed(button->flags) == TRUE);
//    unref(position);
//    cassert(FALSE);
//}

/*---------------------------------------------------------------------------*/

void osbutton_state(OSButton *button, const state_t state)
{
    cassert_no_null(button);
    cassert(button->control.type == ekGUI_COMPONENT_BUTTON);
    if (button->flags == ekBTCHECK2 || button->flags == ekBTRADIO || button->flags == ekBTFLATGLE)
    {
        DWORD cstate = 0;
        switch (state)
        {
            case ekON:
            case ekMIXED:
                cstate = BST_CHECKED;
                break;
            case ekOFF:
                cstate = BST_UNCHECKED;
                break;
            cassert_default();
        }

        SendMessage(button->control.hwnd, BM_SETCHECK, cstate, (LPARAM)0);
    }
    else if (button->flags == ekBTCHECK3)
    {
        DWORD cstate = 0;
        switch (state)
        {
            case ekON:
                cstate = BST_CHECKED;
                break;
            case ekOFF:
                cstate = BST_UNCHECKED;
                break;
            case ekMIXED:
                cstate = BST_INDETERMINATE;
                break;
            cassert_default();
        }

        SendMessage(button->control.hwnd, BM_SETCHECK, cstate, (LPARAM)0);
    }
}

/*---------------------------------------------------------------------------*/

static state_t i_get_state(const button_flag_t flags, HWND hwnd)
{
    switch (button_type(flags))
    {
        case ekBTPUSH:
        case ekBTFLAT:
        case ekBTHEADER:
            return ekON;

        case ekBTCHECK2:
        case ekBTCHECK3:
        case ekBTRADIO:
        case ekBTFLATGLE:
        {
            register LRESULT state = SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
            if (state == BST_CHECKED)
            {
                return ekON;
            }
            else if (state == BST_UNCHECKED)
            {
                return ekOFF;
            }
            else
            {
                cassert(state == BST_INDETERMINATE);
                return ekMIXED;
            }
        }

        cassert_default();
    }

    return ENUM_MAX(state_t);
}

/*---------------------------------------------------------------------------*/

state_t osbutton_get_state(const OSButton *button)
{
    cassert_no_null(button);
    return i_get_state(button->flags, button->control.hwnd);
}

/*---------------------------------------------------------------------------*/

void osbutton_bounds(const OSButton *button, const char_t *text, const real32_t refwidth, const real32_t refheight, real32_t *width, real32_t *height)
{
    cassert_no_null(button);
    cassert_no_null(width);
    cassert_no_null(height);

    switch (button_type(button->flags))
    {
        case ekBTPUSH:
        case ekBTHEADER:
        {
            real32_t woff, hoff;

            _oscontrol_text_bounds((const OSControl*)button, text, button->font, -1.f, width, height);

            if (refheight > *height)
                *height = refheight;

            if (button_type(button->flags) == ekBTPUSH)
                _oscontrol_text_bounds((const OSControl*)button, "O", button->font, -1.f, &woff, &hoff);
            else
                _oscontrol_text_bounds((const OSControl*)button, "O", button->font, -1.f, &woff, &hoff);

            if (refwidth > 0.f)
            {
                *width += refwidth;
                *width += (real32_t)(2 * GetSystemMetrics(SM_CXEDGE));
            }

            *width += 2 * woff;
            *height = bmath_ceilf(1.5f * *height) + 2.f;
            break;
        }

        case ekBTCHECK2:
        case ekBTCHECK3:
        case ekBTRADIO:
            _oscontrol_text_bounds((const OSControl*)button, text, button->font, -1.f, width, height);
            *width += (real32_t)GetSystemMetrics(SM_CXMENUCHECK);
            *width += (real32_t)GetSystemMetrics(SM_CXEDGE);
            *height = (real32_t)GetSystemMetrics(SM_CYMENUCHECK);
            break;

        case ekBTFLAT:
        case ekBTFLATGLE:
            *width = (real32_t)(uint32_t)((refwidth * 1.5f) + .5f);
            *height = (real32_t)(uint32_t)((refheight * 1.5f) + .5f); 
            break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void osbutton_attach(OSButton *button, OSPanel *panel)
{
    _ospanel_attach_control(panel, (OSControl*)button);
}

/*---------------------------------------------------------------------------*/

void osbutton_detach(OSButton *button, OSPanel *panel)
{
    _ospanel_detach_control(panel, (OSControl*)button);
}

/*---------------------------------------------------------------------------*/

void osbutton_visible(OSButton *button, const bool_t visible)
{
    _oscontrol_set_visible((OSControl*)button, visible);
}

/*---------------------------------------------------------------------------*/

void osbutton_enabled(OSButton *button, const bool_t enabled)
{
    _oscontrol_set_enabled((OSControl*)button, enabled);
    if (button->flags == ekBTFLAT || button->flags == ekBTFLATGLE);
        InvalidateRect(button->control.hwnd, NULL, FALSE);
}

/*---------------------------------------------------------------------------*/

void osbutton_size(const OSButton *button, real32_t *width, real32_t *height)
{
    _oscontrol_get_size((const OSControl*)button, width, height);
}

/*---------------------------------------------------------------------------*/

void osbutton_origin(const OSButton *button, real32_t *x, real32_t *y)
{
    _oscontrol_get_origin((const OSControl*)button, x, y);
}

/*---------------------------------------------------------------------------*/

void osbutton_frame(OSButton *button, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    _oscontrol_set_frame((OSControl*)button, x, y, width, height);
}

/*---------------------------------------------------------------------------*/

void _osbutton_detach_and_destroy(OSButton **button, OSPanel *panel)
{
    cassert_no_null(button);
    osbutton_detach(*button, panel);
    osbutton_destroy(button);
}

/*---------------------------------------------------------------------------*/

void _osbutton_command(OSButton *button, WPARAM wParam)
{
    cassert_no_null(button);
    if (HIWORD(wParam) == BN_CLICKED)
    {
        if (button->flags == ekBTRADIO)
        {
            // This avoid the BN_CLICKED event in RadioButtons after receive the focus. 
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
            listener_event(button->OnClick, ekEVBUTTON, button, &params, NULL, OSButton, EvButton, void);
        }

        if (button->def == FALSE && button_type(button->flags) == ekBTPUSH)
            _osbutton_unset_default(button);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _osbutton_is_pushbutton(const OSButton *button)
{
    cassert_no_null(button);
    return (bool_t)(button_type(button->flags) == ekBTPUSH);
}

/*---------------------------------------------------------------------------*/

void _osbutton_set_default(OSButton *button)
{
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTPUSH);
    {
        LONG style = GetWindowLong(button->control.hwnd, GWL_STYLE);
        style &= ~BS_PUSHBUTTON;
        style |= BS_DEFPUSHBUTTON;
        SetWindowLong(button->control.hwnd, GWL_STYLE, style);
        InvalidateRect(button->control.hwnd, NULL, TRUE);
        button->def = TRUE;
    }
}

/*---------------------------------------------------------------------------*/

void _osbutton_unset_default(OSButton *button)
{
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTPUSH);
    {
        LONG style = GetWindowLong(button->control.hwnd, GWL_STYLE);
        style &= ~BS_DEFPUSHBUTTON;
        style |= BS_PUSHBUTTON;
        SetWindowLong(button->control.hwnd, GWL_STYLE, style);
        InvalidateRect(button->control.hwnd, NULL, TRUE);
        button->def = FALSE;
    }
}

