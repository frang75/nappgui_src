/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw_win.cpp
 *
 */

/* Drawing commands */

#include "dctx_win.inl"
#include "draw_win.inl"
#include "../draw.h"
#include "../draw.inl"
#include "../dctxh.h"
#include "../font.inl"
#include "../image.inl"
#include "../color.h"
#include "../font.h"
#include <geom2d/v2d.h>
#include <core/heap.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is only for Windows
#endif

/*---------------------------------------------------------------------------*/

static Gdiplus::ColorPalette *i_kGRAY8_PALETTE = NULL;
int kLOG_PIXY = 0;
LONG kTWIPS_PER_PIXEL = 0;

/*---------------------------------------------------------------------------*/

void _draw_alloc_globals(void)
{
    i_kGRAY8_PALETTE = NULL;

    /* TWIPS for Font Size */
    {
        HDC hdc = GetDC(NULL);
        kLOG_PIXY = GetDeviceCaps(hdc, LOGPIXELSY);
        int ret = ReleaseDC(NULL, hdc);
        cassert_unref(ret == 1, ret);
        kTWIPS_PER_PIXEL = 1440 / kLOG_PIXY;
    }
}

/*---------------------------------------------------------------------------*/

void _draw_dealloc_globals(void)
{
    if (i_kGRAY8_PALETTE != NULL)
        heap_free(dcast(&i_kGRAY8_PALETTE, byte_t), sizeof(Gdiplus::ColorPalette) + 256 * sizeof(Gdiplus::ARGB), "Gray8Palette");
}

/*---------------------------------------------------------------------------*/

