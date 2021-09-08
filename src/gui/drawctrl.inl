/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawctrl.inl
 *
 */

/* Drawing custom GUI controls */

#include "draw2d.ixx"

__EXTERN_C

Font *drawctrl_font(DCtx *ctx);

uint32_t drawctrl_row_padding(DCtx *ctx);

uint32_t drawctrl_check_width(DCtx *ctx);

uint32_t drawctrl_check_height(DCtx *ctx);

multisel_t drawctrl_multisel(DCtx *ctx, const vkey_t key);

void drawctrl_clear(DCtx *ctx);

void drawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state);

void drawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height);

void drawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state);

void drawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y);

void drawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const cstate_t state);

void drawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const cstate_t state);

__END_C

