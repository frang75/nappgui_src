/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimage.inl
 *
 */

/* Images */

#include "draw2d_gdi.ixx"

__EXTERN_C 

void *osimage_bitmap(const OSImage *osimage);

HBITMAP osimage_hbitmap(const Image *image, COLORREF background);

HBITMAP osimage_hbitmap_cache(const Image *image, COLORREF background, LONG *width, LONG *height);

HBITMAP osimage_transparent_hbitmap(const uint32_t width, const uint32_t height);

HCURSOR osimage_hcursor(const Image *image, const uint32_t hot_x, const uint32_t hot_y);

void osimage_draw(const Image *image, HDC hdc, const uint32_t frame_index, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const BOOL gray);

__END_C

