/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
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

OSFont *osfont_create(const char_t *family, const real32_t size, const uint32_t style);

void osfont_destroy(OSFont **font);

String *osfont_family_name(const OSFont *font);

font_family_t osfont_system(const char_t *family);

void osfont_metrics(const OSFont *font, const real32_t size, real32_t *ascent, real32_t *descent, real32_t *leading, real32_t *cell_size, bool_t *monospace);

void osfont_extents(const OSFont *font, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);

const void *osfont_native(const OSFont *font);

__END_C
