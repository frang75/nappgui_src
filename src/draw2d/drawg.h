/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawg.h
 * https://nappgui.com/en/draw2d/drawg.html
 *
 */

/* Drawing 2D Geometry entities */

#include "draw2d.hxx"

__EXTERN_C

void draw_v2df(DCtx *ctx, const drawop_t op, const V2Df *v2d, const real32_t radius);

void draw_v2dd(DCtx *ctx, const drawop_t op, const V2Dd *v2d, const real64_t radius);

void draw_seg2df(DCtx *ctx, const Seg2Df *seg);

void draw_seg2dd(DCtx *ctx, const Seg2Dd *seg);

void draw_cir2df(DCtx *ctx, const drawop_t op, const Cir2Df *cir);

void draw_cir2dd(DCtx *ctx, const drawop_t op, const Cir2Dd *cir);

void draw_box2df(DCtx *ctx, const drawop_t op, const Box2Df *box);

void draw_box2dd(DCtx *ctx, const drawop_t op, const Box2Dd *box);

void draw_obb2df(DCtx *ctx, const drawop_t op, const OBB2Df *obb);

void draw_obb2dd(DCtx *ctx, const drawop_t op, const OBB2Dd *obb);

void draw_tri2df(DCtx *ctx, const drawop_t op, const Tri2Df *tri);

void draw_tri2dd(DCtx *ctx, const drawop_t op, const Tri2Dd *tri);

void draw_pol2df(DCtx *ctx, const drawop_t op, const Pol2Df *pol);

void draw_pol2dd(DCtx *ctx, const drawop_t op, const Pol2Dd *pol);

__END_C

