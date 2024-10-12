/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osfont.cpp
 *
 */

/* Fonts */

#include "font.h"
#include "font.inl"
#include "draw.inl"
#include "draw2d.inl"
#include "draw_win.inl"
#include <core/arrpt.h>
#include <core/arrst.h>
#include <core/heap.h>
#include <core/strings.h>
#include <osbs/osbs.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>
#include "draw2d_gdi.ixx"

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

/*---------------------------------------------------------------------------*/

typedef struct _user_font_t UserFont;

struct _user_font_t
{
    String *name;
    HANDLE handle;
};

DeclSt(UserFont);
static ArrSt(UserFont) *kUSER_FONTS = NULL;
static String *i_SYSTEM_FONT_FAMILY = NULL;

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

void osfont_alloc_globals(void)
{
    kUSER_FONTS = NULL;
}

/*---------------------------------------------------------------------------*/

void osfont_dealloc_globals(void)
{
    if (kUSER_FONTS != NULL)
        arrst_destroy(&kUSER_FONTS, i_remove_font, UserFont);

    str_destopt(&i_SYSTEM_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

static void i_metrics(NONCLIENTMETRICS *metrics)
{
    BOOL ret;
    cassert_no_null(metrics);

    metrics->cbSize = sizeof(NONCLIENTMETRICS);

#if _MSC_VER > 1400
    if (osbs_windows() <= ekWIN_XP)
        metrics->cbSize -= sizeof(metrics->iPaddedBorderWidth);
#endif

    ret = SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), metrics, 0);
    cassert_unref(ret != 0, ret);
}

/*---------------------------------------------------------------------------*/

