/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.cpp
 *
 */

/* Drawing custom GUI controls */

#include "osimg.inl"
#include "osstyleXP.inl"
#include "osgui_win.inl"
#include "../osdrawctrl.h"
#include <draw2d/color.h>
#include <draw2d/dctxh.h>
#include <draw2d/font.h>
#include <core/heap.h>
#include <osbs/osbs.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1400
#define DLISS_NORMAL LIS_NORMAL
#define DLISS_HOT LIS_HOT
#define DLISS_SELECTED LIS_SELECTED
#define DLISS_SELECTEDNOTFOCUS LIS_SELECTEDNOTFOCUS
#define DLISS_DISABLED LIS_DISABLED
#else
#define DLISS_NORMAL LISS_NORMAL
#define DLISS_HOT LISS_HOT
#define DLISS_SELECTED LISS_SELECTED
#define DLISS_SELECTEDNOTFOCUS LISS_SELECTEDNOTFOCUS
#define DLISS_DISABLED LISS_DISABLED
#endif

static LONG kCHECKBOX_WIDTH = 0;
static LONG kCHECKBOX_HEIGHT = 0;
#define UNDEF_WIDTH 10000
#define UNDEF_HEIGHT 10000

/*---------------------------------------------------------------------------*/

Font *osdrawctrl_font(const DCtx *ctx)
{
    HTHEME theme = _osstyleXP_OpenTheme(GetDesktopWindow(), L"Explorer::ListView");
    Font *font = NULL;

    unref(ctx);

    if (theme != NULL)
    {
        LOGFONTW lfont;
        if (GetThemeFont(theme, NULL, LVP_LISTITEM, DLISS_NORMAL, TMT_FONT, &lfont) == S_OK)
        {
            // TODO: Create a font from LOGFONT
            font = font_system(font_regular_size(), 0);
        }

        _osstyleXP_CloseTheme(theme);
    }

    if (font == NULL)
        font = font_system(font_regular_size(), 0);

    return font;
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_row_padding(const DCtx *ctx)
{
    unref(ctx);
    return 4;
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_check_width(const DCtx *ctx)
{
    unref(ctx);
    if (kCHECKBOX_WIDTH == 0)
        kCHECKBOX_WIDTH = GetSystemMetrics(SM_CXMENUCHECK);
    return (uint32_t)kCHECKBOX_WIDTH;
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_check_height(const DCtx *ctx)
{
    unref(ctx);
    if (kCHECKBOX_HEIGHT == 0)
        kCHECKBOX_HEIGHT = GetSystemMetrics(SM_CXMENUCHECK);
    return (uint32_t)kCHECKBOX_HEIGHT;
}

/*---------------------------------------------------------------------------*/

ctrl_msel_t osdrawctrl_multisel(const DCtx *ctx, const vkey_t key)
{
    unref(ctx);
    if (key == ekKEY_LCTRL || key == ekKEY_RCTRL)
        return ekCTRL_MSEL_SINGLE;
    else if (key == ekKEY_LSHIFT || key == ekKEY_RSHIFT)
        return ekCTRL_MSEL_BURST;
    return ekCTRL_MSEL_NO;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_clear(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const enum_t nonused)
{
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    FillRect((HDC)dctx_native(ctx), &rect, GetSysColorBrush(COLOR_WINDOW));
    unref(nonused);
}

/*---------------------------------------------------------------------------*/

static ___INLINE int i_list_state(const ctrl_state_t state)
{
    switch (state)
    {
    case ekCTRL_STATE_NORMAL:
    case ekCTRL_STATE_BKNORMAL:
        return DLISS_NORMAL;
    case ekCTRL_STATE_HOT:
    case ekCTRL_STATE_BKHOT:
        return DLISS_HOT;
    case ekCTRL_STATE_PRESSED:
        return DLISS_SELECTED;
    case ekCTRL_STATE_BKPRESSED:
        return DLISS_SELECTEDNOTFOCUS;
    case ekCTRL_STATE_DISABLED:
        return DLISS_DISABLED;
    default:
        cassert_default(state);
    }

    return DLISS_NORMAL;
}

/*---------------------------------------------------------------------------*/

static ___INLINE int i_header_state(const ctrl_state_t state)
{
    switch (state)
    {
    case ekCTRL_STATE_NORMAL:
    case ekCTRL_STATE_BKNORMAL:
        return HIS_NORMAL;
    case ekCTRL_STATE_HOT:
    case ekCTRL_STATE_BKHOT:
        return HIS_HOT;
    case ekCTRL_STATE_PRESSED:
    case ekCTRL_STATE_BKPRESSED:
        return HIS_PRESSED;
    case ekCTRL_STATE_DISABLED:
        return HIS_NORMAL;
    default:
        cassert_default(state);
    }

    return HIS_NORMAL;
}

/*---------------------------------------------------------------------------*/

static ___INLINE HTHEME i_button_theme(DCtx *ctx)
{
    OSDraw *custom_data = dctx_get_data(ctx, OSDraw);
    cassert_no_null(custom_data);
    return custom_data->button_theme;
}

/*---------------------------------------------------------------------------*/

static ___INLINE HTHEME i_list_theme(DCtx *ctx)
{
    OSDraw *custom_data = dctx_get_data(ctx, OSDraw);
    cassert_no_null(custom_data);
    return custom_data->list_theme;
}

/*---------------------------------------------------------------------------*/

static ___INLINE HTHEME i_header_theme(DCtx *ctx)
{
    OSDraw *custom_data = dctx_get_data(ctx, OSDraw);
    cassert_no_null(custom_data);
    return custom_data->header_theme;
}

/*---------------------------------------------------------------------------*/

static ___INLINE SIZE i_sort_size(DCtx *ctx)
{
    SIZE *sz = NULL;
    OSDraw *custom_data = dctx_get_data(ctx, OSDraw);
    cassert_no_null(custom_data);
    sz = &custom_data->sort_size;
    if (sz->cx == -1)
        GetThemePartSize(custom_data->header_theme, (HDC)dctx_native(ctx), HP_HEADERSORTARROW, HSAS_SORTEDDOWN, NULL, TS_DRAW, sz);
    return *sz;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_header(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    int istate = i_header_state(state);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    rect.left = (LONG)x + (LONG)offset_x + 1;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    _osstyleXP_DrawThemeBackground2(i_header_theme(ctx), HP_HEADERITEM, istate, (HDC)dctx_native(ctx), &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_indicator(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const indicator_t indicator)
{
    int istate = 0;

    if (indicator & ekINDDOWN_ARROW)
        istate = HSAS_SORTEDDOWN;
    else if (indicator & ekINDUP_ARROW)
        istate = HSAS_SORTEDUP;

    if (istate != 0)
    {
        RECT rect;
        real32_t offset_x = 0, offset_y = 0;
        HTHEME theme = i_header_theme(ctx);
        SIZE sz = i_sort_size(ctx);
        draw_set_raster_mode(ctx);
        dctx_offset(ctx, &offset_x, &offset_y);
        rect.left = (LONG)x + (LONG)offset_x + 1;
        rect.top = (LONG)y + (LONG)offset_y + 1;
        rect.right = rect.left + (LONG)width;
        rect.bottom = rect.top + sz.cy;
        unref(height);
        _osstyleXP_DrawThemeBackground2(theme, HP_HEADERSORTARROW, istate, (HDC)dctx_native(ctx), &rect);
    }
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_fill(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    int istate = i_list_state(state);
    HDC hdc = (HDC)dctx_native(ctx);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    if (osbs_windows() > ekWIN_XP3)
    {
        _osstyleXP_DrawThemeBackground2(i_list_theme(ctx), LVP_LISTITEM, istate, hdc, &rect);
    }
    else
    {
        /* Seems WindowsXP doesn't use Vista Styles for selected text */
        HBRUSH brush = NULL;
        if (state == ekCTRL_STATE_PRESSED || state == ekCTRL_STATE_BKPRESSED)
            brush = GetSysColorBrush(COLOR_HIGHLIGHT);
        else
            brush = GetSysColorBrush(COLOR_WINDOW);
        FillRect(hdc, &rect, brush);
    }
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_focus(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    unref(state);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    DrawFocusRect((HDC)dctx_native(ctx), &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_line(DCtx *ctx, const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1)
{
    HDC hdc = NULL;
    real32_t offset_x = 0, offset_y = 0;
    cassert_no_null(ctx);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    hdc = (HDC)dctx_native(ctx);
    MoveToEx(hdc, (int)(x0 + (int32_t)offset_x), (int)(y0 + (int32_t)offset_y), NULL);
    LineTo(hdc, (int)(x1 + (int32_t)offset_x), (int)(y1 + (int32_t)offset_y));
}

/*---------------------------------------------------------------------------*/

/* GDI does not handle ellipses well */
static void i_ellipsis(HDC hdc, WCHAR **text, const uint32_t num_chars, const uint32_t width, const ellipsis_t ellipsis)
{
    RECT rect = {0, 0, 0, 0};
    cassert_no_null(text);
    DrawTextW(hdc, *text, -1, &rect, DT_SINGLELINE | DT_CALCRECT);
    if ((uint32_t)rect.right > width)
    {
        const WCHAR *ellipchar = L"…";
        uint32_t vchars = (uint32_t)bmath_floorf(((real32_t)width * (real32_t)num_chars) / (real32_t)rect.right);
        if (vchars > 2)
        {
            vchars -= 1;
            switch (ellipsis)
            {
            case ekELLIPBEGIN:
                *text = *text + (num_chars - vchars);
                (*text)[0] = ellipchar[0];
                break;

            case ekELLIPMIDDLE:
            {
                uint32_t mchar = vchars / 2;
                uint32_t i, n = vchars - mchar;
                (*text)[mchar] = ellipchar[0];
                for (i = 1; i <= n; ++i)
                    (*text)[mchar + i] = (*text)[num_chars - n + i];
                (*text)[vchars] = '\0';
                break;
            }

            case ekELLIPEND:
                (*text)[vchars - 1] = ellipchar[0];
                (*text)[vchars] = '\0';
                break;

            case ekELLIPMLINE:
            case ekELLIPNONE:
            default:
                cassert_default(ellipsis);
            }
        }
        else if (vchars == 2)
        {
            (*text)[0] = ellipchar[0];
            (*text)[1] = '\0';
        }
        else
        {
            (*text)[0] = '\0';
        }
    }
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_text(DCtx *ctx, const char_t *text, const int32_t x, const int32_t y, const ctrl_state_t state)
{
    RECT rect;
    WString str;
    const WCHAR *wtext = _osgui_wstr_init(text, &str);
    real32_t offset_x = 0, offset_y = 0;
    align_t align = dctx_text_intalign(ctx);
    ellipsis_t trim = dctx_text_trim(ctx);
    real32_t text_width = dctx_text_width(ctx);
    HDC hdc = (HDC)dctx_native(ctx);
    UINT format = DT_TOP;

    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);

    /*
     * For GDI-based raster text in bitmap context, we have to 'delete' the GDI object
     * Check 'Using GDI+ on a GDI HDC'
     */
    cassert(dctx_internal_bitmap(ctx) == NULL);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = text_width >= 0 ? rect.left + (LONG)text_width : UNDEF_WIDTH;
    rect.bottom = rect.top + UNDEF_HEIGHT;

    switch (align)
    {
    case ekLEFT:
    case ekJUSTIFY:
        format |= DT_LEFT;
        break;
    case ekCENTER:
        format |= DT_CENTER;
        break;
    case ekRIGHT:
        format |= DT_RIGHT;
        break;
    default:
        cassert_default(align);
    }

    if (text_width >= 0)
    {
        switch (trim)
        {
        case ekELLIPNONE:
            format |= DT_SINGLELINE;
            break;
        case ekELLIPBEGIN:
        case ekELLIPMIDDLE:
        case ekELLIPEND:
            format |= DT_SINGLELINE;
            i_ellipsis(hdc, dcast(&wtext, WCHAR), str.nchars - 1, (uint32_t)text_width, trim);
            break;
        case ekELLIPMLINE:
            format |= DT_WORDBREAK;
            break;
        default:
            cassert_default(trim);
        }
    }

    /* Text alignment. DrawText need the real text width */
    if (text_width < 0 && !(format & DT_SINGLELINE) && ((format & DT_CENTER) || (format & DT_RIGHT)))
    {
        RECT nrect = {0, 0, 0, 0};
        DrawTextW(hdc, wtext, -1, &nrect, format | DT_CALCRECT);
        rect.right = rect.left + (nrect.right - nrect.left);
        rect.bottom = rect.top + (nrect.bottom - nrect.top);
    }

    if (dctx_text_color(ctx) == kCOLOR_DEFAULT)
    {
        if (osbs_windows() > ekWIN_XP3)
        {
            int istate = i_list_state(state);
            _osstyleXP_DrawThemeText2(i_list_theme(ctx), hdc, LVP_LISTITEM, istate, wtext, -1, format, &rect);
        }
        else
        {
            /* Seems WindowsXP doesn't use Vista Styles for selected text */
            COLORREF color = GetSysColor(COLOR_WINDOWTEXT);
            if (state == ekCTRL_STATE_PRESSED || state == ekCTRL_STATE_BKPRESSED)
                color = GetSysColor(COLOR_HIGHLIGHTTEXT);
            SetTextColor(hdc, color);
            DrawTextW(hdc, wtext, -1, &rect, format);
        }
    }
    else
    {
        DrawTextW(hdc, wtext, -1, &rect, format);
    }

    _osgui_wstr_remove(&str);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_image(DCtx *ctx, const Image *image, const int32_t x, const int32_t y, const ctrl_state_t state)
{
    HBITMAP hbitmap;
    real32_t offset_x = 0, offset_y = 0;
    HDC hdc = (HDC)dctx_native(ctx);
    HDC memhdc = NULL;
    LONG width, height;
    unref(state);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);
    hbitmap = _osimg_hbitmap_cache(image, (COLORREF)dctx_background_color(ctx), &width, &height);
    memhdc = CreateCompatibleDC(hdc);
    SelectObject(memhdc, hbitmap);
    BitBlt(hdc, (int)x + (int)offset_x, (int)y + (int)offset_y, width, height, memhdc, 0, 0, SRCCOPY);
    DeleteDC(memhdc);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_checkbox(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    int istate = 0;
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    HDC hdc = (HDC)dctx_native(ctx);
    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);

    switch (state)
    {
    case ekCTRL_STATE_NORMAL:
    case ekCTRL_STATE_BKNORMAL:
        istate = CBS_CHECKEDNORMAL;
        break;
    case ekCTRL_STATE_HOT:
    case ekCTRL_STATE_BKHOT:
        istate = CBS_CHECKEDHOT;
        break;
    case ekCTRL_STATE_PRESSED:
    case ekCTRL_STATE_BKPRESSED:
        istate = CBS_CHECKEDPRESSED;
        break;
    case ekCTRL_STATE_DISABLED:
        istate = CBS_CHECKEDDISABLED;
        break;
    default:
        cassert_default(state);
    }

    cassert((LONG)width == kCHECKBOX_WIDTH);
    cassert((LONG)height == kCHECKBOX_HEIGHT);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    _osstyleXP_DrawThemeBackground2(i_button_theme(ctx), BP_CHECKBOX, istate, hdc, &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_uncheckbox(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    int istate = 0;
    RECT rect;
    real32_t offset_x = 0, offset_y = 0;
    HDC hdc = (HDC)dctx_native(ctx);

    draw_set_raster_mode(ctx);
    dctx_offset(ctx, &offset_x, &offset_y);

    switch (state)
    {
    case ekCTRL_STATE_NORMAL:
    case ekCTRL_STATE_BKNORMAL:
        istate = CBS_UNCHECKEDNORMAL;
        break;
    case ekCTRL_STATE_HOT:
    case ekCTRL_STATE_BKHOT:
        istate = CBS_UNCHECKEDHOT;
        break;
    case ekCTRL_STATE_PRESSED:
    case ekCTRL_STATE_BKPRESSED:
        istate = CBS_UNCHECKEDPRESSED;
        break;
    case ekCTRL_STATE_DISABLED:
        istate = CBS_UNCHECKEDDISABLED;
        break;
    default:
        cassert_default(state);
    }

    cassert((LONG)width == kCHECKBOX_WIDTH);
    cassert((LONG)height == kCHECKBOX_HEIGHT);
    rect.left = (LONG)x + (LONG)offset_x;
    rect.top = (LONG)y + (LONG)offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    _osstyleXP_DrawThemeBackground2(i_button_theme(ctx), BP_CHECKBOX, istate, hdc, &rect);
}
