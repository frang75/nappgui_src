/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.h
 *
 */

/* Operating System Custom Control Drawing */

#include "osgui.hxx"

__EXTERN_C

Font *osdrawctrl_font(const DCtx *ctx);

uint32_t osdrawctrl_row_padding(const DCtx *ctx);

uint32_t osdrawctrl_check_width(const DCtx *ctx);

uint32_t osdrawctrl_check_height(const DCtx *ctx);

multisel_t osdrawctrl_multisel(const DCtx *ctx, const vkey_t key);

void osdrawctrl_clear(DCtx *ctx);

void osdrawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state);

void osdrawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state);

void osdrawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state);

void osdrawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y, const cstate_t state);

void osdrawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state);

void osdrawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state);

__END_C
