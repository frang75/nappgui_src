/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: nsvg.h
 *
 */

/* NanoSVG support */

#include "nsvg.hxx"

__EXTERN_C

/* NSVG object can cache SVG operations */
_nsvg_api NSVG *nsvg_from_data(const byte_t *data, const uint32_t size);

_nsvg_api void nsvg_destroy(NSVG **svg);

_nsvg_api Pixbuf *nsvg_pixbuf(const NSVG *svg, const uint32_t width, const uint32_t height, const pixformat_t format);

/* Draw the SVG in vector form in a drawing context */
_nsvg_api void nsvg_draw(const NSVG *svg, DCtx *ctx);

__END_C
