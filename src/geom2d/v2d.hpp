/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: v2d.hpp
 *
 */

/* Vector 2d */

#ifndef __V2D_HPP__
#define __V2D_HPP__

#include "geom2d.hxx"

template<typename real>
struct V2D
{
    V2D() {x = 0, y = 0;}

    V2D(const real _x, const real _y) {x = _x, y = _y;}

    static V2D<real> (*add)(const V2D<real> *v1, const V2D<real> *v2);

    static V2D<real> (*sub)(const V2D<real> *v1, const V2D<real> *v2);

    static V2D<real> (*mul)(const V2D<real> *v, const real s);

    static V2D<real> (*from)(const V2D<real> *v, const V2D<real> *dir, const real length);

    static V2D<real> (*mid)(const V2D<real> *v1, const V2D<real> *v2);

    static V2D<real> (*unit)(const V2D<real> *v1, const V2D<real> *v2, real *dist);

    static V2D<real> (*unit_xy)(const real x1, const real y1, const real x2, const real y2, real *dist);

    static V2D<real> (*perp_pos)(const V2D<real> *v);

    static V2D<real> (*perp_neg)(const V2D<real> *v);

    static V2D<real> (*from_angle)(const real a);

    static bool_t (*norm)(V2D<real> *v);

    static real (*length)(const V2D<real> *v);

    static real (*sqlength)(const V2D<real> *v);

    static real (*dot)(const V2D<real> *v1, const V2D<real> *v2);

    static real (*dist)(const V2D<real> *v1, const V2D<real> *v2);

    static real (*sqdist)(const V2D<real> *v1, const V2D<real> *v2);

    static real (*angle)(const V2D<real> *v1, const V2D<real> *v2);

    static void (*rotate)(V2D<real> *v, const real a);

    static const V2D<real> *kZERO;

    static const V2D<real> *kX;

    static const V2D<real> *kY;

    real x;
    real y;
};

#endif
