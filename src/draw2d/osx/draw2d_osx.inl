/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d_osx.inl
 *
 */

/* Draw */

void draw_raster_mode(DCtx *ctx);

void draw_shape_fill_color(DCtx *ctx, const color_t color);

void draw_text_wrap(DCtx *ctx, const ellipsis_t ellipsis);

NSString *drawctrl_begin_text(DCtx *ctx, const char_t *text, const real32_t x, const real32_t y, NSRect *rect);


