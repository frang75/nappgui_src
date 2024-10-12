/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osfont.m
 *
 */

/* Font native implementation */

#include <sewer/nowarn.hxx>
#include <Cocoa/Cocoa.h>
#include <sewer/warn.hxx>
#include "draw2d_osx.ixx"

#include "font.h"
#include "font.inl"
#include "draw2d.inl"
#include "draw.inl"
#include <core/arrpt.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

static String *i_SYSTEM_FONT_FAMILY = NULL;
static String *i_MONOSPACE_FONT_FAMILY = NULL;
static bool_t i_SYSTEM_FONT_REDUCED = FALSE;

/*---------------------------------------------------------------------------*/

void osfont_alloc_globals(void)
{
    /*
     * I don't know why, in some newer macOS the system default font
     * has a great font scaling by default. This affect when apply font
     * transform, necessary for some fonts italic shear and x-scaling.
     * This code detects if the system font has this big scaling
     */
    real32_t xscale = 1.1f;
    OSFont *font = osfont_create("__SYSTEM__", font_regular_size(), -1, xscale, 0);
    real32_t w, h;
    osfont_extents(font, "OO", -1, xscale, &w, &h);
    osfont_destroy(&font);
    unref(w);
    if (h > 40)
        i_SYSTEM_FONT_REDUCED = TRUE;
}

/*---------------------------------------------------------------------------*/

