/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ddraw.c
 *
 */

/* Die drawing */

#include "ddraw.h"
#include "draw2dall.h"

/*---------------------------------------------------------------------------*/

static const real32_t i_MAX_PADDING = 0.2f;

/*---------------------------------------------------------------------------*/

void die_draw(DCtx *ctx, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const real32_t padding, const real32_t corner, const real32_t radius, const uint32_t face)
{
    color_t white = color_rgb(255, 255, 255);
    color_t black = color_rgb(0, 0, 0);
    real32_t dsize, dx, dy;
    real32_t rc, rr;
    real32_t p1, p2, p3;

    dsize = width < height ? width : height;    
    dsize -= bmath_floorf(2.f * dsize * padding * i_MAX_PADDING);
    dx = x + .5f * (width - dsize);
    dy = y + .5f * (height - dsize);
    rc = dsize * (.1f + .3f * corner);
    rr = dsize * (.05f + .1f * radius);
    p1 = 0.5f * dsize;
    p2 = 0.2f * dsize;
    p3 = 0.8f * dsize;

    draw_fill_color(ctx, white);
    draw_rndrect(ctx, ekFILL, dx, dy, dsize, dsize, rc);
    draw_fill_color(ctx, black);

    if (face == 1 || face == 3 || face == 5)
        draw_circle(ctx, ekFILL, dx + p1, dy + p1, rr);

    if (face != 1)
    {
        draw_circle(ctx, ekFILL, dx + p3, dy + p2, rr);
        draw_circle(ctx, ekFILL, dx + p2, dy + p3, rr);
    }

    if (face == 4 || face == 5 || face == 6)
    {
        draw_circle(ctx, ekFILL, dx + p2, dy + p2, rr);
        draw_circle(ctx, ekFILL, dx + p3, dy + p3, rr);
    }

    if (face == 6)
    {
        draw_circle(ctx, ekFILL, dx + p2, dy + p1, rr);
        draw_circle(ctx, ekFILL, dx + p3, dy + p1, rr);
    }
}
