/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: r2d.hpp
 *
 */

/* 2D rectangles */

#ifndef __R2D_HPP__
#define __R2D_HPP__

#include "v2d.hpp"
#include "s2d.hpp"

template<typename real>
struct R2D
{
    R2D();

    R2D(const real x, const real y, const real width, const real height);

    static bool_t(*collide)(const R2D *r2d1, const R2D *r2d2);

    static bool_t(*contains)(const R2D *r2d, const real x, const real y);

    static bool_t(*clip)(const R2D *viewport, const R2D *r2d);

    static void(*join)(R2D *r2d, const R2D *src);

    static const R2D<real> *kZERO;

    V2D<real> pos;
    S2D<real> size;
};

#endif

