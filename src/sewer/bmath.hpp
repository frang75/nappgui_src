/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmath.hpp
 *
 */

/* Math funcions */

#include "sewer.hxx"

template<typename real>
struct BMath
{
    static real(*cos)(const real angle);

    static real(*sin)(const real angle);

    static real(*tan)(const real angle);

    static real(*acos)(const real cos);

    static real(*asin)(const real sin);

    static real(*atan2)(const real y, const real x);

    static real(*norm_angle)(const real a);

    static real(*sqrt)(const real value);

    static real(*isqrt)(const real value);

    static real(*log)(const real value);

    static real(*log10)(const real value);

    static real(*exp)(const real value);

    static real(*pow)(const real base, const real exponent);

    static real(*abs)(const real value);

    static real(*max)(const real value1, const real value2);

    static real(*min)(const real value1, const real value2);

    static real(*clamp)(const real value, const real min, const real max);

    static real(*mod)(const real num, const real den);

    static real(*modf)(const real value, real *intpart);

    static uint32_t(*prec)(const real value);

    static real(*round)(const real value);

    static real(*round_step)(const real value, const real step);

    static real(*floor)(const real value);

    static real(*ceil)(const real value);

    static const real kE;
    static const real kLN2;
    static const real kLN10;
    static const real kPI;
    static const real kSQRT2;
    static const real kSQRT3;
    static const real kDEG2RAD;
    static const real kRAD2DEG;
    static const real kINFINITY;
};

