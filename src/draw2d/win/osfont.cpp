/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osfont.cpp
 *
 */

/* Fonts */

#include "draw_win.inl"
#include "../font.h"
#include "../font.inl"
#include "../draw.inl"
#include "../draw2d.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/heap.h>
#include <core/strings.h>
#include <osbs/osbs.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>
#include "draw2d_gdi.ixx"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

/*---------------------------------------------------------------------------*/

typedef struct _user_font_t UserFont;
typedef struct _hdpi_t HDpi;

struct _user_font_t
{
    String *name;
    HANDLE handle;
};

struct _hdpi_t
{
    uint32_t dpi;
    HFONT hfont;
    bool_t metrics;
    real32_t ascent;
    real32_t descent;
    real32_t leading;
    real32_t cell_size;
    real32_t avg_width;
    bool_t monospace;
};

DeclSt(UserFont);
DeclSt(HDpi);
static ArrSt(UserFont) *kUSER_FONTS = NULL;
static String *i_SYSTEM_FONT_FAMILY = NULL;
#define UNDEF_WIDTH 10000
#define UNDEF_HEIGHT 10000

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif

/* SystemParametersInfoForDpi (Windows 10 1607+), resolved dynamically for older SO compatibility */
typedef BOOL(__stdcall *SYSTEMPARAMETERSINFOFORDPI)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);
static SYSTEMPARAMETERSINFOFORDPI i_SystemParametersInfoForDpi = NULL;

/* DPI used by osfont_extents()/osfont_ascent()/etc. to measure text. Refreshed by 'oswindow' right
   before every layout compose/relocate pass, so text is measured at the real DPI of the window
   being laid out (not the DPI the font happened to be created at) */
static uint32_t i_METRICS_DPI = USER_DEFAULT_SCREEN_DPI;

struct _osfont_t
{
    String *family;
    real32_t size;
    real32_t width;
    uint32_t style;
    uint32_t dpi;
    ArrSt(HDpi) *hdpi;
};

/*---------------------------------------------------------------------------*/

static void i_remove_font(UserFont *font)
{
    BOOL ok;
    cassert_no_null(font);
    cassert(FALSE);
    /* Don't work in VS2005 */
    ok = 1; /* RemoveFontMemResourceEx(font->handle); */
    cassert_unref(ok != 0, ok);
    str_destroy(&font->name);
}

/*---------------------------------------------------------------------------*/

static void i_remove_hdpi(HDpi *hdpi)
{
    BOOL ret = 0;
    cassert_no_null(hdpi);
    ret = DeleteObject(hdpi->hfont);
    cassert_unref(ret != 0, ret);
    heap_auditor_delete("HFONT");
}

/*---------------------------------------------------------------------------*/

void osfont_alloc_globals(void)
{
    kUSER_FONTS = NULL;
    {
        HMODULE user32_mod = GetModuleHandle(L"user32.dll");
        if (user32_mod != NULL)
            i_SystemParametersInfoForDpi = cast_func(GetProcAddress(user32_mod, "SystemParametersInfoForDpi"), SYSTEMPARAMETERSINFOFORDPI);
    }
}

/*---------------------------------------------------------------------------*/

void osfont_dealloc_globals(void)
{
    if (kUSER_FONTS != NULL)
        arrst_destroy(&kUSER_FONTS, i_remove_font, UserFont);
    str_destopt(&i_SYSTEM_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

static void i_system_metrics(NONCLIENTMETRICS *metrics)
{
    BOOL ret;
    cassert_no_null(metrics);

    metrics->cbSize = sizeof(NONCLIENTMETRICS);

#if _MSC_VER > 1400
    if (osbs_windows() <= ekWIN_XP)
        metrics->cbSize -= sizeof(metrics->iPaddedBorderWidth);
#endif

    if (i_SystemParametersInfoForDpi != NULL)
        ret = i_SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), metrics, 0, USER_DEFAULT_SCREEN_DPI);
    else
        ret = SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), metrics, 0);

    cassert_unref(ret != 0, ret);
}

/*---------------------------------------------------------------------------*/

