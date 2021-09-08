/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx_win.inl
 *
 */

/* Draw context */

#include "draw2d_win.ixx"

__EXTERN_C

//Gdiplus::ColorPalette* _dctx_4bpp_grayscale_palette(void);

Gdiplus::ColorPalette* dctx_8bpp_grayscale_palette(void);

void dctx_gradient_transform(DCtx *ctx);

void dctx_set_gdi_mode(DCtx *ctx);

__END_C

