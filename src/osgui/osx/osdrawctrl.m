/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.m
 *
 */

/* Drawing custom GUI controls */

#include "nowarn.hxx"
#include <Cocoa/Cocoa.h>
#include "warn.hxx"

#include "osdrawctrl.h"
#include "oscolor.inl"
#include "osglobals.inl"
#include "osx/draw2d_osx.inl"
#include "osx/draw2d_osx.ixx"
#include "draw.h"
#include "draw.inl"
#include "dctx.inl"
#include "cassert.h"
#include "font.h"
#include "image.inl"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

Font *osdrawctrl_font(const DCtx *ctx)
{
    unref(ctx);
    return font_system(font_regular_size(), 0);
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_row_padding(const DCtx *ctx)
{
    unref(ctx);
    return 3;
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_check_width(const DCtx *ctx)
{
    unref(ctx);
    return osglobals_check_width();
}

/*---------------------------------------------------------------------------*/

uint32_t osdrawctrl_check_height(const DCtx *ctx)
{
    unref(ctx);
    return osglobals_check_height();
}

/*---------------------------------------------------------------------------*/

multisel_t osdrawctrl_multisel(const DCtx *ctx, const vkey_t key)
{
    unref(ctx);
    if (key == ekKEY_LWIN || key == ekKEY_RWIN)
        return ekMULTISEL_SINGLE;
    else if (key == ekKEY_LSHIFT || key == ekKEY_RSHIFT)
        return ekMULTISEL_BURST;
    return ekMULTISEL_NO;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_clear(DCtx *ctx)
{
    uint32_t width, height;
    CGRect rect;
    CGFloat r, g, b, a;
    
    cassert_no_null(ctx);
    
    if (ctx->raster_mode == FALSE)
        draw_raster_mode(ctx);
    
    dctx_size(ctx, &width, &height);
    rect.origin.x = 0;
    rect.origin.y = 0;
    rect.size.width = (CGFloat)width;
    rect.size.height = (CGFloat)height;
    oscolor_NSColor_rgba([NSColor controlBackgroundColor], &r, &g, &b, &a);
    CGContextSetRGBFillColor(ctx->context, r, g, b, a);
    CGContextFillRect(ctx->context, rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    CGRect rect;
    const CGFloat *color = NULL;
    
    cassert_no_null(ctx);
    if (ctx->raster_mode == FALSE)
        draw_raster_mode(ctx);

    switch (state) {
    case ekCSTATE_NORMAL:
        color = osglobals_back_color();
        break;
            
    case ekCSTATE_HOT:
        color = osglobals_hotbg_color();
        break;

    case ekCSTATE_PRESSED:
	    color = osglobals_selbg_color();
    	break;
            
    case ekCSTATE_BKNORMAL:
        color = osglobals_backbackdrop_color();
        break;
            
    case ekCSTATE_BKHOT:
        color = osglobals_hotbgbackdrop_color();
        break;
            
    case ekCSTATE_BKPRESSED:
        color = osglobals_selbgbackdrop_color();
        break;
            
    case ekCSTATE_DISABLED:
        color = osglobals_backbackdrop_color();
        break;
            
    cassert_default();
    }

    rect.origin.x = (CGFloat)x;
    rect.origin.y = (CGFloat)y;
    rect.size.width = (CGFloat)width;
    rect.size.height = (CGFloat)height;
    CGContextSetRGBFillColor(ctx->context, color[0], color[1], color[2], color[3]);
    CGContextFillRect(ctx->context, rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state)
{
    NSRect rect;
    const CGFloat *color = nil;
    NSColor *nscolor = nil;
    NSString *str = nil;
    
    cassert_no_null(ctx);
        
    switch (state) {
    case ekCSTATE_NORMAL:
        color = osglobals_text_color();
        break;
        
    case ekCSTATE_HOT:
        color = osglobals_hottx_color();
        break;
        
    case ekCSTATE_PRESSED:
        color = osglobals_seltx_color();
        break;
        
    case ekCSTATE_BKNORMAL:
        color = osglobals_textbackdrop_color();
        break;
        
    case ekCSTATE_BKHOT:
        color = osglobals_hottxbackdrop_color();
        break;
        
    case ekCSTATE_BKPRESSED:
        color = osglobals_seltxbackdrop_color();
        break;
        
    case ekCSTATE_DISABLED:
        color = osglobals_textbackdrop_color();
        break;
        
    cassert_default();
    }
    
    nscolor = [NSColor colorWithCalibratedRed:color[0] green:color[1] blue:color[2] alpha:color[3]];
    [ctx->text_dict setObject:nscolor forKey:NSForegroundColorAttributeName];
    str = drawctrl_begin_text(ctx, text, (real32_t)x, (real32_t)y, &rect);
    [str drawInRect:rect withAttributes:ctx->text_dict];    
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    unref(ctx);
    unref(x);
    unref(y);
    unref(width);
    unref(height);
    unref(state);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y, const cstate_t state)
{
    const OSImage *osimage = image_native(image);
    unref(state);
    draw_imgimp(ctx, osimage, UINT32_MAX, (real32_t)x, (real32_t)y, TRUE);
}

/*---------------------------------------------------------------------------*/

static void i_draw_checkbox(DCtx *ctx, const real32_t x, const real32_t y, const bool_t pressed, const cstate_t state)
{
    NSImage *image = osglobals_checkbox_image(pressed, state);
    NSRect rect;
    cassert_no_null(ctx);

    if (ctx->raster_mode == FALSE)
        draw_raster_mode(ctx);

    rect.origin = NSMakePoint((CGFloat)x, (CGFloat)y);
    rect.size = osglobals_check_size();

    #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        #if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
        NSCompositingOperation op = NSCompositingOperationSourceOver;
        #else
        NSCompositingOperation op = NSCompositeSourceOver;
        #endif
        [image drawInRect:rect fromRect:osglobals_check_rect() operation:op fraction:1.0f respectFlipped:YES hints:nil];
    #else
        #error Usar NSImage IsFlipped = TRUE y despues restaurar isFlipped = false;
    #endif
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    cassert_unref(width == osglobals_check_width(), width);
    cassert_unref(height == osglobals_check_height(), height);
	i_draw_checkbox(ctx, (real32_t)x, (real32_t)y, TRUE, state);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    cassert_unref(width == osglobals_check_width(), width);
    cassert_unref(height == osglobals_check_height(), height);
	i_draw_checkbox(ctx, (real32_t)x, (real32_t)y, FALSE, state);
}
