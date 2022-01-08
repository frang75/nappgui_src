/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: font.h
 * https://nappgui.com/en/draw2d/font.html
 *
 */

/* Fonts */

#include "draw2d.hxx"

__EXTERN_C

Font *font_create(const char_t *family, const real32_t size, const uint32_t style);

Font *font_system(const real32_t size, const uint32_t style);

Font *font_monospace(const real32_t size, const uint32_t style);

Font *font_with_style(const Font *font, const uint32_t style);

Font *font_copy(const Font *font);

void font_destroy(Font **font);

bool_t font_equals(const Font *font1, const Font *font2);

real32_t font_regular_size(void);

real32_t font_small_size(void);

real32_t font_mini_size(void);

const char_t *font_family(const Font *font);

real32_t font_size(const Font *font);

real32_t font_height(const Font *font);

uint32_t font_style(const Font *font);

void font_extents(const Font *font, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height);

bool_t font_exists_family(const char_t *family);

ArrPt(String) *font_installed_families(void);

__END_C