static int i_font_height(const real32_t size, const uint32_t dpi)
{
    /* 'size' is always in logical screen points (DPI-independent) */
    return -(int)((size * (real32_t)dpi) / (real32_t)USER_DEFAULT_SCREEN_DPI);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_system_font_family(void)
{
    if (i_SYSTEM_FONT_FAMILY == NULL)
    {
        NONCLIENTMETRICS metrics;
        char_t face_name[LF_FULLFACESIZE];
        i_system_metrics(&metrics);
        unicode_convers(cast_const(metrics.lfMessageFont.lfFaceName, char_t), face_name, ekUTF16, ekUTF8, sizeof(face_name));
        i_SYSTEM_FONT_FAMILY = str_c(face_name);
    }

    return tc(i_SYSTEM_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_monospace_font_family(void)
{
    const char_t *desired_fonts[] = {"Consolas", "Courier New"};
    return _draw2d_monospace_family(desired_fonts, sizeof(desired_fonts) / sizeof(const char_t *));
}

/*---------------------------------------------------------------------------*/

static uint32_t i_current_dpi(void)
{
    HDC hdc = GetDC(NULL);
    uint32_t dpi = (uint32_t)GetDeviceCaps(hdc, LOGPIXELSY);
    int ret = ReleaseDC(NULL, hdc);
    cassert_unref(ret == 1, ret);
    return dpi;
}

/*---------------------------------------------------------------------------*/

static HFONT i_create_hfont(const char_t *face_name, const real32_t size, const real32_t width, const uint32_t style, const uint32_t dpi)
{
    WCHAR face_namew[LF_FULLFACESIZE];
    int nHeight = 0;
    int nWidth = 0;
    HFONT hfont = NULL;

    /* face_name to UTF16 */
    {
        uint32_t bytes = unicode_convers(face_name, cast(face_namew, char_t), ekUTF8, ekUTF16, sizeof(face_namew));
        cassert_unref(bytes < sizeof(face_namew), bytes);
    }

    nHeight = i_font_height(size, dpi);

    /* 'width' is always in logical screen points (DPI-independent), same as 'size'/nHeight above */
    if (width >= 0)
        nWidth = (int)bmath_roundf((width * (real32_t)dpi) / (real32_t)USER_DEFAULT_SCREEN_DPI);

    hfont = CreateFont(
        nHeight,
        PARAM(nWidth, nWidth),
        PARAM(nEscapement, 0),
        PARAM(nOrientation, 0),
        (style & ekFBOLD) == ekFBOLD ? FW_BOLD : FW_MEDIUM,
        (DWORD)((style & ekFITALIC) == ekFITALIC ? TRUE : FALSE),
        (DWORD)((style & ekFUNDERLINE) == ekFUNDERLINE ? TRUE : FALSE),
        (DWORD)((style & ekFSTRIKEOUT) == ekFSTRIKEOUT ? TRUE : FALSE),
        PARAM(fdwCharSet, ANSI_CHARSET),
        PARAM(fdwOutputPrecision, OUT_TT_PRECIS),
        PARAM(fdwClipPrecision, CLIP_DEFAULT_PRECIS),
        PARAM(fdwQuality, DEFAULT_QUALITY),
        PARAM(fdwPitchAndFamily, DEFAULT_PITCH | FF_DONTCARE),
        face_namew);

    cassert_fatal_msg(hfont != NULL, "Font is not available on this computer");
    heap_auditor_add("HFONT");
    return hfont;
}

/*---------------------------------------------------------------------------*/

static HDpi *i_hdpi_entry(OSFont *font, const uint32_t dpi)
{
    cassert_no_null(font);
    arrst_foreach(hdpi, font->hdpi, HDpi)
        if (hdpi->dpi == dpi)
            return hdpi;
    arrst_end()

    {
        HDpi *hdpi = arrst_new0(font->hdpi, HDpi);
        hdpi->dpi = dpi;
        hdpi->hfont = i_create_hfont(tc(font->family), font->size, font->width, font->style, dpi);
        return hdpi;
    }
}

/*---------------------------------------------------------------------------*/

OSFont *osfont_create(const char_t *family, const real32_t size, const real32_t width, const real32_t xscale, const uint32_t style)
{
    const char_t *face_name = NULL;
    OSFont *font = NULL;
    HDpi *hdpi = NULL;

    cassert_no_null(family);
    cassert(size > 0.f);
    unref(xscale);

    if (str_equ_c(family, "__SYSTEM__") == TRUE)
    {
        face_name = i_system_font_family();
    }
    else if (str_equ_c(family, "__MONOSPACE__") == TRUE)
    {
        face_name = i_monospace_font_family();
    }
    else
    {
        face_name = family;
    }

    font = heap_new0(OSFont);
    font->family = str_c(face_name);
    font->size = size;
    font->width = width;
    font->style = style;
    font->dpi = i_current_dpi();
    font->hdpi = arrst_create(HDpi);
    hdpi = i_hdpi_entry(font, font->dpi);
    unref(hdpi);
    return font;
}

/*---------------------------------------------------------------------------*/

void osfont_destroy(OSFont **font)
{
    cassert_no_null(font);
    cassert_no_null(*font);
    arrst_destroy(&(*font)->hdpi, i_remove_hdpi, HDpi);
    str_destroy(&(*font)->family);
    heap_delete(font, OSFont);
}

/*---------------------------------------------------------------------------*/

String *osfont_family_name(const OSFont *font)
{
    LOGFONT lf;
    HFONT hfont = NULL;
    cassert_no_null(font);
    hfont = (HFONT)osfont_native(font);
    if (GetObject(hfont, sizeof(LOGFONT), &lf) == sizeof(LOGFONT))
    {
        char_t faceName[LF_FACESIZE];
        unicode_convers(cast_const(lf.lfFaceName, char_t), faceName, ekUTF16, ekUTF8, sizeof(faceName));
        return str_c(faceName);
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

font_family_t osfont_system(const char_t *family)
{
    {
        const char_t *system = i_system_font_family();
        if (str_equ_c(system, family) == TRUE)
            return ekFONT_FAMILY_SYSTEM;
    }

    {
        const char_t *mono = i_monospace_font_family();
        if (str_equ_c(mono, family) == TRUE)
            return ekFONT_FAMILY_MONOSPACE;
    }

    return ENUM_MAX(font_family_t);
}

/*---------------------------------------------------------------------------*/

static void i_metrics(const OSFont *font, HDpi *hdpi)
{
    if (hdpi->metrics == FALSE)
    {
        HDC hdc = GetDC(NULL);
        HGDIOBJ cfont = SelectObject(hdc, hdpi->hfont);
        real32_t scale = (real32_t)hdpi->dpi / (real32_t)USER_DEFAULT_SCREEN_DPI;
        TEXTMETRIC lptm;
        uint32_t len = 0;
        real32_t w = 0, h = 0;
        const char_t *str = NULL;

        GetTextMetrics(hdc, &lptm);
        hdpi->ascent = (real32_t)lptm.tmAscent / scale;
        hdpi->descent = (real32_t)lptm.tmDescent / scale;
        hdpi->leading = (real32_t)lptm.tmInternalLeading / scale;
        hdpi->cell_size = (real32_t)lptm.tmHeight / scale;

        /*
         * If this bit is set the font is a variable pitch font. If this bit is clear the font
         * is a fixed pitch font. Note very carefully that those meanings are the opposite of
         * what the constant name implies.
         */
        if (lptm.tmPitchAndFamily & TMPF_FIXED_PITCH)
            hdpi->monospace = FALSE;
        else
            hdpi->monospace = TRUE;

        SelectObject(hdc, cfont);
        ReleaseDC(NULL, hdc);

        str = _draw2d_str_avg_char_width(&len);
        osfont_extents(font, str, 1.f, -1, &w, &h);
        hdpi->avg_width = w / (real32_t)len;

        hdpi->metrics = TRUE;
    }
}

/*---------------------------------------------------------------------------*/

real32_t osfont_ascent(const OSFont *font)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    i_metrics(font, hdpi);
    return hdpi->ascent;
}

/*---------------------------------------------------------------------------*/

real32_t osfont_descent(const OSFont *font)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    i_metrics(font, hdpi);
    return hdpi->descent;
}

/*---------------------------------------------------------------------------*/

real32_t osfont_leading(const OSFont *font, const real32_t size)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    unref(size);
    i_metrics(font, hdpi);
    return hdpi->leading;
}

/*---------------------------------------------------------------------------*/

real32_t osfont_cell_size(const OSFont *font)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    i_metrics(font, hdpi);
    return hdpi->cell_size;
}

/*---------------------------------------------------------------------------*/

real32_t osfont_avg_width(const OSFont *font, const real32_t xscale)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    unref(xscale);
    i_metrics(font, hdpi);
    return hdpi->avg_width;
}