static int i_font_height(const real32_t size, const uint32_t style)
{
    int height = 0;
    if ((style & ekFPOINTS) == ekFPOINTS)
    {
        height = -(int)((size * (real32_t)kLOG_PIXY) / 72.f);
    }
    else
    {
        cassert((style & ekFPIXELS) == ekFPIXELS);
        height = -(int)size;
    }

    if ((style & ekFCELL) == ekFCELL)
        height = -height;

    return height;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_system_font_family(void)
{
    if (i_SYSTEM_FONT_FAMILY == NULL)
    {
        NONCLIENTMETRICS metrics;
        char_t face_name[LF_FULLFACESIZE];
        i_metrics(&metrics);
        unicode_convers(cast_const(metrics.lfMessageFont.lfFaceName, char_t), face_name, ekUTF16, ekUTF8, sizeof(face_name));
        i_SYSTEM_FONT_FAMILY = str_c(face_name);
    }

    return tc(i_SYSTEM_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

static const char_t *i_monospace_font_family(void)
{
    const char_t *desired_fonts[] = {"Consolas", "Courier New"};
    return draw2d_monospace_family(desired_fonts, sizeof(desired_fonts) / sizeof(const char_t *));
}

/*---------------------------------------------------------------------------*/

OSFont *osfont_create(const char_t *family, const real32_t size, const real32_t width, const real32_t xscale, const uint32_t style)
{
    const char_t *face_name = NULL;
    WCHAR face_namew[LF_FULLFACESIZE];
    int nHeight = 0;
    HFONT hfont = NULL;

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

    /* face_name to UTF16 */
    {
        uint32_t bytes = unicode_convers(face_name, cast(face_namew, char_t), ekUTF8, ekUTF16, sizeof(face_namew));
        cassert_unref(bytes < sizeof(face_namew), bytes);
    }

    nHeight = i_font_height(size, style);

    hfont = CreateFont(
        nHeight,
        PARAM(nWidth, width >= 0 ? (int)width : 0),
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
    return cast(hfont, OSFont);
}

/*---------------------------------------------------------------------------*/

void osfont_destroy(OSFont **font)
{
    BOOL ret = 0;
    cassert_no_null(font);
    cassert_no_null(*font);
    ret = DeleteObject(*cast(font, HFONT));
    cassert_unref(ret != 0, ret);
    heap_auditor_delete("HFONT");
    *font = NULL;
}

/*---------------------------------------------------------------------------*/

String *osfont_family_name(const OSFont *font)
{
    HFONT hfont = (HFONT)font;
    LOGFONT lf;
    cassert_no_null(hfont);

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

void osfont_metrics(const OSFont *font, const real32_t size, const real32_t xscale, real32_t *ascent, real32_t *descent, real32_t *leading, real32_t *cell_size, real32_t *avg_width, bool_t *monospace)
{
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetDC(hwnd);
    HGDIOBJ cfont = SelectObject(hdc, (HFONT)font);
    TEXTMETRIC lptm;
    unref(size);
    GetTextMetrics(hdc, &lptm);

    if (ascent != NULL)
        *ascent = (real32_t)lptm.tmAscent;

    if (descent != NULL)
        *descent = (real32_t)lptm.tmDescent;

    if (leading != NULL)
        *leading = (real32_t)lptm.tmInternalLeading;

    if (cell_size != NULL)
        *cell_size = (real32_t)lptm.tmHeight;

    if (avg_width != NULL)
    {
        uint32_t len = 0;
        real32_t w = 0, h = 0;
        const char_t *str = draw2d_str_avg_char_width(&len);
        osfont_extents(font, str, xscale, -1, &w, &h);
        *avg_width = w / len;
    }

    if (monospace != NULL)
    {
        /*
         * If this bit is set the font is a variable pitch font. If this bit is clear the font
         * is a fixed pitch font. Note very carefully that those meanings are the opposite of
         * what the constant name implies.
         */
        if (lptm.tmPitchAndFamily & TMPF_FIXED_PITCH)
            *monospace = FALSE;
        else
            *monospace = TRUE;
    }

    SelectObject(hdc, cfont);
    ReleaseDC(hwnd, hdc);
}

/*---------------------------------------------------------------------------*/

void osfont_extents(const OSFont *font, const char_t *text, const real32_t xscale, const real32_t refwidth, real32_t *width, real32_t *height)
{
    MeasureStr data;
    HGDIOBJ cfont = NULL;
    int ret = 0;
    unref(xscale);
    data.hdc = GetDC(NULL);
    cfont = SelectObject(data.hdc, (HFONT)font);
    draw2d_extents(&data, draw_word_extents, TRUE, text, refwidth, width, height, MeasureStr);
    SelectObject(data.hdc, cfont);
    ret = ReleaseDC(NULL, data.hdc);
    cassert_unref(ret == 1, ret);
}

/*---------------------------------------------------------------------------*/

const void *osfont_native(const OSFont *font)
{
    cassert_no_null(font);
    return (HFONT)font;
}

/*---------------------------------------------------------------------------*/

real32_t font_regular_size(void)
{
    NONCLIENTMETRICS metrics;
    i_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)metrics.lfMessageFont.lfHeight;
}

/*---------------------------------------------------------------------------*/

real32_t font_small_size(void)
{
    NONCLIENTMETRICS metrics;
    i_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)(metrics.lfMessageFont.lfHeight + 2);
}

/*---------------------------------------------------------------------------*/

real32_t font_mini_size(void)
{
    NONCLIENTMETRICS metrics;
    i_metrics(&metrics);
    cassert(metrics.lfMessageFont.lfHeight < 0);
    return -(real32_t)(metrics.lfMessageFont.lfHeight + 4);
}

/*---------------------------------------------------------------------------*/

// const char_t *font_register(const byte_t *data, const uint32_t size);
// const char_t *font_register(const byte_t *data, const uint32_t size)
//{
//     DWORD nF = 0;
//     // Dont work in VS 2005
//     HANDLE handle = NULL;//AddFontMemResourceEx((PVOID)data, (DWORD)size, NULL, &nF);
//     const char_t *font_name = NULL;
//     cassert(FALSE);
//     if (handle != NULL)
//     {
//         /* Use of 'Gdiplus::PrivateFontCollection' to get the font name */
//         Gdiplus::PrivateFontCollection fonts;
//         Gdiplus::FontFamily family;
//         WCHAR wname[64];
//         char_t name[64];
//         Gdiplus::Status status = fonts.AddMemoryFont((const void*)data, (INT)size);
//         cassert(status == Gdiplus::Ok);
//         int nF2 = fonts.GetFamilyCount();
//         int found = 0;
//         UserFont *user_font = NULL;
//         cassert_unref(nF == (DWORD)nF2, nF2);
//         status = fonts.GetFamilies(1, &family, &found);
//         cassert(status == Gdiplus::Ok);
//         family.GetFamilyName(wname);
//         unicode_convers((const char_t*)wname, name, ekUTF16, ekUTF8, sizeof(name));
//         if (kUSER_FONTS == NULL)
//             kUSER_FONTS = arrst_create(UserFont);
//         user_font = arrst_new(kUSER_FONTS, UserFont);
//         user_font->handle = handle;
//         user_font->name = str_c(name);
//         font_name = tc(user_font->name);
//     }
//
//     return font_name;
// }

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
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetDC(hwnd);
    i_FontExists font_callback;
    int ret = 0;
    font_callback.font_family = family;
    font_callback.exists = FALSE;
    EnumFontFamilies(hdc, NULL, i_exists_font, (LPARAM)&font_callback);
    ret = ReleaseDC(hwnd, hdc);
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
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetDC(hwnd);
    i_FontInstalled font_callback;
    int ret = 0;
    font_callback.font_families = arrpt_create(String);
    font_callback.only_monospace = only_monospace;
    EnumFontFamilies(hdc, NULL, i_font_families, (LPARAM)&font_callback);
    ret = ReleaseDC(hwnd, hdc);
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