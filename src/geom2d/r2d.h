/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: r2d.h
 * https://nappgui.com/en/geom2d/r2d.html
 *
 */

/* 2D rectangles */

#include "geom2d.hxx"

__EXTERN_C

R2Df r2df(const real32_t x, const real32_t y, const real32_t width, const real32_t height);

R2Dd r2dd(const real64_t x, const real64_t y, const real64_t width, const real64_t height);

V2Df r2d_centerf(const R2Df* r2d);

V2Dd r2d_centerd(const R2Dd* r2d);

bool_t r2d_collidef(const R2Df *r2d1, const R2Df *r2d2);

bool_t r2d_collided(const R2Dd *r2d1, const R2Dd *r2d2);

bool_t r2d_containsf(const R2Df *r2d, const real32_t x, const real32_t y);

bool_t r2d_containsd(const R2Dd *r2d, const real64_t x, const real64_t y);

bool_t r2d_clipf(const R2Df *viewport, const R2Df *r2d);

bool_t r2d_clipd(const R2Dd *viewport, const R2Dd *r2d);

void r2d_joinf(R2Df *r2d, const R2Df *src);

void r2d_joind(R2Dd *r2d, const R2Dd *src);

extern const R2Df kR2D_ZEROf;
extern const R2Dd kR2D_ZEROd;

__END_C
