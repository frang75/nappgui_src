/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: pol2d.hpp
 *
 */

/* 2d polygon */

#ifndef __POL2D_HPP__
#define __POL2D_HPP__

#include "t2d.hpp"
#include "tri2d.hpp"
#include "box2d.hpp"
#include "arrst.hpp"
#include "arrpt.hpp"

template<typename real>
struct Pol2D
{
    static Pol2D<real>* (*create)(const V2D<real> *points, const uint32_t n);

    static Pol2D<real>* (*convex_hull)(const V2D<real> *points, const uint32_t n);

    static Pol2D<real>* (*copy)(const Pol2D<real> *pol);

    static void (*destroy)(Pol2D<real> **pol);

    static void (*transform)(Pol2D<real> *pol, const T2D<real> *t2d);

    static const V2D<real>* (*points)(const Pol2D<real> *pol);

    static uint32_t (*n)(const Pol2D<real> *pol);

    static real (*area)(const Pol2D<real> *pol);

    static Box2D<real> (*box)(const Pol2D<real> *pol);

    static bool_t (*ccw)(const Pol2D<real> *pol);

    static bool_t (*convex)(const Pol2D<real> *pol);

    static V2D<real> (*centroid)(const Pol2D<real> *pol);

    static V2D<real> (*visual_center)(const Pol2D<real> *pol, const real tol);

    static ArrSt<Tri2D<real> >* (*triangles)(const Pol2D<real> *pol);

    static ArrPt<Pol2D<real> >* (*convex_partition)(const Pol2D<real> *pol);
};

#endif

