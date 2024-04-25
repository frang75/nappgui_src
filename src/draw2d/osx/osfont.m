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
#include <sewer/cassert.h>

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

static String *i_SYSTEM_FONT_FAMILY = NULL;
static String *i_MONOSPACE_FONT_FAMILY = NULL;

/*---------------------------------------------------------------------------*/

void osfont_alloc_globals(void)
{
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
        /* From Catalina, we have a system predefined monospace font */
#if defined(MAC_OS_X_VERSION_10_15) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
        cassert(FALSE);
        return NULL;
#else
        const char_t *desired_fonts[] = {"SF Mono", "Menlo", "Monaco", "Andale Mono", "Courier New"};
        const char_t *monofont = draw2d_monospace_family(desired_fonts, sizeof(desired_fonts) / sizeof(const char_t *));
        i_MONOSPACE_FONT_FAMILY = str_c(monofont);
#endif
    }

    return tc(i_MONOSPACE_FONT_FAMILY);
}

/*---------------------------------------------------------------------------*/

static NSFont *i_convent_to_italic(NSFont *font, const CGFloat height, NSFontManager *font_manager)
{
    NSFont *italic_font = nil;
    NSFontTraitMask fontTraits = (NSFontTraitMask)0;
    cassert_no_null(font);

    italic_font = [font_manager convertFont:font toHaveTrait:NSItalicFontMask];
    fontTraits = [font_manager traitsOfFont:italic_font];

    if ((fontTraits & NSItalicFontMask) == 0)
    {
        NSAffineTransform *font_transform = [NSAffineTransform transform];
        [font_transform scaleBy:height];

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

        italic_font = [NSFont fontWithDescriptor:[italic_font fontDescriptor] textTransform:font_transform];
    }

    return italic_font;
}

/*---------------------------------------------------------------------------*/

OSFont *osfont_create(const char_t *family, const real32_t size, const uint32_t style)
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
        if (style & ekFBOLD)
            nsfont = [NSFont monospacedSystemFontOfSize:(CGFloat)size weight:NSFontWeightBold];
        else
            nsfont = [NSFont monospacedSystemFontOfSize:(CGFloat)size weight:NSFontWeightLight];
#else
        name = i_monospace_font_family();
#endif
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

    cassert_fatal_msg(nsfont != nil, "Font is not available on this computer.");

    if (nsfont != nil)
    {
        if (style & ekFITALIC)
        {
            NSFontManager *fontManager = [NSFontManager sharedFontManager];
            nsfont = i_convent_to_italic(nsfont, (CGFloat)size, fontManager);
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

void osfont_metrics(const OSFont *font, const real32_t size, real32_t *ascent, real32_t *descent, real32_t *leading, real32_t *cell_size, bool_t *monospace)
{
    NSFont *nsfont = cast(font, NSFont);

    if (ascent != NULL)
        *ascent = (real32_t)[nsfont ascender];

    if (descent != NULL)
        *descent = (real32_t) - [nsfont descender];

    if (leading != NULL)
    {
        real32_t width, height;
        osfont_extents(font, "O", -1, &width, &height);
        unref(width);
        *leading = height - size;
    }

    if (cell_size != NULL)
    {
        real32_t width;
        osfont_extents(font, "O", -1, &width, cell_size);
        unref(width);
    }

    if (monospace != NULL)
        *monospace = (bool_t)[nsfont isFixedPitch];
}

/*---------------------------------------------------------------------------*/

void osfont_extents(const OSFont *font, const char_t *text, const real32_t refwidth, real32_t *width, real32_t *height)
{
    id objects[1];
    id keys[1];
    MeasureStr data;
    NSUInteger count = sizeof(objects) / sizeof(id);
    cassert_no_null(font);
    cassert(count == 1);
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