/*---------------------------------------------------------------------------*/

bool_t osfont_is_monospace(const OSFont *font)
{
    HDpi *hdpi = i_hdpi_entry(cast(font, OSFont), i_METRICS_DPI);
    i_metrics(font, hdpi);
    return hdpi->monospace;
}

/*---------------------------------------------------------------------------*/

void osfont_extents(const OSFont *font, const char_t *text, const real32_t xscale, const real32_t refwidth, real32_t *width, real32_t *height)
{
    WString str;
    const WCHAR *wtext = wstring_init(str_empty_c(text) ? " " : text, &str);
    HDC hdc = NULL;
    HGDIOBJ cfont = NULL;
    HFONT hfont = NULL;
    real32_t scale;
    RECT rect;
    int ret = 0;
    cassert_no_null(font);
    unref(xscale);
    cassert_no_null(width);
    cassert_no_null(height);
    hfont = (HFONT)osfont_native_dpi(cast(font, OSFont), i_METRICS_DPI);
    hdc = GetDC(NULL);
    cfont = SelectObject(hdc, hfont);
    scale = (real32_t)i_METRICS_DPI / (real32_t)USER_DEFAULT_SCREEN_DPI;
    rect.left = 0;
    rect.right = refwidth > 0 ? (LONG)(refwidth * scale) : UNDEF_WIDTH;
    rect.top = 0;
    rect.bottom = UNDEF_HEIGHT;
    DrawTextW(hdc, wtext, -1, &rect, DT_CALCRECT | DT_WORDBREAK);
    *width = bmath_ceilf((real32_t)(rect.right - rect.left) / scale);
    *height = bmath_ceilf((real32_t)(rect.bottom - rect.top) / scale);
    SelectObject(hdc, cfont);
    ret = ReleaseDC(NULL, hdc);
    cassert_unref(ret == 1, ret);
    wstring_remove(&str);
}

