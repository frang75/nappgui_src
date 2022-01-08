/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: box2d.hpp
 *
 */

/* Box 2d */

#ifndef __B2D_HPP__
#define __B2D_HPP__

#include "seg2d.hpp"

template<typename real>
struct Box2D
{
    Box2D() { *this = *kNULL; }

    Box2D(const real minX, const real minY, const real maxX, const real maxY) {min.x = minX, min.y = minY, max.x = maxX, max.y = maxY;}

    static Box2D<real> (*from_points)(const V2D<real> *p, const uint32_t n);

    static V2D<real> (*center)(const Box2D<real> *box);

    static void (*add)(Box2D<real> *box, const V2D<real> *p);

    static void (*addn)(Box2D<real> *box, const V2D<real> *p, const uint32_t n);

    static void (*merge)(Box2D<real> *dest, const Box2D<real> *src);

    static void (*segments)(const Box2D<real> *box, Seg2D<real> *segs);

    static real (*area)(const Box2D<real> *box);

    static bool_t (*is_null)(const Box2D<real> *box);

    static const Box2D<real> *kNULL;

    V2D<real> min;
    V2D<real> max;
};

template<typename real, typename real2>
struct Box2D2
{
    static void (*copy)(Box2D<real> *dest, const Box2D<real2> *src);
};

#endif
