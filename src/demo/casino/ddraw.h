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

void die_draw(
        DCtx *ctx, 
        const real32_t x, 
        const real32_t y, 
        const real32_t width, 
        const real32_t height, 
        const real32_t padding,
        const real32_t corner,
        const real32_t radius,
        const uint32_t face);
