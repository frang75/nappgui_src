/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: t2d.h
 * https://nappgui.com/en/geom2d/t2d.html
 *
 */

/* 2D affine transformations */

#include "geom2d.hxx"

__EXTERN_C

void t2d_movef(T2Df *dest, const T2Df *src, const real32_t x, const real32_t y);

void t2d_moved(T2Dd *dest, const T2Dd *src, const real64_t x, const real64_t y);

void t2d_rotatef(T2Df *dest, const T2Df *src, const real32_t angle);

void t2d_rotated(T2Dd *dest, const T2Dd *src, const real64_t angle);

void t2d_scalef(T2Df *dest, const T2Df *src, const real32_t sx, const real32_t sy);

void t2d_scaled(T2Dd *dest, const T2Dd *src, const real64_t sx, const real64_t sy);

void t2d_invfastf(T2Df *dest, const T2Df *src);

void t2d_invfastd(T2Dd *dest, const T2Dd *src);

void t2d_inversef(T2Df *dest, const T2Df *src);

void t2d_inversed(T2Dd *dest, const T2Dd *src);

void t2d_multf(T2Df *dest, const T2Df *src1, const T2Df *src2);

void t2d_multd(T2Dd *dest, const T2Dd *src1, const T2Dd *src2);

void t2d_vmultf(V2Df *dest, const T2Df *t2d, const V2Df *src);

void t2d_vmultd(V2Dd *dest, const T2Dd *t2d, const V2Dd *src);

void t2d_vmultnf(V2Df *dest, const T2Df *t2d, const V2Df *src, const uint32_t n);

void t2d_vmultnd(V2Dd *dest, const T2Dd *t2d, const V2Dd *src, const uint32_t n);

void t2d_decomposef(const T2Df *t2d, V2Df *pos, real32_t *angle, V2Df *sc);

void t2d_decomposed(const T2Dd *t2d, V2Dd *pos, real64_t *angle, V2Dd *sc);

void t2d_to32(T2Df *dest, const T2Dd *src);

void t2d_to64(T2Dd *dest, const T2Df *src);

extern const T2Df *kT2D_IDENTf;
extern const T2Dd *kT2D_IDENTd;

__END_C