/*---------------------------------------------------------------------------*/

const void *osfont_native(const OSFont *font)
{
    cassert_no_null(font);
    return osfont_native_dpi(cast(font, OSFont), font->dpi);
}

/*---------------------------------------------------------------------------*/

const void *osfont_native_dpi(OSFont *font, const uint32_t dpi)
{
    HDpi *hdpi = i_hdpi_entry(font, dpi);
    return hdpi->hfont;
}

/*---------------------------------------------------------------------------*/

void osfont_metrics_dpi(const uint32_t dpi)
{
    i_METRICS_DPI = dpi;
}

/*---------------------------------------------------------------------------*/

real32_t font_regular_size(void)
{
    NONCLIENTMETRICS metrics;
    i_system_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)metrics.lfMessageFont.lfHeight;
}

/*---------------------------------------------------------------------------*/

real32_t font_small_size(void)
{
    NONCLIENTMETRICS metrics;
    i_system_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)(metrics.lfMessageFont.lfHeight + 2);
}

/*---------------------------------------------------------------------------*/

real32_t font_mini_size(void)
{
    NONCLIENTMETRICS metrics;
    i_system_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)(metrics.lfMessageFont.lfHeight + 4);
}

/*---------------------------------------------------------------------------*/

/*
const char_t *font_register(const byte_t *data, const uint32_t size);
const char_t *font_register(const byte_t *data, const uint32_t size)
{
    DWORD nF = 0;
    // Dont work in VS 2005
    HANDLE handle = NULL;//AddFontMemResourceEx((PVOID)data, (DWORD)size, NULL, &nF);
    const char_t *font_name = NULL;
    cassert(FALSE);
    if (handle != NULL)
    {
        // Use of 'Gdiplus::PrivateFontCollection' to get the font name
        Gdiplus::PrivateFontCollection fonts;
        Gdiplus::FontFamily family;
        WCHAR wname[64];
        char_t name[64];
        Gdiplus::Status status = fonts.AddMemoryFont((const void*)data, (INT)size);
        cassert(status == Gdiplus::Ok);
        int nF2 = fonts.GetFamilyCount();
        int found = 0;
        UserFont *user_font = NULL;
        cassert_unref(nF == (DWORD)nF2, nF2);
        status = fonts.GetFamilies(1, &family, &found);
        cassert(status == Gdiplus::Ok);
        family.GetFamilyName(wname);
        unicode_convers((const char_t*)wname, name, ekUTF16, ekUTF8, sizeof(name));
        if (kUSER_FONTS == NULL)
            kUSER_FONTS = arrst_create(UserFont);
        user_font = arrst_new(kUSER_FONTS, UserFont);
        user_font->handle = handle;
        user_font->name = str_c(name);
        font_name = tc(user_font->name);
    }

    return font_name;
}
*/

