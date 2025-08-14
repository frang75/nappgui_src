/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osfont.c
 *
 */

/* Fonts */

#include "../font.h"
#include "../font.inl"
#include "../dctxh.h"
#include "../draw2d.inl"
#include <core/arrpt.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

#include <sewer/nowarn.hxx>
#include <pango/pangocairo.h>
#include <sewer/warn.hxx>

/* System font should be set from GTK or other toolkit manager */
static String *i_SYSTEM_FONT_FAMILY = NULL;
static real32_t kFONT_REGULAR_SIZE = 0.f;
static real32_t kFONT_SMALL_SIZE = 0.f;
static real32_t kFONT_MINI_SIZE = 0.f;
static real32_t i_PANGO_TO_PIXELS = -1;
static cairo_t *i_CAIRO = NULL;
static PangoLayout *i_LAYOUT = NULL;

/*---------------------------------------------------------------------------*/

void osfont_alloc_globals(void)
{
}

/*---------------------------------------------------------------------------*/

void osfont_dealloc_globals(void)
{
    str_destopt(&i_SYSTEM_FONT_FAMILY);

    if (i_CAIRO != NULL)
    {
        g_object_unref(i_LAYOUT);
        cairo_destroy(i_CAIRO);
        i_LAYOUT = NULL;
        i_CAIRO = NULL;
    }
}

/*---------------------------------------------------------------------------*/

static const char_t *i_monospace_font_family(void)
{
    const char_t *desired_fonts[] = {"Ubuntu Mono", "DejaVu Sans Mono", "Courier New"};
    return _draw2d_monospace_family(desired_fonts, sizeof(desired_fonts) / sizeof(const char_t *));
}

/*---------------------------------------------------------------------------*/

static real32_t i_device_to_pixels(void)
{
    if (i_PANGO_TO_PIXELS < 0)
    {
        /* This object is owned by Pango and must not be freed */
        PangoFontMap *fontmap = pango_cairo_font_map_get_default();
        real32_t dpi = (real32_t)pango_cairo_font_map_get_resolution(cast(fontmap, PangoCairoFontMap));
        i_PANGO_TO_PIXELS = (dpi / 72.f) / PANGO_SCALE;
    }

    return i_PANGO_TO_PIXELS;
}

/*---------------------------------------------------------------------------*/

static gint i_font_size(const real32_t size, const uint32_t style)
{
    if ((style & ekFPOINTS) == ekFPOINTS)
    {
        return (gint)(size * (real32_t)PANGO_SCALE);
    }
    else
    {
        /* Pixels */
        return (gint)(size / i_device_to_pixels());
    }
}

/*---------------------------------------------------------------------------*/

static gint i_scale_size_to_cell(const real32_t size, const int pango_size, PangoFontDescription *font)
{
    real32_t cell_size = 1e8f;
    gint new_pango_size = 0;
    osfont_extents(cast(font, OSFont), "REFTEXT", 1, -1, NULL, &cell_size);
    new_pango_size = (gint)((size * (real32_t)pango_size) / cell_size);
    return new_pango_size;
}

/*---------------------------------------------------------------------------*/

