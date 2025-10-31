/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: drawctrl.inl
 *
 */

/* Drawing custom GUI controls */

#include "gui.hxx"

__EXTERN_C

_gui_api Font *drawctrl_font(DCtx *ctx);

_gui_api uint32_t drawctrl_row_padding(DCtx *ctx);

_gui_api uint32_t drawctrl_check_width(DCtx *ctx);

_gui_api uint32_t drawctrl_check_height(DCtx *ctx);

_gui_api ctrl_msel_t drawctrl_multisel(DCtx *ctx, const vkey_t key);

_gui_api void drawctrl_clear(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);

_gui_api void drawctrl_header(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state);

_gui_api void drawctrl_indicator(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const uint32_t indicator);

_gui_api void drawctrl_fill(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state);

_gui_api void drawctrl_line(DCtx *ctx, const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1);

_gui_api void drawctrl_focus(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state);

_gui_api void drawctrl_text(DCtx *ctx, const char_t *text, const int32_t x, const int32_t y, const ctrl_state_t state);

_gui_api void drawctrl_image(DCtx *ctx, const Image *image, const int32_t x, const int32_t y);

_gui_api void drawctrl_checkbox(DCtx *ctx, const int32_t x, const int32_t y, const ctrl_state_t state);

_gui_api void drawctrl_uncheckbox(DCtx *ctx, const int32_t x, const int32_t y, const ctrl_state_t state);

__END_C
