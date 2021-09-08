/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: pixbuf.h
 * https://nappgui.com/en/draw2d/pixbuf.html
 *
 */

/* Pixel buffers */

#include "draw2d.hxx"

#define pixbuf_get1(data, x, y, width)\
        (uint32_t)((data[((y) * (width) + (x)) / 8] >> (byte_t)(((y) * (width) + (x)) % 8)) & 1)

__EXTERN_C

Pixbuf *pixbuf_create(const uint32_t width, const uint32_t height, const pixformat_t format);

Pixbuf *pixbuf_copy(const Pixbuf *pixbuf);

Pixbuf *pixbuf_trim(const Pixbuf *pixbuf, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height);

Pixbuf *pixbuf_convert(const Pixbuf *pixbuf, const Palette *palette, const pixformat_t oformat);

void pixbuf_destroy(Pixbuf **pixbuf);

pixformat_t pixbuf_format(const Pixbuf *pixbuf);

uint32_t pixbuf_width(const Pixbuf *pixbuf);

uint32_t pixbuf_height(const Pixbuf *pixbuf);

uint32_t pixbuf_size(const Pixbuf *pixbuf);

uint32_t pixbuf_dsize(const Pixbuf *pixbuf);

const byte_t *pixbuf_cdata(const Pixbuf *pixbuf);

byte_t *pixbuf_data(Pixbuf *pixbuf);

uint32_t pixbuf_format_bpp(const pixformat_t format);

uint32_t pixbuf_get(const Pixbuf *pixbuf, const uint32_t x, const uint32_t y);

void pixbuf_set(Pixbuf *pixbuf, const uint32_t x, const uint32_t y, const uint32_t value);

__END_C

