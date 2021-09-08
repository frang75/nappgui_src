/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw.inl
 *
 */

/* Drawing commands */

#include "draw2d.ixx"

__EXTERN_C

void drawimp_alloc_globals(void);

void drawimp_dealloc_globals(void);

void draw_imgimp(DCtx *ctx, const OSImage *image, const uint32_t frame_index, const real32_t x, const real32_t y, const bool_t raster);

__END_C
