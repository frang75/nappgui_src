/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx.h
 * https://nappgui.com/en/draw2d/dctx.html
 *
 */

/* Draw context */

#include "draw2d.hxx"

__EXTERN_C

DCtx *dctx_bitmap(const uint32_t width, const uint32_t height, const pixformat_t format);

Image *dctx_image(DCtx **ctx);

void draw_clear(DCtx *ctx, const color_t color);

void draw_matrixf(DCtx *ctx, const T2Df *t2d);

void draw_matrixd(DCtx *ctx, const T2Dd *t2d);

void draw_matrix_cartesianf(DCtx *ctx, const T2Df *t2d);

void draw_matrix_cartesiand(DCtx *ctx, const T2Dd *t2d);

void draw_antialias(DCtx *ctx, const bool_t on);

__END_C