OSFont *osfont_create(const char_t *family, const real32_t size, const real32_t width, const real32_t xscale, const uint32_t style)
{
    const char_t *name = NULL;
    gint psize = 0;
    PangoFontDescription *font = NULL;

    /*
     * In Pango, font width or scale is not assigned as PangoFontDescription property
     * With be use as scale factor in Cairo contexts by font_xscale()
     */
    unref(width);
    unref(xscale);

    if (str_equ_c(family, "__SYSTEM__") == TRUE)
    {
        cassert(i_SYSTEM_FONT_FAMILY != NULL);
        name = tc(i_SYSTEM_FONT_FAMILY);
    }
    else if (str_equ_c(family, "__MONOSPACE__") == TRUE)
    {
        name = i_monospace_font_family();
    }
    else
    {
        name = family;
    }

    psize = i_font_size(size, style);
    font = pango_font_description_new();
    pango_font_description_set_family(font, name);
    pango_font_description_set_size(font, psize);
    pango_font_description_set_style(font, (style & ekFITALIC) == ekFITALIC ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
    pango_font_description_set_weight(font, (style & ekFBOLD) == ekFBOLD ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);

    if ((style & ekFCELL) == ekFCELL)
    {
        gint s = i_scale_size_to_cell(size, psize, font);
        pango_font_description_set_size(font, s);
    }

    heap_auditor_add("PangoFontDescription");
    return cast(font, OSFont);
}

/*---------------------------------------------------------------------------*/

void osfont_destroy(OSFont **font)
{
    cassert_no_null(font);
    cassert_no_null(*font);
    pango_font_description_free(*dcast(font, PangoFontDescription));
    heap_auditor_delete("PangoFontDescription");
    *font = NULL;
}

/*---------------------------------------------------------------------------*/

String *osfont_family_name(const OSFont *font)
{
    const PangoFontDescription *ffont = cast_const(font, PangoFontDescription);
    const char *desc = NULL;
    cassert_no_null(ffont);
    desc = pango_font_description_get_family(ffont);
    return str_c(desc);
}

/*---------------------------------------------------------------------------*/

font_family_t osfont_system(const char_t *family)
{
    if (str_equ(i_SYSTEM_FONT_FAMILY, family) == TRUE)
        return ekFONT_FAMILY_SYSTEM;

    {
        const char_t *mono = i_monospace_font_family();
        if (str_equ_c(mono, family) == TRUE)
            return ekFONT_FAMILY_MONOSPACE;
    }

    return ENUM_MAX(font_family_t);
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_monospace(PangoFontMap *fontmap, const PangoFontDescription *ffont)
{
    PangoFontFamily **families = NULL;
    const char *desc = NULL;
    bool_t mono = FALSE;
    int i, n;
    cassert_no_null(ffont);
    desc = pango_font_description_get_family(ffont);

    /* This array should be freed with g_free(). */
    pango_font_map_list_families(fontmap, &families, &n);
    for (i = 0; i < n; i++)
    {
        const char_t *name = cast_const(pango_font_family_get_name(families[i]), char_t);
        if (str_equ_c(name, desc) == TRUE)
        {
            mono = pango_font_family_is_monospace(families[i]);
            break;
        }
    }
    g_free(families);
    return mono;
}

/*---------------------------------------------------------------------------*/

void osfont_metrics(const OSFont *font, const real32_t size, const real32_t xscale, real32_t *ascent, real32_t *descent, real32_t *leading, real32_t *cell_size, real32_t *avg_width, bool_t *monospace)
{
    /* This object is owned by Pango and must not be freed */
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoContext *context = pango_font_map_create_context(fontmap);
    PangoFont *ffont = NULL;
    PangoFontMetrics *metrics = NULL;
    cassert_no_null(font);

    ffont = pango_font_map_load_font(fontmap, context, cast(font, PangoFontDescription));
    metrics = pango_font_get_metrics(ffont, NULL);

    if (ascent != NULL)
        *ascent = (real32_t)(pango_font_metrics_get_ascent(metrics) / PANGO_SCALE);

    if (descent != NULL)
        *descent = (real32_t)(pango_font_metrics_get_descent(metrics) / PANGO_SCALE);

    /* We need to get a real text measure */
    if (leading != NULL || cell_size != NULL || avg_width != NULL)
    {
        real32_t width, height;
        uint32_t len;
        const char_t *str = _draw2d_str_avg_char_width(&len);
        osfont_extents(font, str, xscale, -1, &width, &height);

        if (leading != NULL)
            *leading = height - size;

        if (cell_size != NULL)
            *cell_size = height;

        if (avg_width != NULL)
            *avg_width = width / len;
    }

    if (monospace != NULL)
        *monospace = i_is_monospace(fontmap, cast_const(font, PangoFontDescription));

    g_object_unref(context);
    g_object_unref(ffont);
    pango_font_metrics_unref(metrics);
}

/*---------------------------------------------------------------------------*/

void osfont_extents(const OSFont *font, const char_t *text, const real32_t xscale, const real32_t refwidth, real32_t *width, real32_t *height)
{
    int w, h;
    cassert_no_null(font);
    if (i_CAIRO == NULL)
    {
        i_CAIRO = cairo_create(NULL);
        i_LAYOUT = pango_cairo_create_layout(i_CAIRO);
    }

    pango_layout_set_font_description(i_LAYOUT, cast(font, PangoFontDescription));
    pango_layout_set_text(i_LAYOUT, cast_const(text, char), -1);
    pango_layout_set_width(i_LAYOUT, refwidth < 0 ? -1 : (int)((refwidth / xscale) * PANGO_SCALE));
    pango_layout_get_pixel_size(i_LAYOUT, &w, &h);
    ptr_assign(width, (real32_t)w * xscale);
    ptr_assign(height, (real32_t)h);
}

/*---------------------------------------------------------------------------*/

const void *osfont_native(const OSFont *font)
{
    cassert_no_null(font);
    return cast(font, void);
}

/*---------------------------------------------------------------------------*/

real32_t font_regular_size(void)
{
    cassert(i_SYSTEM_FONT_FAMILY != NULL);
    return kFONT_REGULAR_SIZE;
}

/*---------------------------------------------------------------------------*/

real32_t font_small_size(void)
{
    cassert(i_SYSTEM_FONT_FAMILY != NULL);
    return kFONT_SMALL_SIZE;
}

/*---------------------------------------------------------------------------*/

real32_t font_mini_size(void)
{
    cassert(i_SYSTEM_FONT_FAMILY != NULL);
    return kFONT_MINI_SIZE;
}

/*---------------------------------------------------------------------------*/

const char_t *font_register(const byte_t *data, const uint32_t size)
{
    unref(data);
    unref(size);
    cassert(FALSE);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool_t font_exists_family(const char_t *font_family)
{
    /* This object is owned by Pango and must not be freed. */
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families = NULL;
    int i, n;

    /* This array should be freed with g_free(). */
    pango_font_map_list_families(fontmap, &families, &n);
    for (i = 0; i < n; i++)
    {
        const char_t *name = cast_const(pango_font_family_get_name(families[i]), char_t);
        if (str_equ_nocase(font_family, name) == TRUE)
            break;
    }
    g_free(families);
    return (bool_t)(i < n);
}

/*---------------------------------------------------------------------------*/

static ArrPt(String) *i_installed_families(const bool_t only_mono)
{
    ArrPt(String) *fonts = arrpt_create(String);
    /* This object is owned by Pango and must not be freed. */
    PangoFontMap *fontmap = pango_cairo_font_map_get_default();
    PangoFontFamily **families = NULL;
    int i, n;

    /* This array should be freed with g_free(). */
    pango_font_map_list_families(fontmap, &families, &n);
    for (i = 0; i < n; i++)
    {
        const char_t *name = cast_const(pango_font_family_get_name(families[i]), char_t);
        if (only_mono == FALSE || pango_font_family_is_monospace(families[i]) == TRUE)
        {
            String *font = str_c(name);
            arrpt_append(fonts, font, String);
        }
    }
    g_free(families);
    arrpt_sort(fonts, str_scmp, String);
    return fonts;
}

/*---------------------------------------------------------------------------*/

ArrPt(String) *font_installed_families(void)
{
    return i_installed_families(FALSE);
}

/*---------------------------------------------------------------------------*/

ArrPt(String) *font_installed_monospace(void)
{
    return i_installed_families(TRUE);
}

/*---------------------------------------------------------------------------*/

void dctx_set_default_osfont(DCtx *ctx, const void *font)
{
    const PangoFontDescription *fdesc = cast_const(font, PangoFontDescription);
    real32_t scale = i_device_to_pixels();
    const char *family = NULL;
    real32_t size = 0;
    unref(ctx);
    cassert(i_SYSTEM_FONT_FAMILY == NULL);
    family = pango_font_description_get_family(fdesc);
    size = (real32_t)pango_font_description_get_size(fdesc);
    i_SYSTEM_FONT_FAMILY = str_c(cast_const(family, char_t));
    kFONT_REGULAR_SIZE = size * scale;
    kFONT_SMALL_SIZE = kFONT_REGULAR_SIZE - 2.f;
    kFONT_MINI_SIZE = kFONT_REGULAR_SIZE - 4.f;
}