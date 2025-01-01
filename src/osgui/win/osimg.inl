/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osimg.inl
 *
 */

/* GDI Image support */

#include "osgui_win.ixx"

__EXTERN_C

HBITMAP _osimg_hbitmap(const Image *image, COLORREF background);

HBITMAP _osimg_hbitmap_cache(const Image *image, COLORREF background, LONG *width, LONG *height);

HBITMAP _osimg_hbitmap_transparent(const uint32_t width, const uint32_t height);

HCURSOR _osimg_hcursor(const Image *image, const uint32_t hot_x, const uint32_t hot_y);

void _osimg_draw(const Image *image, HDC hdc, const uint32_t frame_index, const real32_t x, const real32_t y, const real32_t width, const real32_t height, const BOOL gray);

__END_C