void _draw_word_extents(MeasureStr *data, const char_t *word, real32_t *width, real32_t *height)
{
    unref(data);
    unref(word);
    unref(width);
    unref(height);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

const WCHAR *wstring_init(const char_t *text, WString *str)
{
    WCHAR *wtext = NULL;
    cassert_no_null(str);
    str->nchars = 1 + unicode_nchars(text, ekUTF8);

    if (str->nchars < STATIC_TEXT_SIZE)
    {
        str->alloctext = NULL;
        wtext = str->statictext;
    }
    else
    {
        str->alloctext = cast(heap_malloc(str->nchars * sizeof(WCHAR), "WString_osdraw"), WCHAR);
        wtext = str->alloctext;
    }

    {
        uint32_t bytes = unicode_convers(text, cast(wtext, char_t), ekUTF8, ekUTF16, str->nchars * sizeof(WCHAR));
        cassert_unref(bytes == str->nchars * sizeof(WCHAR), bytes);
    }

    return wtext;
}

/*---------------------------------------------------------------------------*/

void wstring_remove(WString *str)
{
    cassert_no_null(str);
    if (str->alloctext != NULL)
        heap_free(dcast(&str->alloctext, byte_t), str->nchars * sizeof(WCHAR), "WString_osdraw");
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_set_gdiplus_mode(DCtx *ctx)
{
    cassert_no_null(ctx);
    if (ctx->gdi_mode == TRUE)
    {
        /* Perhaps here we have to create a new Gdiplus::Graphics and init */
        ctx->gdi_mode = FALSE;
    }
}

/*---------------------------------------------------------------------------*/

void _draw_imgimp(DCtx *ctx, const OSImage *image, const uint32_t frame_index, const real32_t x, const real32_t y, const bool_t raster)
{
    Gdiplus::Bitmap *bitmap = NULL;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    cassert_unref(raster == FALSE, raster);
    bitmap = cast(osimage_native(image), Gdiplus::Bitmap);
    i_set_gdiplus_mode(ctx);

    if (frame_index != UINT32_MAX)
    {
        Gdiplus::Status status = bitmap->SelectActiveFrame(&Gdiplus::FrameDimensionTime, (UINT)frame_index);
        cassert_unref(status == Gdiplus::Ok, status);
    }

    Gdiplus::REAL lx = (Gdiplus::REAL)x;
    Gdiplus::REAL ly = (Gdiplus::REAL)y;
    Gdiplus::REAL width = (Gdiplus::REAL)bitmap->GetWidth();
    Gdiplus::REAL height = (Gdiplus::REAL)bitmap->GetHeight();

    switch (ctx->image_halign)
    {
    case ekLEFT:
    case ekJUSTIFY:
        break;
    case ekCENTER:
        lx -= width / 2;
        break;
    case ekRIGHT:
        lx -= width;
        break;
    default:
        cassert_default(ctx->image_halign);
    }

    switch (ctx->image_valign)
    {
    case ekTOP:
    case ekJUSTIFY:
        break;
    case ekCENTER:
        ly -= height / 2;
        break;
    case ekBOTTOM:
        ly -= height;
        break;
    default:
        cassert_default(ctx->image_valign);
    }

    ctx->graphics->DrawImage(bitmap, lx, ly, width, height);
}

/*---------------------------------------------------------------------------*/

Gdiplus::ColorPalette *_dctx_8bpp_grayscale_palette(void)
{
    if (i_kGRAY8_PALETTE == NULL)
    {
        uint32_t i = 0;
        i_kGRAY8_PALETTE = cast(heap_malloc(sizeof(Gdiplus::ColorPalette) + 256 * sizeof(Gdiplus::ARGB), "Gray8Palette"), Gdiplus::ColorPalette);
        i_kGRAY8_PALETTE->Flags = Gdiplus::PaletteFlagsGrayScale;
        i_kGRAY8_PALETTE->Count = 256;
        for (i = 0; i < 256; ++i)
            i_kGRAY8_PALETTE->Entries[i] = Gdiplus::Color::MakeARGB(255, (BYTE)i, (BYTE)i, (BYTE)i);
    }

    return i_kGRAY8_PALETTE;
}

/*---------------------------------------------------------------------------*/

void draw_line(DCtx *ctx, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1)
{
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    i_set_gdiplus_mode(ctx);
    ctx->graphics->DrawLine(ctx->current_pen, (Gdiplus::REAL)x0, (Gdiplus::REAL)y0, (Gdiplus::REAL)x1, (Gdiplus::REAL)y1);
}

/*---------------------------------------------------------------------------*/

void draw_polyline(DCtx *ctx, bool_t closed, const V2Df *points, const uint32_t n)
{
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    cassert_no_null(points);
    cassert(sizeof(V2Df) == sizeof(Gdiplus::PointF));
    i_set_gdiplus_mode(ctx);
    ctx->graphics->DrawLines(ctx->current_pen, (const Gdiplus::PointF *)points, (INT)n);
    if (closed == TRUE)
        ctx->graphics->DrawLine(ctx->current_pen, (Gdiplus::REAL)points[n - 1].x, (Gdiplus::REAL)points[n - 1].y, (Gdiplus::REAL)points[0].x, (Gdiplus::REAL)points[0].y);
}

/*---------------------------------------------------------------------------*/

void draw_arc(DCtx *ctx, const real32_t x, const real32_t y, const real32_t radius, const real32_t start, const real32_t sweep)
{
    Gdiplus::RectF rect;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    cassert(sizeof(V2Df) == sizeof(Gdiplus::PointF));
    i_set_gdiplus_mode(ctx);
    rect.X = (Gdiplus::REAL)(x - radius);
    rect.Y = (Gdiplus::REAL)(y - radius);
    rect.Width = (Gdiplus::REAL)(radius + radius);
    rect.Height = rect.Width;
    ctx->graphics->DrawArc(ctx->current_pen, rect, (Gdiplus::REAL)(start * kBMATH_RAD2DEGf), (Gdiplus::REAL)(sweep * kBMATH_RAD2DEGf));
}

/*---------------------------------------------------------------------------*/

void draw_bezier(DCtx *ctx, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1, const real32_t x2, const real32_t y2, const real32_t x3, const real32_t y3)
{
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    i_set_gdiplus_mode(ctx);
    ctx->graphics->DrawBezier(ctx->current_pen, (Gdiplus::REAL)x0, (Gdiplus::REAL)y0, (Gdiplus::REAL)x1, (Gdiplus::REAL)y1, (Gdiplus::REAL)x2, (Gdiplus::REAL)y2, (Gdiplus::REAL)x3, (Gdiplus::REAL)y3);
}

/*---------------------------------------------------------------------------*/

static Gdiplus::Color i_color(const color_t c)
{
    uint8_t r, g, b, a;
    color_get_rgba(c, &r, &g, &b, &a);
    return Gdiplus::Color((BYTE)a, (BYTE)r, (BYTE)g, (BYTE)b);
}

/*---------------------------------------------------------------------------*/

static COLORREF i_colorref(const color_t color)
{
    uint8_t r, g, b;
    color_get_rgb(color, &r, &g, &b);
    return RGB(r, g, b);
}

/*---------------------------------------------------------------------------*/

void draw_line_color(DCtx *ctx, const color_t color)
{
    cassert_no_null(ctx);
    if (ctx->line_color != color)
    {
        ctx->pen->SetColor(i_color(color));
        ctx->current_pen = ctx->pen;
        if (ctx->gdi_pen != NULL)
        {
            BOOL ok = DeleteObject(ctx->gdi_pen);
            cassert_unref(ok != 0, ok);
            ctx->gdi_pen = NULL;
        }

        ctx->gdi_pen = CreatePen(PS_SOLID, 1, i_colorref(color));

        if (ctx->gdi_mode == TRUE)
            SelectObject(ctx->hdc, ctx->gdi_pen);

        ctx->line_color = color;
    }
}

/*---------------------------------------------------------------------------*/

void draw_line_fill(DCtx *ctx)
{
    cassert_no_null(ctx);
    if (ctx->fpen == NULL)
    {
        Gdiplus::REAL pattern[16];
        ctx->fpen = new Gdiplus::Pen((Gdiplus::ARGB)Gdiplus::Color::Black);
        ctx->fpen->SetBrush(ctx->current_brush);
        ctx->fpen->SetWidth(ctx->pen->GetWidth());
        ctx->pen->GetDashPattern(pattern, ctx->pen->GetDashPatternCount());
        ctx->fpen->SetDashPattern(pattern, ctx->pen->GetDashPatternCount());
        ctx->fpen->SetDashStyle(ctx->pen->GetDashStyle());
    }

    ctx->current_pen = ctx->fpen;
}

/*---------------------------------------------------------------------------*/

void draw_line_width(DCtx *ctx, const real32_t width)
{
    cassert_no_null(ctx);
    ctx->pen->SetWidth((Gdiplus::REAL)width);
    if (ctx->fpen != NULL)
        ctx->fpen->SetWidth((Gdiplus::REAL)width);
}

/*---------------------------------------------------------------------------*/

static ___INLINE Gdiplus::LineCap i_linecap(const linecap_t cap)
{
    switch (cap)
    {
    case ekLCFLAT:
        return Gdiplus::LineCapFlat;
    case ekLCSQUARE:
        return Gdiplus::LineCapSquare;
    case ekLCROUND:
        return Gdiplus::LineCapRound;
    default:
        cassert_default(cap);
    }

    return Gdiplus::LineCapFlat;
}

/*---------------------------------------------------------------------------*/

void draw_line_cap(DCtx *ctx, const linecap_t cap)
{
    cassert_no_null(ctx);
    ctx->pen->SetLineCap(i_linecap(cap), i_linecap(cap), Gdiplus::DashCapFlat);
}

/*---------------------------------------------------------------------------*/

static ___INLINE Gdiplus::LineJoin i_linejoin(const linejoin_t join)
{
    switch (join)
    {
    case ekLJMITER:
        return Gdiplus::LineJoinMiter;
    case ekLJROUND:
        return Gdiplus::LineJoinRound;
    case ekLJBEVEL:
        return Gdiplus::LineJoinBevel;
    default:
        cassert_default(join);
    }

    return Gdiplus::LineJoinMiter;
}

/*---------------------------------------------------------------------------*/

void draw_line_join(DCtx *ctx, const linejoin_t join)
{
    cassert_no_null(ctx);
    ctx->pen->SetLineJoin(i_linejoin(join));
}

/*---------------------------------------------------------------------------*/

void draw_line_dash(DCtx *ctx, const real32_t *pattern, const uint32_t n)
{
    cassert_no_null(ctx);
    if (pattern != NULL)
    {
        Gdiplus::Status status = ctx->pen->SetDashPattern(cast(pattern, Gdiplus::REAL), (INT)n);
        ctx->pen->SetDashStyle(Gdiplus::DashStyleCustom);
        cassert_unref(status == Gdiplus::Ok, status);

        if (ctx->fpen != NULL)
        {
            ctx->fpen->SetDashPattern(cast(pattern, Gdiplus::REAL), (INT)n);
            ctx->fpen->SetDashStyle(Gdiplus::DashStyleCustom);
        }
    }
    else
    {
        ctx->pen->SetDashStyle(Gdiplus::DashStyleSolid);
        if (ctx->fpen != NULL)
            ctx->fpen->SetDashStyle(Gdiplus::DashStyleSolid);
    }
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_draw_path(DCtx *ctx, Gdiplus::GraphicsPath *path, const drawop_t op)
{
    cassert_no_null(path);
    i_set_gdiplus_mode(ctx);

    switch (op)
    {
    case ekSTROKE:
        ctx->graphics->DrawPath(ctx->current_pen, path);
        break;
    case ekFILL:
        ctx->graphics->FillPath(ctx->current_brush, path);
        break;
    case ekSKFILL:
        ctx->graphics->DrawPath(ctx->current_pen, path);
        ctx->graphics->FillPath(ctx->current_brush, path);
        break;
    case ekFILLSK:
        ctx->graphics->FillPath(ctx->current_brush, path);
        ctx->graphics->DrawPath(ctx->current_pen, path);
        break;
    default:
        cassert_default(op);
    }
}

/*---------------------------------------------------------------------------*/

void draw_rect(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t width, const real32_t height)
{
    Gdiplus::GraphicsPath path;
    Gdiplus::REAL x0, x1, y0, y1;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    x0 = (Gdiplus::REAL)x;
    x1 = (Gdiplus::REAL)(x + width);
    y0 = (Gdiplus::REAL)y;
    y1 = (Gdiplus::REAL)(y + height);
    path.AddLine(x0, y0, x1, y0);
    path.AddLine(x1, y0, x1, y1);
    path.AddLine(x1, y1, x0, y1);
    path.CloseFigure();
    i_draw_path(ctx, &path, op);
}

/*---------------------------------------------------------------------------*/

void draw_rndrect(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const real32_t radius)
{
    Gdiplus::GraphicsPath path;
    Gdiplus::REAL radi2 = radius * 2.f;
    Gdiplus::REAL x1 = x + radius;
    Gdiplus::REAL x2 = x + width - radi2;
    Gdiplus::REAL x3 = x + width;
    Gdiplus::REAL y1 = y + radius;
    Gdiplus::REAL y2 = y + height - radi2;
    Gdiplus::REAL y3 = y + height;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    path.AddLine(x1, y, x2, y);
    path.AddArc(x2, y, radi2, radi2, 270.f, 90.f);
    path.AddLine(x3, y1, x3, y2);
    path.AddArc(x2, y2, radi2, radi2, 0.f, 90.f);
    path.AddLine(x2, y3, x1, y3);
    path.AddArc(x, y2, radi2, radi2, 90.f, 90.f);
    path.AddLine(x, y2, x, y1);
    path.AddArc(x, y, radi2, radi2, 180.f, 90.f);
    path.CloseFigure();
    i_draw_path(ctx, &path, op);
}

/*---------------------------------------------------------------------------*/

void draw_circle(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t radius)
{
    Gdiplus::GraphicsPath path;
    Gdiplus::RectF rect;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    rect.X = (Gdiplus::REAL)(x - radius);
    rect.Y = (Gdiplus::REAL)(y - radius);
    rect.Width = (Gdiplus::REAL)(radius + radius);
    rect.Height = rect.Width;
    path.AddEllipse(rect);
    i_draw_path(ctx, &path, op);
}

/*---------------------------------------------------------------------------*/

void draw_ellipse(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t radx, const real32_t rady)
{
    Gdiplus::GraphicsPath path;
    Gdiplus::RectF rect;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    rect.X = (Gdiplus::REAL)(x - radx);
    rect.Y = (Gdiplus::REAL)(y - rady);
    rect.Width = (Gdiplus::REAL)(radx + radx);
    rect.Height = (Gdiplus::REAL)(rady + rady);
    path.AddEllipse(rect);
    i_draw_path(ctx, &path, op);
}

/*---------------------------------------------------------------------------*/

void draw_polygon(DCtx *ctx, const drawop_t op, const V2Df *points, const uint32_t n)
{
    Gdiplus::GraphicsPath path;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    cassert_no_null(points);
    cassert(sizeof(V2Df) == sizeof(Gdiplus::PointF));
    path.AddLines(cast(points, const Gdiplus::PointF), (INT)n);
    path.CloseFigure();
    i_draw_path(ctx, &path, op);
}

/*---------------------------------------------------------------------------*/

void draw_fill_color(DCtx *ctx, const color_t color)
{
    cassert_no_null(ctx);
    if (ctx->fill_color != color)
    {
        Gdiplus::Color c = i_color(color);
        ctx->sbrush->SetColor(c);

        if (ctx->gdi_sbrush != NULL)
        {
            BOOL ok = DeleteObject(ctx->gdi_sbrush);
            cassert_unref(ok != 0, ok);
        }

        ctx->gdi_sbrush = CreateSolidBrush(c.ToCOLORREF());
        ctx->fill_color = color;
    }

    ctx->current_brush = ctx->sbrush;

    if (ctx->fpen != NULL)
        ctx->fpen->SetBrush(ctx->current_brush);
}

/*---------------------------------------------------------------------------*/

static void i_set_gradient_colors(DCtx *ctx)
{
    cassert_no_null(ctx);
    if (ctx->gradient_wrap == Gdiplus::WrapModeClamp)
    {
        /* |---10---|1|---10---| */
        /* If change dimensions, change sc, len in '_dctx_gradient_transform' */
        Gdiplus::REAL st = 10.f / 21.f;
        Gdiplus::REAL norm = 1.f / 21.f;
        Gdiplus::REAL stops[18];
        cassert(ctx->gradient_n <= 16);
        stops[0] = 0;

        for (INT i = 0; i < ctx->gradient_n; ++i)
            stops[i + 1] = st + (ctx->gradient_stops[i + 1] * norm);

        stops[ctx->gradient_n + 1] = 1;

        ctx->lbrush->SetWrapMode(Gdiplus::WrapModeTileFlipX);
        ctx->lbrush->SetInterpolationColors(ctx->gradient_colors, stops, ctx->gradient_n + 2);
    }
    else
    {
        ctx->lbrush->SetWrapMode(ctx->gradient_wrap);
        ctx->lbrush->SetInterpolationColors(ctx->gradient_colors + 1, ctx->gradient_stops + 1, ctx->gradient_n);
    }
}

/*---------------------------------------------------------------------------*/

void draw_fill_linear(DCtx *ctx, const color_t *color, const real32_t *stop, const uint32_t n, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1)
{
    V2Df v;
    uint32_t i;
    cassert_no_null(ctx);
    cassert(n < 16);
    v.x = x1 - x0;
    v.y = y1 - y0;
    ctx->gradient_x = (Gdiplus::REAL)x0;
    ctx->gradient_y = (Gdiplus::REAL)y0;
    ctx->gradient_scale = (Gdiplus::REAL)v2d_lengthf(&v);
    ctx->gradient_angle = (Gdiplus::REAL)bmath_atan2f(v.y, v.x);
    ctx->gradient_angle *= kBMATH_RAD2DEGf;
    ctx->gradient_colors[0] = i_color(color[0]);
    ctx->gradient_stops[0] = 0;

    for (i = 0; i < n; ++i)
    {
        ctx->gradient_colors[i + 1] = i_color(color[i]);
        ctx->gradient_stops[i + 1] = (Gdiplus::REAL)stop[i];
    }

    ctx->gradient_colors[n + 1] = i_color(color[n - 1]);
    ctx->gradient_stops[n + 1] = 1;
    ctx->gradient_n = (INT)n;
    i_set_gradient_colors(ctx);
    ctx->current_brush = ctx->lbrush;
    _dctx_gradient_transform(ctx);

    if (ctx->fpen != NULL)
        ctx->fpen->SetBrush(ctx->current_brush);
}

/*---------------------------------------------------------------------------*/

void draw_fill_matrix(DCtx *ctx, const T2Df *t2d)
{
    cassert_no_null(ctx);
    ctx->gradient_matrix->SetElements(
        (Gdiplus::REAL)t2d->i.x,
        (Gdiplus::REAL)t2d->i.y,
        (Gdiplus::REAL)t2d->j.x,
        (Gdiplus::REAL)t2d->j.y,
        (Gdiplus::REAL)t2d->p.x,
        (Gdiplus::REAL)t2d->p.y);

    _dctx_gradient_transform(ctx);

    if (ctx->fpen != NULL)
        ctx->fpen->SetBrush(ctx->current_brush);
}

/*---------------------------------------------------------------------------*/

static ___INLINE Gdiplus::WrapMode i_wrap(const fillwrap_t wrap)
{
    switch (wrap)
    {
    case ekFCLAMP:
        return Gdiplus::WrapModeClamp;
    case ekFTILE:
        return Gdiplus::WrapModeTile;
    case ekFFLIP:
        return Gdiplus::WrapModeTileFlipX;
    default:
        cassert_default(wrap);
    }

    return Gdiplus::WrapModeClamp;
}

/*---------------------------------------------------------------------------*/

void draw_fill_wrap(DCtx *ctx, const fillwrap_t wrap)
{
    cassert_no_null(ctx);
    ctx->gradient_wrap = i_wrap(wrap);
    i_set_gradient_colors(ctx);
    _dctx_gradient_transform(ctx);

    if (ctx->fpen != NULL)
        ctx->fpen->SetBrush(ctx->current_brush);
}

/*---------------------------------------------------------------------------*/

static void i_font(const Font *font, Gdiplus::Font **ffont, Gdiplus::FontFamily **ffamily, INT *fstyle, Gdiplus::REAL *fsize, Gdiplus::REAL *fintleading)
{
    const char_t *family = NULL;
    WCHAR wfamily[128];
    uint32_t style = 0;
    INT lstyle = 0;
    Gdiplus::Unit unit = Gdiplus::UnitPixel;

    family = font_family(font);
    unicode_convers(family, cast(wfamily, char_t), ekUTF8, ekUTF16, sizeof(wfamily));

    style = font_style(font);
    if (style & ekFBOLD)
        lstyle |= Gdiplus::FontStyleBold;
    if (style & ekFITALIC)
        lstyle |= Gdiplus::FontStyleItalic;
    if (style & ekFSTRIKEOUT)
        lstyle |= Gdiplus::FontStyleStrikeout;
    if (style & ekFUNDERLINE)
        lstyle |= Gdiplus::FontStyleUnderline;
    if (style & ekFPOINTS)
        unit = Gdiplus::UnitPoint;

    if (*ffamily != NULL)
        delete *ffamily;

    *ffamily = new Gdiplus::FontFamily(wfamily);
    *fstyle = lstyle;
    *fsize = (Gdiplus::REAL)font_size(font);

    if (style & ekFCELL)
    {
        *fintleading = (Gdiplus::REAL)font_leading(font);
        *fsize -= *fintleading;
    }
    else
    {
        *fintleading = 0;
    }

    if (*ffont != NULL)
        delete *ffont;

    /*
     * Careful creating GDI+ fonts from HFONT
     * Pure GDI fonts don't allow fonts with fsize < 1
     */
    *ffont = new Gdiplus::Font(*ffamily, *fsize, *fstyle, unit);
}

/*---------------------------------------------------------------------------*/

void draw_font(DCtx *ctx, const Font *font)
{
    cassert_no_null(ctx);
    if (ctx->font == NULL)
    {
        ctx->font = font_copy(font);
        i_font(ctx->font, &ctx->ffont, &ctx->ffamily, &ctx->fstyle, &ctx->fsize, &ctx->fintleading);
    }
    else if (font_equals(font, ctx->font) == FALSE)
    {
        font_destroy(&ctx->font);
        ctx->font = font_copy(font);
        i_font(ctx->font, &ctx->ffont, &ctx->ffamily, &ctx->fstyle, &ctx->fsize, &ctx->fintleading);
    }

    if (ctx->gdi_mode == TRUE)
    {
        cassert_no_null(ctx->hdc);
        SelectObject(ctx->hdc, (HFONT)font_native(ctx->font));
    }
}

/*---------------------------------------------------------------------------*/

static Gdiplus::StringAlignment i_align(const align_t align)
{
    switch (align)
    {
    case ekLEFT:
    case ekJUSTIFY:
        return Gdiplus::StringAlignmentNear;
    case ekCENTER:
        return Gdiplus::StringAlignmentCenter;
    case ekRIGHT:
        return Gdiplus::StringAlignmentFar;
    default:
        cassert_default(align);
    }

    return Gdiplus::StringAlignmentNear;
}

/*---------------------------------------------------------------------------*/

void draw_text_color(DCtx *ctx, const color_t color)
{
    Gdiplus::Color c = i_color(color);
    cassert_no_null(ctx);
    ctx->text_color = color;
    ctx->tbrush->SetColor(c);
    SetTextColor(ctx->hdc, c.ToCOLORREF());
}

/*---------------------------------------------------------------------------*/

static Gdiplus::RectF i_text_origin(DCtx *ctx, const WCHAR *wtext, const real32_t x, const real32_t y)
{
    Gdiplus::PointF origin;
    Gdiplus::SizeF size;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    origin.X = (Gdiplus::REAL)x;
    origin.Y = (Gdiplus::REAL)y;

    if (ctx->text_width >= 0)
        size.Width = ctx->text_width;
    else
        size.Width = 1e8f;

    size.Height = 1e8f;

    if (ctx->text_halign != ekLEFT || ctx->text_valign != ekTOP || ctx->text_intalign != ekLEFT || ctx->text_width >= 0)
    {
        Gdiplus::RectF layout;
        Gdiplus::RectF out;
        real32_t xscale = font_xscale(ctx->font);
        bool_t trimmed = FALSE;
        layout.X = 0;
        layout.Y = 0;
        layout.Height = 1e8f;

        if (ctx->text_width >= 0 && ctx->text_ellipsis == ekELLIPMLINE)
            layout.Width = (Gdiplus::REAL)(ctx->text_width / xscale);
        else
            layout.Width = 1e8f;

        ctx->graphics->MeasureString(wtext, -1, ctx->ffont, layout, &out);

        if (ctx->text_width >= 0 && ctx->text_ellipsis != ekELLIPMLINE)
            out.Width = ctx->text_width / xscale;

        out.Width *= xscale;
        size.Width = out.Width;
        size.Height = out.Height;

        if (ctx->text_width >= 0)
            trimmed = TRUE;

        /*
         * Don 't try to make sense of it. This has been set up by testing all cases of trimmed,
         * single-line, and multi-line text with all possible internal alignments.
         * Don' t touch it !
         */
        switch (ctx->text_halign)
        {
        case ekLEFT:
        case ekJUSTIFY:
            switch (ctx->text_intalign)
            {
            case ekLEFT:
            case ekJUSTIFY:
                break;

            case ekCENTER:
                if (trimmed == FALSE)
                    origin.X += out.Width / 2;
                break;

            case ekRIGHT:
                if (trimmed == FALSE)
                    origin.X += out.Width;
                break;

            default:
                cassert_default(ctx->text_intalign);
            }
            break;

        case ekCENTER:
            switch (ctx->text_intalign)
            {
            case ekLEFT:
            case ekJUSTIFY:
                origin.X -= out.Width / 2;
                break;

            case ekCENTER:
                if (trimmed == TRUE)
                    origin.X -= out.Width / 2;
                break;

            case ekRIGHT:
                if (trimmed == TRUE)
                    origin.X -= out.Width / 2;
                else
                    origin.X += out.Width / 2;
                break;

            default:
                cassert_default(ctx->text_intalign);
            }
            break;

        case ekRIGHT:
            switch (ctx->text_intalign)
            {
            case ekLEFT:
            case ekJUSTIFY:
                origin.X -= out.Width;
                break;

            case ekCENTER:
                if (trimmed == TRUE)
                    origin.X -= out.Width;
                else
                    origin.X -= out.Width / 2;
                break;

            case ekRIGHT:
                if (trimmed == TRUE)
                    origin.X -= out.Width;
                break;

            default:
                cassert_default(ctx->text_intalign);
            }
            break;

        default:
            cassert_default(ctx->text_halign);
        }

        switch (ctx->text_valign)
        {
        case ekTOP:
        case ekJUSTIFY:
            break;
        case ekCENTER:
            origin.Y -= out.Height / 2;
            break;
        case ekBOTTOM:
            origin.Y -= out.Height;
            break;
        default:
            cassert_default(ctx->text_valign);
        }
    }

    return Gdiplus::RectF(origin, size);
}

/*---------------------------------------------------------------------------*/

static void i_draw_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y, const drawop_t op, Gdiplus::Brush *brush)
{
    WString str;
    const WCHAR *wtext = wstring_init(text, &str);
    Gdiplus::RectF rect;
    Gdiplus::Matrix matrix;
    real32_t xscale = 1;
    Gdiplus::StringFormat format;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    i_set_gdiplus_mode(ctx);

    rect = i_text_origin(ctx, wtext, x, y);
    xscale = font_xscale(ctx->font);

    if (bmath_absf(xscale - 1) > 0.01f)
    {
        ctx->graphics->GetTransform(&matrix);
        ctx->graphics->TranslateTransform(rect.X, rect.Y);
        ctx->graphics->ScaleTransform((Gdiplus::REAL)xscale, 1);
        ctx->graphics->TranslateTransform(-rect.X, -rect.Y);
    }

    format.SetAlignment(i_align(ctx->text_intalign));

    if (ctx->text_width >= 0)
    {
        Gdiplus::RectF erect = rect;
        erect.Width /= xscale;
        switch (ctx->text_ellipsis)
        {
        case ekELLIPBEGIN:
        case ekELLIPMIDDLE:
            format.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
            break;
        case ekELLIPEND:
            format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
            break;
        case ekELLIPNONE:
        case ekELLIPMLINE:
            format.SetTrimming(Gdiplus::StringTrimmingNone);
            break;
        default:
            cassert_default(ctx->text_ellipsis);
        }

        if (brush != NULL)
        {
            cassert(op == ekFILL);
            ctx->graphics->DrawString(wtext, -1, ctx->ffont, erect, &format, brush);
        }
        else
        {
            /* If we have stroke text --> Use a path */
            Gdiplus::GraphicsPath path;
            Gdiplus::REAL size = ctx->fsize - ctx->fintleading;
            cassert(op != ekFILL);
            path.AddString(wtext, -1, ctx->ffamily, ctx->fstyle, size, erect, &format);
            i_draw_path(ctx, &path, op);
        }
    }
    else
    {
        if (brush != NULL)
        {
            ctx->graphics->DrawString(wtext, -1, ctx->ffont, Gdiplus::PointF(rect.X, rect.Y), &format, brush);
        }
        else
        {
            /* If we have stroke text --> Use a path */
            Gdiplus::GraphicsPath path;
            Gdiplus::REAL size = ctx->fsize - ctx->fintleading;
            cassert(op != ekFILL);
            path.AddString(wtext, -1, ctx->ffamily, ctx->fstyle, size, Gdiplus::PointF(rect.X, rect.Y), &format);
            i_draw_path(ctx, &path, op);
        }
    }

    if (bmath_absf(xscale - 1) > 0.01f)
        ctx->graphics->SetTransform(&matrix);

    wstring_remove(&str);
}

/*---------------------------------------------------------------------------*/

void draw_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y)
{
    cassert_no_null(ctx);
    i_draw_text(ctx, text, x, y, ekFILL, ctx->tbrush);
}

/*---------------------------------------------------------------------------*/

void draw_text_path(DCtx *ctx, const drawop_t op, const char_t *text, const real32_t x, const real32_t y)
{
    cassert_no_null(ctx);
    i_draw_text(ctx, text, x, y, op, op == ekFILL ? ctx->current_brush : NULL);
}

/*---------------------------------------------------------------------------*/

void draw_text_width(DCtx *ctx, const real32_t width)
{
    cassert_no_null(ctx);
    ctx->text_width = width;
}

/*---------------------------------------------------------------------------*/

void draw_text_trim(DCtx *ctx, const ellipsis_t ellipsis)
{
    cassert_no_null(ctx);
    ctx->text_ellipsis = ellipsis;
}

/*---------------------------------------------------------------------------*/

void draw_text_align(DCtx *ctx, const align_t halign, const align_t valign)
{
    cassert_no_null(ctx);
    ctx->text_halign = halign;
    ctx->text_valign = valign;
}

/*---------------------------------------------------------------------------*/

void draw_text_halign(DCtx *ctx, const align_t halign)
{
    cassert_no_null(ctx);
    ctx->text_intalign = halign;
}

/*---------------------------------------------------------------------------*/

void draw_text_extents(DCtx *ctx, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    /*
     * GDI+ render text slightly different than GDI, event using the same font.
     * For this reason, we can't use font_extents() in Windows (based on GDI 'DrawText(DT_CALCRECT)').
     * We must use 'MeasureString' that is the correct way to do it in GDI+.
     */
    WString str;
    const WCHAR *wtext = wstring_init(text, &str);
    real32_t xscale = 1.f;
    Gdiplus::RectF layout;
    Gdiplus::RectF out;
    cassert_no_null(ctx);
    cassert_no_null(ctx->graphics);
    i_set_gdiplus_mode(ctx);
    xscale = font_xscale(ctx->font);
    layout.X = 0;
    layout.Y = 0;
    layout.Width = (Gdiplus::REAL)(refwidth > 0 ? refwidth / xscale : 1e8);
    layout.Height = 1e8;
    ctx->graphics->MeasureString(wtext, -1, ctx->ffont, layout, &out);
    *width = bmath_ceilf((real32_t)out.Width * xscale);
    *height = bmath_ceilf((real32_t)out.Height);
    wstring_remove(&str);
}

/*---------------------------------------------------------------------------*/

void draw_image_align(DCtx *ctx, const align_t halign, const align_t valign)
{
    cassert_no_null(ctx);
    ctx->image_halign = halign;
    ctx->image_valign = valign;
}

/*---------------------------------------------------------------------------*/

static void i_set_gdi_mode(DCtx *ctx)
{
    cassert_no_null(ctx);
    if (ctx->gdi_mode == FALSE)
    {
        cassert_no_null(ctx->hdc);
        SetBkMode(ctx->hdc, TRANSPARENT);
        if (ctx->font != NULL)
            SelectObject(ctx->hdc, (HFONT)font_native(ctx->font));
        if (ctx->gdi_pen != NULL)
            SelectObject(ctx->hdc, ctx->gdi_pen);
        ctx->gdi_mode = TRUE;
    }
}

/*---------------------------------------------------------------------------*/

void draw_set_raster_mode(DCtx *ctx)
{
    i_set_gdi_mode(ctx);
}

/*---------------------------------------------------------------------------*/

void draw_text_raster(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y)
{
    unref(ctx);
    unref(text);
    unref(x);
    unref(y);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void draw_image_raster(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y)
{
    unref(ctx);
    unref(image);
    unref(x);
    unref(y);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void draw_line_imp(DCtx *ctx, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1, const bool_t raster)
{
    unref(ctx);
    unref(x0);
    unref(y0);
    unref(x1);
    unref(y1);
    unref(raster);
    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

void draw_rect_imp(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const bool_t raster)
{
    unref(ctx);
    unref(op);
    unref(x);
    unref(y);
    unref(width);
    unref(height);
    unref(raster);
    cassert(FALSE);
}
