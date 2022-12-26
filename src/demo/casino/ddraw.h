/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ddraw.h
 *
 */

/* Die drawing */

#include "casino.hxx"

_casino_api void die_draw(
                        DCtx *ctx,
                        const real32_t x,
                        const real32_t y,
                        const real32_t width,
                        const real32_t height,
                        const real32_t padding,
                        const real32_t corner,
                        const real32_t radius,
                        const uint32_t face);

_casino_api extern const real32_t kDEF_PADDING;

_casino_api extern const real32_t kDEF_CORNER;

_casino_api extern const real32_t kDEF_RADIUS;
