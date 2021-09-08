/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: btexth.inl
 *
 */

/* Text formatted blocks */

#include "draw2d.ixx"

__EXTERN_C

BText *btext_create(void);

void btext_destroy(BText **block);

void btext_push_font_family(BText *block, const char_t *family);

void btext_push_font_relsize(BText *block, const real32_t rsize);

void btext_push_font_size(BText *block, const real32_t size);

void btext_push_font_style(BText *block, const uint32_t style);

void btext_push_line_color(BText *block, const color_t color);

void btext_push_fill_color(BText *block, const color_t color);

void btext_push_halign(BText *block, const align_t align);

void btext_push_drawop(BText *block, const drawop_t op);

uint32_t btext_push_track(BText *block);

void btext_pop(BText *block);

void btext_text(BText *block, const char_t *text);

void btext_update(BText *block, const real32_t max_width, const real32_t max_height, const Font *font);

void btext_bounds(const BText *block, real32_t *width, real32_t *height);

void btext_draw(DCtx *ctx, const BText *block, const real32_t x, const real32_t y, const align_t halign, const align_t valign);

void btext_draw_raster(DCtx *ctx, const BText *block, const real32_t x, const real32_t y, const align_t halign, const align_t valign);

uint32_t btext_track(const BText *block, const real32_t x, const real32_t y);

void btext_track_line_color(BText *block, const uint32_t track_id, const color_t color);

void btext_track_fill_color(BText *block, const uint32_t track_id, const color_t color);

void btext_track_underline(BText *block, const uint32_t track_id, bool_t underline);

__END_C