void osfont_dealloc_globals(void)
{
    str_destopt(&i_SYSTEM_FONT_FAMILY);
    str_destopt(&i_MONOSPACE_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

real32_t font_regular_size(void)
{
    return (real32_t)[NSFont systemFontSize];
}

/*---------------------------------------------------------------------------*/

real32_t font_small_size(void)
{
    return (real32_t)[NSFont smallSystemFontSize];
}

/*---------------------------------------------------------------------------*/

real32_t font_mini_size(void)
{
    return (real32_t)[NSFont smallSystemFontSize] - 2.f;
}

/*---------------------------------------------------------------------------*/

static const char_t *i_monospace_font_family(void)
{
    if (i_MONOSPACE_FONT_FAMILY == NULL)
    {
        const char_t *desired_fonts[] = {"SF Mono", "Menlo", "Monaco", "Andale Mono", "Courier New"};
        const char_t *monofont = draw2d_monospace_family(desired_fonts, sizeof(desired_fonts) / sizeof(const char_t *));
        i_MONOSPACE_FONT_FAMILY = str_c(monofont);
    }

    return tc(i_MONOSPACE_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/
/*
 * This funtion apply two transforms:
 *  - A shear if itatic is required.
 *  - A x-scale if we need a char width different that font defaults.
 */
static NSFont *i_font_transform(NSFont *font, const CGFloat xscale, const CGFloat height, const BOOL italic, NSFontManager *font_manager)
{
    NSFont *tfont = nil;
    BOOL with_italic = NO;
    cassert_no_null(font);

    if (italic == YES)
    {
        /* The NSFontManager can apply the italic without affine */
        NSFontTraitMask traits = 0;
        tfont = [font_manager convertFont:font toHaveTrait:NSItalicFontMask];
        traits = [font_manager traitsOfFont:tfont];
        if ((traits & NSItalicFontMask) == NSItalicFontMask)
            with_italic = YES;
    }
    else
    {
        tfont = font;
    }

    /* We have to apply an affine transform to text */
    if ((italic == YES && with_italic == NO) || (xscale > 0 && fabs((double)xscale - 1) > 0.01))
    {
        NSAffineTransform *font_transform = [NSAffineTransform transform];

        /* Apply the x-scale */
        [font_transform scaleXBy:xscale * height yBy:height];

        /* Italic is required, but don't apply by FontManager */
        if (italic == YES && with_italic == NO)
        {
            NSAffineTransformStruct data;
            NSAffineTransform *italic_transform = nil;
            data.m11 = 1.f;
            data.m12 = 0.f;
            data.m21 = -tanf(/*italic_angle*/ -10.f * 0.017453292519943f);
            data.m22 = 1.f;
            data.tX = 0.f;
            data.tY = 0.f;
            italic_transform = [NSAffineTransform transform];
            [italic_transform setTransformStruct:data];
            [font_transform appendTransform:italic_transform];
        }

        tfont = [NSFont fontWithDescriptor:[tfont fontDescriptor] textTransform:font_transform];
    }

    return tfont;
}

/*---------------------------------------------------------------------------*/

static NSFont *i_nsfont(const char_t *family, const real32_t size, const uint32_t style)
{
    const char_t *name = NULL;
    NSFont *nsfont = nil;
    cassert(size > 0.f);

    if (str_equ_c(family, "__SYSTEM__") == TRUE)
    {
        if (style & ekFBOLD)
            nsfont = [NSFont boldSystemFontOfSize:(CGFloat)size];
        else
            nsfont = [NSFont systemFontOfSize:(CGFloat)size];
    }
    else if (str_equ_c(family, "__MONOSPACE__") == TRUE)
    {
        /* From Catalina, we have a system predefined monospace font */
#if defined(MAC_OS_X_VERSION_10_15) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
        if (draw2d_get_preferred_monospace() == NULL)
        {
            if (style & ekFBOLD)
                nsfont = [NSFont monospacedSystemFontOfSize:(CGFloat)size weight:NSFontWeightBold];
            else
                nsfont = [NSFont monospacedSystemFontOfSize:(CGFloat)size weight:NSFontWeightLight];
        }
#endif

        if (nsfont == nil)
            name = i_monospace_font_family();
    }
    else
    {
        name = family;
    }

    if (nsfont == nil && name != NULL)
    {
        NSFontManager *fontManager = [NSFontManager sharedFontManager];
        NSString *ffamily = [NSString stringWithUTF8String:name];
        NSUInteger mask = (style & ekFBOLD) ? NSBoldFontMask : 0;
        nsfont = [fontManager fontWithFamily:ffamily traits:(NSFontTraitMask)mask weight:5 size:(CGFloat)size];
    }

    return nsfont;
}

/*---------------------------------------------------------------------------*/

static real32_t i_cell_size(NSFont *font, const real32_t size)
{
    const char_t *reftext = "ABCDEabcde";
    real32_t twidth = 0, theight = 0;
    real32_t scale = 0;
    osfont_extents(cast(font, OSFont), reftext, 1, -1, &twidth, &theight);
    scale = size / theight;
    return bmath_floorf(size * scale);
}

/*---------------------------------------------------------------------------*/

OSFont *osfont_create(const char_t *family, const real32_t size, const real32_t width, const real32_t xscale, const uint32_t style)
{
    real32_t esize = size;
    NSFont *nsfont = i_nsfont(family, esize, style);
    cassert_fatal_msg(nsfont != nil, "Font is not available on this computer.");
    unref(width);

    if ((style & ekFCELL) == ekFCELL)
    {
        esize = i_cell_size(nsfont, esize);
        nsfont = i_nsfont(family, esize, style);
    }

    if (nsfont != nil)
    {
        BOOL with_italic = (style & ekFITALIC) == ekFITALIC;
        if (with_italic || (xscale > 0 && fabsf(xscale - 1) > 0.01))
        {
            NSFontManager *manager = [NSFontManager sharedFontManager];
            if (str_equ_c(family, "__SYSTEM__") == TRUE && i_SYSTEM_FONT_REDUCED == TRUE)
                esize /= size;
            nsfont = i_font_transform(nsfont, (CGFloat)xscale, (CGFloat)esize, with_italic, manager);
        }

        [nsfont retain];
    }

    return (OSFont *)nsfont;
}

/*---------------------------------------------------------------------------*/

void osfont_destroy(OSFont **font)
{
    cassert_no_null(font);
    cassert_no_null(*font);
    [cast(*font, NSFont) release];
    *font = NULL;
}

/*---------------------------------------------------------------------------*/

String *osfont_family_name(const OSFont *font)
{
    NSFont *nsfont = cast(font, NSFont);
    NSString *fname = nil;
    const char_t *utf8name = NULL;
    cassert_no_null(nsfont);
    fname = [nsfont familyName];
    utf8name = cast_const([fname UTF8String], char_t);
    return str_c(utf8name);
}

/*---------------------------------------------------------------------------*/

font_family_t osfont_system(const char_t *family)
{
    if (i_SYSTEM_FONT_FAMILY == NULL)
    {
        Font *font = font_system(15, 0);
        const char_t *family = font_family(font);
        i_SYSTEM_FONT_FAMILY = str_c(family);
        font_destroy(&font);
    }

    if (str_equ(i_SYSTEM_FONT_FAMILY, family) == TRUE)
        return ekFONT_FAMILY_SYSTEM;

    if (i_MONOSPACE_FONT_FAMILY == NULL)
    {
        Font *font = font_monospace(15, 0);
        const char_t *family = font_family(font);
        i_MONOSPACE_FONT_FAMILY = str_c(family);
        font_destroy(&font);
    }

    if (str_equ(i_MONOSPACE_FONT_FAMILY, family) == TRUE)
        return ekFONT_FAMILY_MONOSPACE;

    return ENUM_MAX(font_family_t);
}

/*---------------------------------------------------------------------------*/

void osfont_metrics(const OSFont *font, const real32_t size, const real32_t xscale, real32_t *ascent, real32_t *descent, real32_t *leading, real32_t *cell_size, real32_t *avg_width, bool_t *monospace)
{
    NSFont *nsfont = cast(font, NSFont);

    if (ascent != NULL)
        *ascent = (real32_t)[nsfont ascender];

    if (descent != NULL)
        *descent = (real32_t) - [nsfont descender];

    /* We need to get a real text measure */
    if (leading != NULL || cell_size != NULL || avg_width != NULL)
    {
        real32_t width, height;
        uint32_t len;
        const char_t *str = draw2d_str_avg_char_width(&len);
        osfont_extents(font, str, xscale, -1, &width, &height);

        if (leading != NULL)
            *leading = height - size;

        if (cell_size != NULL)
            *cell_size = height;

        if (avg_width != NULL)
            *avg_width = width / len;
    }

    if (monospace != NULL)
        *monospace = (bool_t)[nsfont isFixedPitch];
}

/*---------------------------------------------------------------------------*/

void osfont_extents(const OSFont *font, const char_t *text, const real32_t xscale, const real32_t refwidth, real32_t *width, real32_t *height)
{
    id objects[1];
    id keys[1];
    MeasureStr data;
    NSUInteger count = sizeof(objects) / sizeof(id);
    cassert_no_null(font);
    cassert(count == 1);
    unref(xscale);
    objects[0] = cast(font, NSFont);
    keys[0] = NSFontAttributeName;
    data.dict = [NSDictionary dictionaryWithObjects:objects forKeys:keys count:count];
    draw2d_extents(&data, draw_word_extents, TRUE, text, refwidth, width, height, MeasureStr);
}

/*---------------------------------------------------------------------------*/

const void *osfont_native(const OSFont *font)
{
    return cast(font, void);
}

/*---------------------------------------------------------------------------*/

bool_t font_exists_family(const char_t *ffamily)
{
    NSFontManager *fontManager = [NSFontManager sharedFontManager];
    NSArray *families = nil;
    NSUInteger i, count;
    cassert_no_null(fontManager);
    families = [fontManager availableFontFamilies];
    count = [families count];
    for (i = 0; i < count; ++i)
    {
        NSString *family = cast([families objectAtIndex:i], NSString);
        const char_t *family_str = [family UTF8String];
        if (str_equ_c(ffamily, family_str) == TRUE)
            return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static ArrPt(String) *i_installed_families(const bool_t only_mono)
{
    NSFontManager *fontManager = [NSFontManager sharedFontManager];
    NSArray *families = nil;
    NSUInteger i, count;
    ArrPt(String) *font_families = NULL;
    cassert_no_null(fontManager);
    families = [fontManager availableFontFamilies];
    count = [families count];
    font_families = arrpt_create(String);
    for (i = 0; i < count; ++i)
    {
        NSString *family = cast([families objectAtIndex:i], NSString);
        BOOL add = YES;

        if (only_mono == TRUE)
        {
            NSFont *nsfont = [fontManager fontWithFamily:family traits:0 weight:5 size:20];
            add = [nsfont isFixedPitch];
        }

        if (add == YES)
        {
            const char_t *family_str = [family UTF8String];
            String *ffamily = str_c(family_str);
            arrpt_append(font_families, ffamily, String);
        }
    }

    arrpt_sort(font_families, str_scmp, String);
    return font_families;
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
