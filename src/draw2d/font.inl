/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: font.inl
 *
 */

/* Fonts */

#include "draw2d.ixx"

__EXTERN_C

void osfont_alloc_globals(void);

void osfont_dealloc_globals(void);

OSFont *osfont_create(const char_t *family, const real32_t size, const real32_t width, const real32_t xscale, const uint32_t style);

void osfont_destroy(OSFont **font);

String *osfont_family_name(const OSFont *font);

font_family_t osfont_system(const char_t *family);

real32_t osfont_ascent(const OSFont *font);

real32_t osfont_descent(const OSFont *font);

real32_t osfont_leading(const OSFont *font, const real32_t size);

real32_t osfont_cell_size(const OSFont *font);

real32_t osfont_avg_width(const OSFont *font, const real32_t xscale);

bool_t osfont_is_monospace(const OSFont *font);

void osfont_extents(const OSFont *font, const char_t *text, const real32_t xscale, const real32_t refwidth, real32_t *width, real32_t *height);

const void *osfont_native(const OSFont *font);

const void *osfont_native_dpi(OSFont *font, const uint32_t dpi);

void osfont_metrics_dpi(const uint32_t dpi);

__END_C