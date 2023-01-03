/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: oscolor.m
 *
 */

/* Color */

#include "nowarn.hxx"
#include <Cocoa/Cocoa.h>
#include "warn.hxx"

#include "color.h"
#include "oscolor.inl"
#include "cassert.h"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

static void i_NSColor_rgba(NSColor *c, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a)
{
#if defined (MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8
    /* [NSColor CGColor] --> 10.8 */
    CGColorRef cr = [c CGColor];
    CGColorSpaceRef cs = CGColorGetColorSpace(cr);
    CGColorSpaceModel m = CGColorSpaceGetModel(cs);
    switch (m) {
        case kCGColorSpaceModelRGB:
        {
            const CGFloat *comps = CGColorGetComponents(cr);
            cassert(CGColorGetNumberOfComponents(cr) == 4);
            *r = comps[0];
            *g = comps[1];
            *b = comps[2];
            *a = comps[3];
            return;
        }
            
        case kCGColorSpaceModelMonochrome:
        {
            const CGFloat *comps = CGColorGetComponents(cr);
            cassert(CGColorGetNumberOfComponents(cr) == 2);
            *r = comps[0];
            *g = comps[0];
            *b = comps[0];
            *a = comps[1];
            return;
        }
            
        case kCGColorSpaceModelCMYK:
        {
            const CGFloat *comps = CGColorGetComponents(cr);
            cassert(CGColorGetNumberOfComponents(cr) == 5);
            *r = (1 - comps[0]) * (1 - comps[3]);
            *g = (1 - comps[1]) * (1 - comps[3]);
            *b = (1 - comps[2]) * (1 - comps[3]);
            *a = comps[4];
            return;
        }
            
        default:
            cassert_msg(FALSE, "Unknown color space 'osglobals_color'");
    }
    
    *r = 0;
    *g = 0;
    *b = 0;
    *a = 0;
    return;
#else
    
    /* https://gist.github.com/aquarius/5344499 */
    NSColor *nc = nil;
    
    if ([[c colorSpaceName] isEqualToString:NSCalibratedRGBColorSpace])
    {
        nc = c;
    }
	else
    {
        /* Try to convert to RGB */
        nc = [c colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
    }
    
    if (nc == nil)
    {
        /* Convert to image and extract first px. The result won't be 100% correct,
         but better than a completely undefined color */
        NSBitmapImageRep *bitmapRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:1 pixelsHigh:1 bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:4 bitsPerPixel:32];
        NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithBitmapImageRep:bitmapRep];
        [NSGraphicsContext saveGraphicsState];
        [NSGraphicsContext setCurrentContext:context];
        [c setFill];
        NSRectFill(NSMakeRect(0, 0, 1, 1));
        [context flushGraphics];
        [NSGraphicsContext restoreGraphicsState];
        nc = [bitmapRep colorAtX:0 y:0];
        [bitmapRep release];
    }
    
    [nc getRed:r green:g blue:b alpha:a];
#endif
}

/*---------------------------------------------------------------------------*/

static color_t i_from_NSColor(NSColor *c)
{
    CGFloat r, g, b, a;
    i_NSColor_rgba(c, &r, &g, &b, &a);
    return color_rgbaf((real32_t)r, (real32_t)g, (real32_t)b, (real32_t)a);
}

/*---------------------------------------------------------------------------*/

color_t oscolor_from_NSColor(NSColor *color)
{
    return i_from_NSColor(color);
}

/*---------------------------------------------------------------------------*/

void oscolor_NSColor_rgba(NSColor *color, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a)
{
    i_NSColor_rgba(color, r, g, b, a);
}

/*---------------------------------------------------------------------------*/

NSColor *oscolor_NSColor(color_t color)
{
    real32_t r, g, b, a;
    color_get_rgbaf(color, &r, &g, &b, &a);
    return [NSColor colorWithCalibratedRed:(CGFloat)r green:(CGFloat)g blue:(CGFloat)b alpha:(CGFloat)a];
}
