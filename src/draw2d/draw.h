/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw.h
 * https://nappgui.com/en/draw2d/draw.html
 *
 */

/* Drawing commands */

#include "draw2d.hxx"

__EXTERN_C

void draw_line(DCtx *ctx, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1);

void draw_polyline(DCtx *ctx, const bool_t closed, const V2Df *points, const uint32_t n);

void draw_arc(DCtx *ctx, const real32_t x, const real32_t y, const real32_t radius, const real32_t start, const real32_t sweep);

void draw_bezier(DCtx *ctx, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1, const real32_t x2, const real32_t y2, const real32_t x3, const real32_t y3);

void draw_line_color(DCtx *ctx, const color_t color);

void draw_line_fill(DCtx *ctx);

void draw_line_width(DCtx *ctx, const real32_t width);

void draw_line_cap(DCtx *ctx, const linecap_t cap);

void draw_line_join(DCtx *ctx, const linejoin_t join);

void draw_line_dash(DCtx *ctx, const real32_t *pattern, const uint32_t n);

void draw_rect(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t width, const real32_t height);

void draw_rndrect(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const real32_t radius);

void draw_circle(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t radius);

void draw_ellipse(DCtx *ctx, const drawop_t op, const real32_t x, const real32_t y, const real32_t radx, const real32_t rady);

void draw_polygon(DCtx *ctx, const drawop_t op, const V2Df *points, const uint32_t n);

void draw_fill_color(DCtx *ctx, const color_t color);

void draw_fill_linear(DCtx *ctx, const color_t *color, const real32_t *stop, const uint32_t n, const real32_t x0, const real32_t y0, const real32_t x1, const real32_t y1);

void draw_fill_matrix(DCtx *ctx, const T2Df *t2d);

void draw_fill_wrap(DCtx *ctx, const fillwrap_t wrap);

void draw_font(DCtx *ctx, const Font *font);

void draw_text_color(DCtx *ctx, const color_t color);

void draw_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y);

void draw_text_path(DCtx *ctx, const drawop_t op, const char_t *text, const real32_t x, const real32_t y);

void draw_text_width(DCtx *ctx, const real32_t width);

void draw_text_trim(DCtx *ctx, const ellipsis_t ellipsis);

void draw_text_align(DCtx *ctx, const align_t halign, const align_t valign);

void draw_text_halign(DCtx *ctx, const align_t halign);

void draw_text_extents(DCtx *ctx, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);

void draw_image(DCtx *ctx, const Image *image, const real32_t x, const real32_t y);

void draw_image_frame(DCtx *ctx, const Image *image, const uint32_t frame, const real32_t x, const real32_t y);

void draw_image_align(DCtx *ctx, const align_t halign, const align_t valign);

__END_C
