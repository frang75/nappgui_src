/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: color.h
 * https://nappgui.com/en/draw2d/color.html
 *
 */

/* Color */

#include "draw2d.hxx"

__EXTERN_C

color_t color_rgb(const uint8_t r, const uint8_t g, const uint8_t b);

color_t color_rgba(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

color_t color_rgbaf(const real32_t r, const real32_t g, const real32_t b, const real32_t a);

color_t color_hsbf(const real32_t hue, const real32_t sat, const real32_t bright);

color_t color_red(const uint8_t r);

color_t color_green(const uint8_t g);

color_t color_blue(const uint8_t b);

color_t color_gray(const uint8_t l);

color_t color_bgr(const uint32_t bgr);

color_t color_html(const char_t *html);

void color_to_hsbf(const color_t color, real32_t *hue, real32_t *sat, real32_t *bright);

void color_to_html(const color_t color, char_t *html, const uint32_t size);

void color_get_rgb(const color_t color, uint8_t *r, uint8_t *g, uint8_t *b);

void color_get_rgbf(const color_t color, real32_t *r, real32_t *g, real32_t *b);

void color_get_rgba(const color_t color, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a);

void color_get_rgbaf(const color_t color, real32_t *r, real32_t *g, real32_t *b, real32_t *a);

uint8_t color_get_alpha(const color_t color);

color_t color_set_alpha(const color_t color, const uint8_t alpha);

extern const color_t kCOLOR_TRANSPARENT;

extern const color_t kCOLOR_DEFAULT;

extern const color_t kCOLOR_BLACK;

extern const color_t kCOLOR_WHITE;

extern const color_t kCOLOR_RED;

extern const color_t kCOLOR_GREEN;

extern const color_t kCOLOR_BLUE;

extern const color_t kCOLOR_YELLOW;

extern const color_t kCOLOR_CYAN;

extern const color_t kCOLOR_MAGENTA;

__END_C
