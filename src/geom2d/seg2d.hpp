/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: seg2d.hpp
 *
 */

/* 2d segments */

#ifndef __SEG2D_HPP__
#define __SEG2D_HPP__

#include "v2d.hpp"

template<typename real>
struct Seg2D
{
    Seg2D() {p0.x = 0, p0.y = 0, p1.x = 0, p1.y = 0;}

    Seg2D(const real x0, const real y0, const real x1, const real y1) {p0.x = x0, p0.y = y0, p1.x = x1, p1.y = y1;}

    Seg2D(const V2D<real> *_p0, const V2D<real> *_p1) {p0 = *_p0, p1 = *_p1;}

    static real (*length)(const Seg2D<real> *seg);

    static real (*sqlength)(const Seg2D<real> *seg);

    static V2D<real> (*eval)(const Seg2D<real> *seg, const real t);

    static real (*close_param)(const Seg2D<real> *seg, const V2D<real> *pnt);

    static real (*point_sqdist)(const Seg2D<real> *seg, const V2D<real> *pnt, real *t);

    static real (*sqdist)(const Seg2D<real> *seg1, const Seg2D<real> *seg2, real*, real*);

    V2D<real> p0;
    V2D<real> p1;
};

#endif

