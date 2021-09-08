/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: s2d.h
 * https://nappgui.com/en/geom2d/s2d.html
 *
 */

/* Size 2d */

#include "geom2d.hxx"

__EXTERN_C

S2Df s2df(const real32_t width, const real32_t height);

S2Df s2di(const uint32_t width, const uint32_t height);

S2Dd s2dd(const real64_t width, const real64_t height);

extern const S2Df kS2D_ZEROf;
extern const S2Dd kS2D_ZEROd;

__END_C
