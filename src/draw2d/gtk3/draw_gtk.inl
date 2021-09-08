/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw_gtk.inl
 *
 */

/* Drawing commands */

#include "draw2d.ixx"
//#include <gtk/gtk.h>

__EXTERN_C

void drawimp_raster_mode(DCtx *ctx);

void drawimp_begin_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y);

void drawimp_color(cairo_t *cairo, const color_t color, color_t *source_color);

void drawimp_draw(DCtx *ctx, const drawop_t op);

__END_C

