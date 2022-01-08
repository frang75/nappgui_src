/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: s2d.hpp
 *
 */

/* Size 2d */

#ifndef __S2D_HPP__
#define __S2D_HPP__

template<typename real>
struct S2D
{
    S2D() {width = 0, height = 0;}

    S2D(const real _width, const real _height) {width = _width, height = _height;}

    static const S2D<real> *kZERO;

    real width;
    real height;
};

#endif

