/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: font.c
 *
 */

/* Fonts */

#include "font.h"
#include "font.inl"
#include "draw2d.inl"
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _font_t
{
    uint32_t num_instances;
    uint32_t family;
    uint32_t style;
    real32_t size;
    real32_t xscale;
    real32_t cell_size;
    real32_t leading;
    real32_t ascent;
    real32_t descent;
    real32_t avg_width;
    bool_t monospace;
    bool_t metrics;
    String *family_name;
    OSFont *osfont;
};

/*---------------------------------------------------------------------------*/

#define i_abs(x) (((x) < 0.f) ? -(x) : (x))

/*---------------------------------------------------------------------------*/

static Font *i_create_font(const uint32_t family, const real32_t size, const uint32_t style)
{
    Font *font = heap_new(Font);
    font->num_instances = 1;
    font->family = family;
    font->size = size;
    font->style = style;
    font->xscale = 1;
    font->cell_size = -1;
    font->leading = -1;
    font->ascent = -1;
    font->descent = -1;
    font->avg_width = -1;
    font->monospace = FALSE;
    font->metrics = FALSE;
    font->family_name = NULL;
    font->osfont = NULL;
    return font;
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_osfont(Font *font)
{
    cassert_no_null(font);
    if (font->osfont == NULL)
    {
        const char_t *fname = draw2d_font_family(font->family);
        font->osfont = osfont_create(fname, font->size, -1, -1, font->style);
    }
}

/*---------------------------------------------------------------------------*/

static ___INLINE void i_metrics(Font *font)
{
    cassert_no_null(font);
    if (font->metrics == FALSE)
    {
        i_osfont(font);
        osfont_metrics(font->osfont, font->size, font->xscale, &font->ascent, &font->descent, &font->leading, &font->cell_size, &font->avg_width, &font->monospace);
        font->metrics = TRUE;
    }
}

/*---------------------------------------------------------------------------*/

void font_destroy(Font **font)
{
    cassert_no_null(font);
    cassert_no_null(*font);
    if ((*font)->num_instances == 1)
    {
        if ((*font)->osfont != NULL)
            osfont_destroy(&(*font)->osfont);
        str_destopt(&(*font)->family_name);
        heap_delete(font, Font);
    }
    else
    {
        cassert((*font)->num_instances > 0);
        (*font)->num_instances -= 1;
    }
}

/*---------------------------------------------------------------------------*/

Font *font_create(const char_t *family, const real32_t size, const uint32_t style)
{
    uint32_t ffamily = draw2d_register_font(family);
    return i_create_font(ffamily, size, style);
}

/*---------------------------------------------------------------------------*/

Font *font_system(const real32_t size, const uint32_t style)
{
    return i_create_font((uint32_t)ekFONT_FAMILY_SYSTEM, size, style);
}

/*---------------------------------------------------------------------------*/

Font *font_monospace(const real32_t size, const uint32_t style)
{
    return i_create_font((uint32_t)ekFONT_FAMILY_MONOSPACE, size, style);
}

/*---------------------------------------------------------------------------*/

Font *font_with_style(const Font *font, const uint32_t style)
{
    cassert_no_null(font);
    return i_create_font(font->family, font->size, style);
}

/*---------------------------------------------------------------------------*/

Font *font_with_width(const Font *font, const real32_t width)
{
    Font *font_nscaled = cast(font, Font);
    Font *font_scaled = NULL;
    const char_t *fname = NULL;
    cassert_no_null(font);
    cassert(width > 0);

    /* The incomming font is scaled */
    if (bmath_absf(font_nscaled->xscale - 1.f) > 0.01f)
        font_nscaled = i_create_font(font->family, font->size, font->style);

    i_metrics(font_nscaled);
    cassert(font_nscaled->avg_width > 0);

    font_scaled = i_create_font(font_nscaled->family, font_nscaled->size, font_nscaled->style);
    font_scaled->xscale = width / font_nscaled->avg_width;
    cassert(font_scaled->osfont == NULL);
    fname = draw2d_font_family(font_scaled->family);
    font_scaled->osfont = osfont_create(fname, font_scaled->size, width, font_scaled->xscale, font_scaled->style);

    if (font_nscaled != font)
        font_destroy(&font_nscaled);

    return font_scaled;
}

/*---------------------------------------------------------------------------*/

Font *font_with_xscale(const Font *font, const real32_t scale)
{
    Font *font_nscaled = cast(font, Font);
    Font *font_scaled = NULL;
    const char_t *fname = NULL;
    real32_t width = 0;
    cassert_no_null(font);
    cassert(scale > 0);

    /* The incomming font is scaled */
    if (bmath_absf(font_nscaled->xscale - 1.f) > 0.01f)
        font_nscaled = i_create_font(font->family, font->size, font->style);

    i_metrics(font_nscaled);
    cassert(font_nscaled->avg_width > 0);

    font_scaled = i_create_font(font_nscaled->family, font_nscaled->size, font_nscaled->style);
    font_scaled->xscale = scale;
    width = font_nscaled->avg_width * scale;
    cassert(font_scaled->osfont == NULL);
    fname = draw2d_font_family(font_scaled->family);
    font_scaled->osfont = osfont_create(fname, font_scaled->size, width, font_scaled->xscale, font_scaled->style);

    if (font_nscaled != font)
        font_destroy(&font_nscaled);

    return font_scaled;
}

/*---------------------------------------------------------------------------*/

Font *font_copy(const Font *font)
{
    cassert_no_null(font);
    cast(font, Font)->num_instances += 1;
    return cast(font, Font);
}

/*---------------------------------------------------------------------------*/

bool_t font_equals(const Font *font1, const Font *font2)
{
    cassert_no_null(font1);
    cassert_no_null(font2);
    if (font1->family != font2->family)
        return FALSE;
    if (i_abs(font1->size - font2->size) > 0.0001f)
        return FALSE;
    if (i_abs(font1->xscale - font2->xscale) > 0.01f)
        return FALSE;
    if (font1->style != font2->style)
        return FALSE;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

const char_t *font_family(const Font *font)
{
    cassert_no_null(font);
    if (font->family_name == NULL)
    {
        i_osfont(cast(font, Font));
        cast(font, Font)->family_name = osfont_family_name(font->osfont);
    }

    return tc(font->family_name);
}

/*---------------------------------------------------------------------------*/

real32_t font_size(const Font *font)
{
    cassert_no_null(font);
    return font->size;
}

/*---------------------------------------------------------------------------*/

real32_t font_height(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->cell_size;
}

/*---------------------------------------------------------------------------*/

real32_t font_width(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->avg_width;
}

/*---------------------------------------------------------------------------*/

real32_t font_xscale(const Font *font)
{
    cassert_no_null(font);
    return font->xscale;
}

/*---------------------------------------------------------------------------*/

real32_t font_ascent(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->ascent;
}

/*---------------------------------------------------------------------------*/

real32_t font_descent(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->descent;
}

/*---------------------------------------------------------------------------*/

real32_t font_leading(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->leading;
}

/*---------------------------------------------------------------------------*/

bool_t font_is_monospace(const Font *font)
{
    cassert_no_null(font);
    i_metrics(cast(font, Font));
    return font->monospace;
}

/*---------------------------------------------------------------------------*/

uint32_t font_style(const Font *font)
{
    cassert_no_null(font);
    return font->style;
}

/*---------------------------------------------------------------------------*/

void font_extents(const Font *font, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    cassert_no_null(font);
    i_osfont(cast(font, Font));
    osfont_extents(font->osfont, text, font->xscale, refwidth, width, height);
}

/*---------------------------------------------------------------------------*/

const void *font_native(const Font *font)
{
    cassert_no_null(font);
    i_osfont(cast(font, Font));
    return font->osfont;
}