/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nsvg.c
 *
 */

/* NanoSVG support */

#include "nsvg.h"
#include <draw2d/color.h>
#include <draw2d/dctx.h>
#include <draw2d/draw.h>
#include <draw2d/pixbuf.h>
#include <core/heap.h>
#include <geom2d/t2d.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/nowarn.hxx>

#define NANOSVG_IMPLEMENTATION
#include "depend/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "depend/nanosvgrast.h"
#include <sewer/warn.hxx>

struct _nsvg_t
{
    NSVGimage *image;
    real32_t width;
    real32_t height;
};

/*---------------------------------------------------------------------------*/

static bool_t i_scale_params(const real32_t svg_width, const real32_t svg_height, const uint32_t width, const uint32_t height, uint32_t *raster_width, uint32_t *raster_height, real32_t *scale, real32_t *tx, real32_t *ty)
{
    bool_t has_width = (bool_t)(width != UINT32_MAX);
    bool_t has_height = (bool_t)(height != UINT32_MAX);
    uint32_t target_width = 0;
    uint32_t target_height = 0;
    real32_t lscale = 1.f;
    real32_t sx = 1.f;
    real32_t sy = 1.f;
    real32_t content_width = 0.f;
    real32_t content_height = 0.f;

    cassert_no_null(raster_width);
    cassert_no_null(raster_height);
    cassert_no_null(scale);
    cassert_no_null(tx);
    cassert_no_null(ty);

    if (svg_width <= 0.f || svg_height <= 0.f)
        return FALSE;

    if (has_width == FALSE && has_height == FALSE)
    {
        target_width = (uint32_t)(svg_width + .5f);
        target_height = (uint32_t)(svg_height + .5f);
    }
    else if (has_width == TRUE && has_height == FALSE)
    {
        target_width = width > 0 ? width : 1;
        target_height = (uint32_t)(((svg_height * (real32_t)target_width) / svg_width) + .5f);
    }
    else if (has_width == FALSE && has_height == TRUE)
    {
        target_height = height > 0 ? height : 1;
        target_width = (uint32_t)(((svg_width * (real32_t)target_height) / svg_height) + .5f);
    }
    else
    {
        target_width = width > 0 ? width : 1;
        target_height = height > 0 ? height : 1;
    }

    if (target_width == 0)
        target_width = 1;
    if (target_height == 0)
        target_height = 1;

    sx = (real32_t)target_width / svg_width;
    sy = (real32_t)target_height / svg_height;

    if (has_width == TRUE && has_height == TRUE)
        lscale = sx < sy ? sx : sy;
    else if (has_width == TRUE)
        lscale = sx;
    else if (has_height == TRUE)
        lscale = sy;
    else
        lscale = 1.f;

    if (lscale <= 0.f)
        return FALSE;

    content_width = svg_width * lscale;
    content_height = svg_height * lscale;

    *raster_width = target_width;
    *raster_height = target_height;
    *scale = lscale;
    *tx = ((real32_t)target_width - content_width) * .5f;
    *ty = ((real32_t)target_height - content_height) * .5f;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static color_t i_color(const unsigned int rgba)
{
    return color_rgba((uint8_t)(rgba & 0xFF), (uint8_t)((rgba >> 8) & 0xFF), (uint8_t)((rgba >> 16) & 0xFF), (uint8_t)((rgba >> 24) & 0xFF));
}

/*---------------------------------------------------------------------------*/

static fillwrap_t i_fill_wrap(const char spread)
{
    switch (spread)
    {
    case NSVG_SPREAD_REFLECT:
        return ekFFLIP;
    case NSVG_SPREAD_REPEAT:
        return ekFTILE;
    case NSVG_SPREAD_PAD:
    default:
        return ekFCLAMP;
    }
}

/*---------------------------------------------------------------------------*/

static linecap_t i_line_cap(const char cap)
{
    switch (cap)
    {
    case NSVG_CAP_ROUND:
        return ekLCROUND;
    case NSVG_CAP_SQUARE:
        return ekLCSQUARE;
    case NSVG_CAP_BUTT:
    default:
        return ekLCFLAT;
    }
}

/*---------------------------------------------------------------------------*/

static linejoin_t i_line_join(const char join)
{
    switch (join)
    {
    case NSVG_JOIN_ROUND:
        return ekLJROUND;
    case NSVG_JOIN_BEVEL:
        return ekLJBEVEL;
    case NSVG_JOIN_MITER:
    default:
        return ekLJMITER;
    }
}

/*---------------------------------------------------------------------------*/

static fillrule_t i_fill_rule(const char rule)
{
    switch (rule)
    {
    case NSVG_FILLRULE_EVENODD:
        return ekFILLEVENODD;
    case NSVG_FILLRULE_NONZERO:
    default:
        return ekFILLNONZERO;
    }
}

/*---------------------------------------------------------------------------*/

static void i_gradient_matrix(const float *xform, T2Df *t2d)
{
    T2Df inv;
    real32_t det;
    cassert_no_null(xform);
    cassert_no_null(t2d);
    inv.i.x = xform[0];
    inv.i.y = xform[1];
    inv.j.x = xform[2];
    inv.j.y = xform[3];
    inv.p.x = xform[4];
    inv.p.y = xform[5];
    det = (inv.i.x * inv.j.y) - (inv.j.x * inv.i.y);

    /* NanoSVG can leave degenerate gradient transforms non-inverted. */
    if (det > -1e-6f && det < 1e-6f)
        *t2d = inv;
    else
        t2d_inversef(t2d, &inv);
}

/*---------------------------------------------------------------------------*/

static void i_set_gradient(DCtx *ctx, const NSVGpaint *paint, const real32_t opacity)
{
    color_t *colors = NULL;
    real32_t *stop = NULL;
    const NSVGgradient *gradient = paint->gradient;
    T2Df matrix;
    int i;

    cassert_no_null(ctx);
    cassert_no_null(paint);
    cassert_no_null(gradient);
    cassert(gradient->nstops > 0);

    colors = cast(heap_malloc((uint32_t)gradient->nstops * sizeof(color_t), "NSVGColors"), color_t);
    stop = cast(heap_malloc((uint32_t)gradient->nstops * sizeof(real32_t), "NSVGStops"), real32_t);

    for (i = 0; i < gradient->nstops; ++i)
    {
        colors[i] = i_color(nsvg__applyOpacity(gradient->stops[i].color, opacity));
        stop[i] = (real32_t)gradient->stops[i].offset;
    }

    draw_fill_wrap(ctx, i_fill_wrap(gradient->spread));

    if (paint->type == NSVG_PAINT_LINEAR_GRADIENT)
    {
        draw_fill_linear(ctx, colors, stop, (uint32_t)gradient->nstops, 0.f, 0.f, 0.f, 1.f);
    }
    else
    {
        cassert(paint->type == NSVG_PAINT_RADIAL_GRADIENT);
        draw_fill_radial(ctx, colors, stop, (uint32_t)gradient->nstops, gradient->fx, gradient->fy, 0.f, 0.f, 0.f, 1.f);
    }

    i_gradient_matrix(gradient->xform, &matrix);
    draw_fill_matrix(ctx, &matrix);

    heap_free(dcast(&stop, byte_t), (uint32_t)gradient->nstops * sizeof(real32_t), "NSVGStops");
    heap_free(dcast(&colors, byte_t), (uint32_t)gradient->nstops * sizeof(color_t), "NSVGColors");
}

/*---------------------------------------------------------------------------*/

static void i_set_fill_paint(DCtx *ctx, const NSVGpaint *paint, const real32_t opacity)
{
    cassert_no_null(ctx);
    cassert_no_null(paint);

    if (paint->type == NSVG_PAINT_COLOR)
    {
        draw_fill_color(ctx, i_color(nsvg__applyOpacity(paint->color, opacity)));
    }
    else
    {
        cassert(paint->type == NSVG_PAINT_LINEAR_GRADIENT || paint->type == NSVG_PAINT_RADIAL_GRADIENT);
        i_set_gradient(ctx, paint, opacity);
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_stroke_paint(DCtx *ctx, const NSVGpaint *paint, const real32_t opacity)
{
    cassert_no_null(ctx);
    cassert_no_null(paint);

    if (paint->type == NSVG_PAINT_COLOR)
    {
        draw_line_color(ctx, i_color(nsvg__applyOpacity(paint->color, opacity)));
    }
    else
    {
        cassert(paint->type == NSVG_PAINT_LINEAR_GRADIENT || paint->type == NSVG_PAINT_RADIAL_GRADIENT);
        i_set_gradient(ctx, paint, opacity);
        draw_line_fill(ctx);
    }
}

/*---------------------------------------------------------------------------*/

static void i_add_paths(DCtx *ctx, const NSVGshape *shape)
{
    const NSVGpath *path = NULL;

    cassert_no_null(ctx);
    cassert_no_null(shape);

    draw_path_begin(ctx);

    for (path = shape->paths; path != NULL; path = path->next)
    {
        int j = 0;
        cassert(path->npts > 0);
        draw_path_move(ctx, path->pts[0], path->pts[1]);

        for (j = 0; j < path->npts - 1; j += 3)
        {
            float *p = &path->pts[j * 2];
            draw_path_bezier(ctx, p[2], p[3], p[4], p[5], p[6], p[7]);
        }

        if (path->closed)
            draw_path_close(ctx);
    }
}

/*---------------------------------------------------------------------------*/

static void i_draw_fill_paths(DCtx *ctx, const NSVGshape *shape)
{
    draw_fill_rule(ctx, i_fill_rule(shape->fillRule));
    i_set_fill_paint(ctx, &shape->fill, (real32_t)shape->opacity);
    i_add_paths(ctx, shape);
    draw_path_end(ctx, ekFILL);
}

/*---------------------------------------------------------------------------*/

static void i_draw_stroke_paths(DCtx *ctx, const NSVGshape *shape)
{
    real32_t dash_pattern[8];
    uint32_t i = 0;
    real32_t line_width = (real32_t)shape->strokeWidth;

    draw_line_width(ctx, line_width);
    draw_line_cap(ctx, i_line_cap(shape->strokeLineCap));
    draw_line_join(ctx, i_line_join(shape->strokeLineJoin));
    draw_line_miter_limit(ctx, (real32_t)shape->miterLimit);

    if (shape->strokeDashCount > 0 && line_width > 0.f)
    {
        for (i = 0; i < (uint32_t)shape->strokeDashCount; ++i)
            dash_pattern[i] = (real32_t)shape->strokeDashArray[i] / line_width;

        draw_line_dash(ctx, dash_pattern, (uint32_t)shape->strokeDashCount);
        draw_line_dash_offset(ctx, (real32_t)shape->strokeDashOffset / line_width);
    }
    else
    {
        draw_line_dash(ctx, NULL, 0);
        draw_line_dash_offset(ctx, 0.f);
    }

    i_set_stroke_paint(ctx, &shape->stroke, (real32_t)shape->opacity);
    i_add_paths(ctx, shape);
    draw_path_end(ctx, ekSTROKE);
    draw_line_dash(ctx, NULL, 0);
    draw_line_dash_offset(ctx, 0.f);
}

/*---------------------------------------------------------------------------*/

NSVG *nsvg_from_data(const byte_t *data, const uint32_t size)
{
    NSVG *svg = NULL;
    char_t *svg_data = NULL;

    cassert_no_null(data);

    if (size == 0)
        return NULL;

    svg_data = cast(heap_malloc(size + 1, "NSVGData"), char_t);
    bmem_copy(cast(svg_data, byte_t), data, size);
    svg_data[size] = '\0';

    svg = heap_new0(NSVG);
    svg->image = nsvgParse(svg_data, "px", 96.f);
    heap_free(dcast(&svg_data, byte_t), size + 1, "NSVGData");

    if (svg->image == NULL)
    {
        heap_delete(&svg, NSVG);
        return NULL;
    }

    svg->width = (real32_t)svg->image->width;
    svg->height = (real32_t)svg->image->height;
    return svg;
}

/*---------------------------------------------------------------------------*/

void nsvg_destroy(NSVG **svg)
{
    cassert_no_null(svg);
    cassert_no_null(*svg);
    if ((*svg)->image != NULL)
        nsvgDelete((*svg)->image);
    heap_delete(svg, NSVG);
}

/*---------------------------------------------------------------------------*/

Pixbuf *nsvg_pixbuf(const NSVG *svg, const uint32_t width, const uint32_t height, const pixformat_t format)
{
    Pixbuf *pixbuf = NULL;
    Pixbuf *converted = NULL;
    NSVGrasterizer *rasterizer = NULL;
    uint32_t raster_width = 0;
    uint32_t raster_height = 0;
    real32_t scale = 1.f;
    real32_t tx = 0.f;
    real32_t ty = 0.f;

    cassert_no_null(svg);

    if (format != ekRGBA32 && format != ekRGB24 && format != ekGRAY8)
        return NULL;

    if (i_scale_params(svg->width, svg->height, width, height, &raster_width, &raster_height, &scale, &tx, &ty) == FALSE)
        return NULL;

    pixbuf = pixbuf_create(raster_width, raster_height, ekRGBA32);
    rasterizer = nsvgCreateRasterizer();
    if (rasterizer == NULL)
    {
        pixbuf_destroy(&pixbuf);
        return NULL;
    }

    nsvgRasterize(rasterizer, svg->image, tx, ty, scale, pixbuf_data(pixbuf), (int)raster_width, (int)raster_height, (int)(raster_width * 4));

    if (format != ekRGBA32)
    {
        converted = pixbuf_convert(pixbuf, NULL, format);
        pixbuf_destroy(&pixbuf);
        pixbuf = converted;
    }

    if (rasterizer != NULL)
        nsvgDeleteRasterizer(rasterizer);

    return pixbuf;
}

/*---------------------------------------------------------------------------*/

void nsvg_draw(const NSVG *svg, DCtx *ctx)
{
    const NSVGshape *shape = NULL;

    cassert_no_null(svg);
    cassert_no_null(ctx);

    for (shape = svg->image->shapes; shape != NULL; shape = shape->next)
    {
        int i = 0;

        if ((shape->flags & NSVG_FLAGS_VISIBLE) == 0)
            continue;

        for (i = 0; i < 3; ++i)
        {
            unsigned char paint_order = (unsigned char)((shape->paintOrder >> (2 * i)) & 0x03);

            if (paint_order == NSVG_PAINT_FILL && shape->fill.type != NSVG_PAINT_NONE)
                i_draw_fill_paths(ctx, shape);

            if (paint_order == NSVG_PAINT_STROKE && shape->stroke.type != NSVG_PAINT_NONE && shape->strokeWidth > 0.f)
                i_draw_stroke_paths(ctx, shape);
        }
    }
}