/*---------------------------------------------------------------------------*/

typedef struct _font_exists_t
{
    const char_t *font_family;
    bool_t exists;
} i_FontExists;

/*---------------------------------------------------------------------------*/

static int CALLBACK i_exists_font(const LOGFONT *lpelf, const TEXTMETRIC *lpntm, DWORD FontType, LPARAM lParam)
{
    i_FontExists *font_callback = cast(lParam, i_FontExists);
    cassert_no_null(lpelf);
    cassert_no_null(font_callback);
    unref(FontType);
    unref(lpntm);
    if (lpelf->lfFaceName[0] != '@')
    {
        char_t faceName[LF_FACESIZE];
        unicode_convers(cast_const(lpelf->lfFaceName, char_t), faceName, ekUTF16, ekUTF8, sizeof(faceName));
        if (str_equ_c(faceName, font_callback->font_family) == TRUE)
        {
            font_callback->exists = TRUE;
            return 0;
        }
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

bool_t font_exists_family(const char_t *family)
{
    HDC hdc = GetDC(NULL);
    i_FontExists font_callback;
    int ret = 0;
    font_callback.font_family = family;
    font_callback.exists = FALSE;
    EnumFontFamilies(hdc, NULL, i_exists_font, (LPARAM)&font_callback);
    ret = ReleaseDC(NULL, hdc);
    cassert_unref(ret == 1, ret);

    if (font_callback.exists == FALSE && kUSER_FONTS != NULL)
    {
        arrst_foreach(font, kUSER_FONTS, UserFont)
            if (str_equ(font->name, family) == TRUE)
                return TRUE;
        arrst_end()
    }

    return font_callback.exists;
}

/*---------------------------------------------------------------------------*/

typedef struct _font_installed_t
{
    ArrPt(String) *font_families;
    bool_t only_monospace;
} i_FontInstalled;

/*---------------------------------------------------------------------------*/

static int CALLBACK i_font_families(const LOGFONT *lpelf, const TEXTMETRIC *lpntm, DWORD fontType, LPARAM lParam)
{
    i_FontInstalled *font_callback = cast(lParam, i_FontInstalled);
    cassert_no_null(lpelf);
    cassert_no_null(font_callback);
    unref(fontType);
    unref(lpntm);
    if (lpelf->lfFaceName[0] != '@')
    {
        if (font_callback->only_monospace == FALSE || lpelf->lfPitchAndFamily & FIXED_PITCH)
        {
            char_t faceName[LF_FACESIZE];
            String *font_family = NULL;
            unicode_convers(cast_const(lpelf->lfFaceName, char_t), faceName, ekUTF16, ekUTF8, sizeof(faceName));
            font_family = str_c(faceName);
            arrpt_append(font_callback->font_families, font_family, String);
        }
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static ArrPt(String) *i_installed_families(const bool_t only_monospace)
{
    HDC hdc = GetDC(NULL);
    i_FontInstalled font_callback;
    int ret = 0;
    font_callback.font_families = arrpt_create(String);
    font_callback.only_monospace = only_monospace;
    EnumFontFamilies(hdc, NULL, i_font_families, (LPARAM)&font_callback);
    ret = ReleaseDC(NULL, hdc);
    cassert_unref(ret == 1, ret);
    arrpt_sort(font_callback.font_families, str_scmp, String);
    return font_callback.font_families;
}

/*---------------------------------------------------------------------------*/

ArrPt(String) *font_installed_families(void)
{
    return i_installed_families(FALSE);
}

/*---------------------------------------------------------------------------*/

ArrPt(String) *font_installed_monospace(void)
{
    return i_installed_families(TRUE);
}
