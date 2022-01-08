/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.cpp
 *
 */

/* Drawing custom GUI controls */

#include "draw2d.ixx"
#include "win/dctx_win.inl"
#include "win/osimage.inl"
#include "dctx.inl"
#include "osdrawctrl.h"
#include "osdrawctrl.inl"
#include "osstyleXP.inl"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "font.inl"
#include "unicode.h"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

#if _MSC_VER > 1400
#define DLISS_NORMAL LISS_NORMAL
#define DLISS_HOT LISS_HOT
#define DLISS_SELECTED LISS_SELECTED
#define DLISS_SELECTEDNOTFOCUS LISS_SELECTEDNOTFOCUS
#define DLISS_DISABLED LISS_DISABLED
#else
#define DLISS_NORMAL LIS_NORMAL
#define DLISS_HOT LIS_HOT
#define DLISS_SELECTED LIS_SELECTED
#define DLISS_SELECTEDNOTFOCUS LIS_SELECTEDNOTFOCUS
#define DLISS_DISABLED LIS_DISABLED
#endif

static LONG kCHECKBOX_WIDTH = 0;
static LONG kCHECKBOX_HEIGHT = 0;

/*---------------------------------------------------------------------------*/

Font *osdrawctrl_font(const DCtx *ctx)
{
    HTHEME theme = osstyleXP_OpenTheme(GetDesktopWindow(), L"Explorer::ListView");
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

        osstyleXP_CloseTheme(theme);
    }

    if (font == NULL)
        font = font_system(font_regular_size(), 0);

    return font;
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_row_padding(const DCtx *ctx)
{
    unref(ctx);
    return 0;
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

multisel_t osdrawctrl_multisel(const DCtx *ctx, const vkey_t key)
{
    unref(ctx);
    if (key == ekKEY_LCTRL || key == ekKEY_RCTRL)
        return ekMULTISEL_SINGLE;
    else if (key == ekKEY_LSHIFT || key == ekKEY_RSHIFT)
        return ekMULTISEL_BURST;
    return ekMULTISEL_NO;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_clear(DCtx *ctx)
{
    //OSDraw *osdraw = (OSDraw*)ctx->custom_data;
    RECT rect;
    uint32_t w, h;
    dctx_set_gdi_mode(ctx);
    dctx_size(ctx, &w, &h);
    rect.left = 0;
    rect.top = 0;
    rect.right = (LONG)w;
    rect.bottom = (LONG)h;
    FillRect(ctx->hdc, &rect, GetSysColorBrush(COLOR_WINDOW));    
    //osstyleXP_DrawThemeBackgroundNoBorder(osdraw->list_theme, 0, 0, ctx->hdc, &rect);
}

/*---------------------------------------------------------------------------*/

static __INLINE int i_state(const cstate_t state)
{
    switch (state) {
    case ekCSTATE_NORMAL:
    case ekCSTATE_BKNORMAL:
        return DLISS_NORMAL;
    case ekCSTATE_HOT:
    case ekCSTATE_BKHOT:
        return DLISS_HOT;
    case ekCSTATE_PRESSED:
        return DLISS_SELECTED;
    case ekCSTATE_BKPRESSED:
        return DLISS_SELECTEDNOTFOCUS;
    case ekCSTATE_DISABLED:
        return DLISS_DISABLED;
    cassert_default();
    }

    return DLISS_NORMAL;
}

/*---------------------------------------------------------------------------*/

static __INLINE HTHEME i_list_theme(DCtx *ctx)
{
    cassert_no_null(ctx);
    cassert_no_null(ctx->custom_data);
    return ((OSDraw*)ctx->custom_data)->list_theme;
}

/*---------------------------------------------------------------------------*/

static __INLINE HTHEME i_button_theme(DCtx *ctx)
{
    cassert_no_null(ctx);
    cassert_no_null(ctx->custom_data);
    return ((OSDraw*)ctx->custom_data)->button_theme;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    RECT rect;
    int istate = i_state(state);    
    dctx_set_gdi_mode(ctx);
    rect.left = (LONG)x + (LONG)ctx->offset_x;
    rect.top = (LONG)y + (LONG)ctx->offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    osstyleXP_DrawThemeBackground2(i_list_theme(ctx), LVP_LISTITEM, istate, ctx->hdc, &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    RECT rect;
    unref(state);
    rect.left = (LONG)x + (LONG)ctx->offset_x;
    rect.top = (LONG)y + (LONG)ctx->offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    DrawFocusRect(ctx->hdc, &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state)
{
    RECT rect;
    uint32_t num_bytes = 0;
    WCHAR wtext[1024];

    dctx_set_gdi_mode(ctx);

    // For GDI-based raster text in bitmap context, we have to 'delete' the GDI object
    // Check 'Using GDI+ on a GDI HDC'
    cassert(ctx->bitmap == NULL);

    rect.left = (LONG)x + (LONG)ctx->offset_x;
    rect.right = ctx->text_width > 0 ? rect.left + (LONG)ctx->text_width : 10000;
    rect.top = (LONG)y + (LONG)ctx->offset_y;
    rect.bottom = rect.top + 10000;
    num_bytes = unicode_convers(text, (char_t*)wtext, ekUTF8, ekUTF16, sizeof(wtext));
    unref(num_bytes);

    if (ctx->text_color == kCOLOR_DEFAULT)
    {
        int istate = i_state(state);
        osstyleXP_DrawThemeText2(i_list_theme(ctx), ctx->hdc, LVP_LISTITEM, istate, wtext, -1, DT_LEFT | DT_END_ELLIPSIS, &rect);
    }
    else
    {
        DrawTextW(ctx->hdc, wtext, -1, &rect, DT_LEFT | DT_END_ELLIPSIS);
    }
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y, const cstate_t state)
{
    HBITMAP hbitmap;
    HDC memhdc = NULL;
    LONG width, height;
    unref(state);
    dctx_set_gdi_mode(ctx);
    hbitmap = osimage_hbitmap_cache(image, ctx->background_color, &width, &height);
    memhdc = CreateCompatibleDC(ctx->hdc);
    SelectObject(memhdc, hbitmap);
    BitBlt(ctx->hdc, (int)x + (int)ctx->offset_x, (int)y + (int)ctx->offset_y, width, height, memhdc, 0, 0, SRCCOPY);
    DeleteDC(memhdc);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    int istate = 0;
    RECT rect;

    dctx_set_gdi_mode(ctx);

    switch (state) {
    case ekCSTATE_NORMAL:
    case ekCSTATE_BKNORMAL:
        istate = CBS_CHECKEDNORMAL;
        break;

    case ekCSTATE_HOT:
    case ekCSTATE_BKHOT:
        istate = CBS_CHECKEDHOT;
        break;

    case ekCSTATE_PRESSED:
    case ekCSTATE_BKPRESSED:
        istate = CBS_CHECKEDPRESSED;
        break;

    case ekCSTATE_DISABLED:
        istate = CBS_CHECKEDDISABLED;
        break;

    cassert_default();
    }

    cassert((LONG)width == kCHECKBOX_WIDTH);
    cassert((LONG)height == kCHECKBOX_HEIGHT);
    rect.left = (LONG)x + (LONG)ctx->offset_x;
    rect.top = (LONG)y + (LONG)ctx->offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    osstyleXP_DrawThemeBackground2(i_button_theme(ctx), BP_CHECKBOX, istate, ctx->hdc, &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    int istate = 0;
    RECT rect;

    dctx_set_gdi_mode(ctx);

    switch (state) {
    case ekCSTATE_NORMAL:
    case ekCSTATE_BKNORMAL:
        istate = CBS_UNCHECKEDNORMAL;
        break;

    case ekCSTATE_HOT:
    case ekCSTATE_BKHOT:
        istate = CBS_UNCHECKEDHOT;
        break;

    case ekCSTATE_PRESSED:
    case ekCSTATE_BKPRESSED:
        istate = CBS_UNCHECKEDPRESSED;
        break;

    case ekCSTATE_DISABLED:
        istate = CBS_UNCHECKEDDISABLED;
        break;

    cassert_default();
    }

    cassert((LONG)width == kCHECKBOX_WIDTH);
    cassert((LONG)height == kCHECKBOX_HEIGHT);
    rect.left = (LONG)x + (LONG)ctx->offset_x;
    rect.top = (LONG)y + (LONG)ctx->offset_y;
    rect.right = rect.left + (LONG)width;
    rect.bottom = rect.top + (LONG)height;
    osstyleXP_DrawThemeBackground2(i_button_theme(ctx), BP_CHECKBOX, istate, ctx->hdc, &rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_header(HWND hwnd, HDC hdc, HFONT font, const RECT *rect, int state, const WCHAR *text, const align_t align, const Image *image)
{
    BOOL use_style = FALSE;

    cassert_no_null(rect);
    cassert(FALSE);

    use_style = osstyleXP_OpenThemeData(hwnd, L"HEADER");

    if (use_style == TRUE)
    {
        DWORD flags;
        RECT rect2;

        switch (align) {
        case ekLEFT:
        case ekJUSTIFY:
            flags = DT_LEFT;
            break;
        case ekCENTER:
            flags = DT_CENTER;
            break;
        case ekRIGHT:
            flags = DT_RIGHT;
            break;
        cassert_default();
        }

        flags |= DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;

        osstyleXP_DrawThemeBackground(hwnd, hdc, HP_HEADERITEM, state, FALSE, rect, NULL);
        cassert(osstyleXP_HasThemeFont(hdc, HP_HEADERITEM, state, TMT_FONT) == FALSE);
        SelectObject(hdc, font);
        rect2 = *rect;
        InflateRect(&rect2, -16, 0);
        osstyleXP_DrawThemeText(hdc, HP_HEADERITEM, state, text, UINT32_MAX, flags, &rect2);
        rect2 = *rect;
        rect2.bottom = rect2.top + 10;
        osstyleXP_DrawThemeText(hdc, HP_HEADERSORTARROW, HSAS_SORTEDUP, L"W", UINT32_MAX, flags, rect);

    //LOGFONT lf;
    //SelectObject(hdc, tv->font);
    //res = GetThemeFont(i_STYLEXP.theme, hdc, iPartId, iStateId, TMT_FONT, &lf);
    //cassert_unref(res == S_OK, res);

    }
    else
    {
        cassert(FALSE);
        //UINT state = DFCS_BUTTONPUSH;
        //if (enabled == FALSE)
        //{
        //    state |= DFCS_INACTIVE;
        //}
        //else if (SendMessage(hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED)
        //{
        //    state |= DFCS_PUSHED;
        //}
        //else if (_osgui_hit_test(hwnd) == TRUE)
        //{
        //    if ((GetKeyState(VK_LBUTTON) & 0x100) != 0)
        //        state |= DFCS_PUSHED;
        //    else
        //        state |= DFCS_HOT;
        //}

        //osstyleXP_DrawNonThemedButtonBackground(hwnd, hdc, FALSE, state, &rect, &border);
    }

    if (image != NULL)
    {
    //    uint32_t width, height;
    //    uint32_t offset_x, offset_y;
    //    image_size(image, &width, &height);
    //    offset_x = (rect.right - rect.left - width) / 2;
    //    offset_y = (rect.bottom - rect.top - height) / 2;
    //    osimage_draw(image, hdc, UINT32_MAX, (real32_t)offset_x, (real32_t)offset_y, (real32_t)width, (real32_t)height, !enabled);
    }

    if (use_style == TRUE)
        osstyleXP_CloseThemeData();
}
