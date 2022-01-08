/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawg.hpp
 *
 */

/* Drawing 2D Geometry entities */

#ifndef __DRAWG_HPP__
#define __DRAWG_HPP__

#include "draw2d.hxx"
#include "cir2d.hpp"
#include "obb2d.hpp"
#include "pol2d.hpp"

template<typename real>
struct Draw
{
    static void (*v2d)(DCtx *ctx, const drawop_t op, const V2D<real> *v2d, const real radius);

    static void (*seg2d)(DCtx *ctx, const Seg2D<real> *seg);

    static void (*cir2d)(DCtx *ctx, const drawop_t op, const Cir2D<real> *cir);

    static void (*box2d)(DCtx *ctx, const drawop_t op, const Box2D<real> *box);

    static void (*obb2d)(DCtx *ctx, const drawop_t op, const OBB2D<real> *obb);

    static void (*tri2d)(DCtx *ctx, const drawop_t op, const Tri2D<real> *tri);

    static void (*pol2d)(DCtx *ctx, const drawop_t op, const Pol2D<real> *pol);
};

#endif

