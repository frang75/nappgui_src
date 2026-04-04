/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: svg.inl
 *
 */

/* Internal SVG support */

#include "draw2d.ixx"

__EXTERN_C

bool_t _svg_is_data(const byte_t *data, const uint32_t size);

Pixbuf *_svg_render(const byte_t *data, const uint32_t size, const uint32_t width, const uint32_t height, real32_t *svg_width, real32_t *svg_height);

__END_C
