/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: svg.c
 *
 */

/* SVG image support */

#include "svg.inl"
#include "pixbuf.h"
#include <core/heap.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/nowarn.hxx>

#define NANOSVG_IMPLEMENTATION
#include "depend/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "depend/nanosvgrast.h"
#include <sewer/warn.hxx>

/*---------------------------------------------------------------------------*/

static ___INLINE char_t i_tolower(const char_t ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return (char_t)(ch + ('a' - 'A'));
    return ch;
}

/*---------------------------------------------------------------------------*/

static bool_t i_svg_tag(const byte_t *data, const uint32_t size, const uint32_t start)
{
    uint32_t i = start;
    uint32_t name = start;

    while (i < size)
    {
        byte_t ch = data[i];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n')
            break;
        i += 1;
    }

    if (i >= size)
        return FALSE;

    if (data[i] == '?' || data[i] == '!')
        return FALSE;

    name = i;
    while (i < size)
    {
        byte_t ch = data[i];
        if (ch == ':' || ch == '>' || ch == '/' || ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            break;
        i += 1;
    }

    if (i < size && data[i] == ':')
    {
        name = i + 1;
        i += 1;
        while (i < size)
        {
            byte_t ch = data[i];
            if (ch == '>' || ch == '/' || ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
                break;
            i += 1;
        }
    }

    if (i - name != 3)
        return FALSE;

    return (bool_t)(i_tolower((char_t)data[name]) == 's' && i_tolower((char_t)data[name + 1]) == 'v' && i_tolower((char_t)data[name + 2]) == 'g');
}

/*---------------------------------------------------------------------------*/

bool_t _svg_is_data(const byte_t *data, const uint32_t size)
{
    uint32_t limit, i;
    cassert_no_null(data);

    if (size == 0)
        return FALSE;

    limit = size < 8192 ? size : 8192;
    for (i = 0; i < limit; ++i)
    {
        if (data[i] == '<' && i_svg_tag(data, limit, i + 1) == TRUE)
            return TRUE;
    }

    return FALSE;
}

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
    real32_t content_width = 0;
    real32_t content_height = 0;

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

Pixbuf *_svg_render(const byte_t *data, const uint32_t size, const uint32_t width, const uint32_t height, real32_t *svg_width, real32_t *svg_height)
{
    Pixbuf *pixbuf = NULL;
    char_t *svg_data = NULL;
    NSVGimage *svg = NULL;
    NSVGrasterizer *rasterizer = NULL;
    uint32_t raster_width = 0;
    uint32_t raster_height = 0;
    real32_t scale = 1.f;
    real32_t tx = 0.f;
    real32_t ty = 0.f;
    bool_t ok = FALSE;

    cassert_no_null(data);

    if (size == 0)
        return NULL;

    svg_data = cast(heap_malloc(size + 1, "SvgData"), char_t);
    bmem_copy(cast(svg_data, byte_t), data, size);
    svg_data[size] = '\0';

    svg = nsvgParse(svg_data, "px", 96.f);
    if (svg == NULL)
        goto cleanup;

    ptr_assign(svg_width, (real32_t)svg->width);
    ptr_assign(svg_height, (real32_t)svg->height);

    if (i_scale_params((real32_t)svg->width, (real32_t)svg->height, width, height, &raster_width, &raster_height, &scale, &tx, &ty) == FALSE)
        goto cleanup;

    pixbuf = pixbuf_create(raster_width, raster_height, ekRGBA32);
    rasterizer = nsvgCreateRasterizer();
    if (rasterizer == NULL)
        goto cleanup;

    nsvgRasterize(rasterizer, svg, tx, ty, scale, pixbuf_data(pixbuf), (int)raster_width, (int)raster_height, (int)(raster_width * 4));
    ok = TRUE;

cleanup:

    if (rasterizer != NULL)
        nsvgDeleteRasterizer(rasterizer);

    if (svg != NULL)
        nsvgDelete(svg);

    if (svg_data != NULL)
        heap_free(dcast(&svg_data, byte_t), size + 1, "SvgData");

    if (pixbuf != NULL && ok == FALSE)
        pixbuf_destroy(&pixbuf);

    return pixbuf;
}
