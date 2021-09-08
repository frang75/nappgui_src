/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: cir2d.hpp
 *
 */

/* Circle 2d */

#ifndef __CIR2D_HPP__
#define __CIR2D_HPP__

#include "box2d.hpp"

template<typename real>
struct Cir2D
{
    Cir2D() {c.x = 0, c.y = 0, r = 0;}

    Cir2D(const real _x, const real _y, const real _r) {c.x = _x, c.y = _y, r = _r;}

    static Cir2D<real> (*from_box)(const Box2D<real> *box);

    static Cir2D<real> (*from_points)(const V2D<real> *p, const uint32_t n);

    static Cir2D<real> (*minimum)(const V2D<real> *p, const uint32_t n);

    static real (*area)(const Cir2D<real> *cir);

    static bool_t (*is_null)(const Cir2D<real> *cir);

    static const Cir2D<real> *kNULL;

    V2D<real> c;
    real r;
};

#endif

