/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
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
#include "draw.h"
#include "dctxh.h"
#include "cassert.h"
#include "color.h"
#include "font.h"

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

ctrl_msel_t osdrawctrl_multisel(const DCtx *ctx, const vkey_t key)
{
    unref(ctx);
    if (key == ekKEY_LWIN || key == ekKEY_RWIN)
        return ekCTRL_MSEL_SINGLE;
    else if (key == ekKEY_LSHIFT || key == ekKEY_RSHIFT)
        return ekCTRL_MSEL_BURST;
    return ekCTRL_MSEL_NO;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_clear(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const enum_t nonused)
{
    CGRect rect;
    CGFloat r, g, b, a;
    uint32_t nwidth, nheight;
    CGContextRef cgctx = (CGContextRef)dctx_native(ctx);
    dctx_set_raster_mode(ctx);
    dctx_size(ctx, &nwidth, &nheight);
    rect.origin.x = (CGFloat)x;
    rect.origin.y = (CGFloat)y;
    rect.size.width = (CGFloat)nwidth;
    rect.size.height = (CGFloat)nheight;
    oscolor_NSColor_rgba([NSColor controlBackgroundColor], &r, &g, &b, &a);
    CGContextSetRGBFillColor(cgctx, r, g, b, a);
    CGContextFillRect(cgctx, rect);
    unref(width);
    unref(height);
    unref(nonused);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_header(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    /* TODO */
    unref(ctx);
    unref(x);
    unref(y);
    unref(width);
    unref(height);
    unref(state);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_indicator(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const indicator_t indicator)
{
    /* TODO */
    unref(ctx);
    unref(x);
    unref(y);
    unref(width);
    unref(height);
    unref(indicator);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_fill(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    CGRect rect;
    const CGFloat *color = NULL;
    CGContextRef cgctx = (CGContextRef)dctx_native(ctx);
    
    cassert_no_null(ctx);
    dctx_set_raster_mode(ctx);

    switch (state) {
    case ekCTRL_STATE_NORMAL:
        color = osglobals_back_color();
        break;
            
    case ekCTRL_STATE_HOT:
        color = osglobals_hotbg_color();
        break;

    case ekCTRL_STATE_PRESSED:
	    color = osglobals_selbg_color();
    	break;
            
    case ekCTRL_STATE_BKNORMAL:
        color = osglobals_backbackdrop_color();
        break;
            
    case ekCTRL_STATE_BKHOT:
        color = osglobals_hotbgbackdrop_color();
        break;
            
    case ekCTRL_STATE_BKPRESSED:
        color = osglobals_selbgbackdrop_color();
        break;
            
    case ekCTRL_STATE_DISABLED:
        color = osglobals_backbackdrop_color();
        break;
            
    cassert_default();
    }

    rect.origin.x = (CGFloat)x;
    rect.origin.y = (CGFloat)y;
    rect.size.width = (CGFloat)width;
    rect.size.height = (CGFloat)height;
    CGContextSetRGBFillColor(cgctx, color[0], color[1], color[2], color[3]);
    CGContextFillRect(cgctx, rect);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_text(DCtx *ctx, const char_t *text, const int32_t x, const int32_t y, const ctrl_state_t state)
{
    const CGFloat *color = nil;
    color_t ncolor = 0;
    
    cassert_no_null(ctx);
        
    switch (state) {
    case ekCTRL_STATE_NORMAL:
        color = osglobals_text_color();
        break;
        
    case ekCTRL_STATE_HOT:
        color = osglobals_hottx_color();
        break;
        
    case ekCTRL_STATE_PRESSED:
        color = osglobals_seltx_color();
        break;
        
    case ekCTRL_STATE_BKNORMAL:
        color = osglobals_textbackdrop_color();
        break;
        
    case ekCTRL_STATE_BKHOT:
        color = osglobals_hottxbackdrop_color();
        break;
        
    case ekCTRL_STATE_BKPRESSED:
        color = osglobals_seltxbackdrop_color();
        break;
        
    case ekCTRL_STATE_DISABLED:
        color = osglobals_textbackdrop_color();
        break;
        
    cassert_default();
    }
    
    ncolor = color_rgbaf((real32_t)color[0], (real32_t)color[1], (real32_t)color[2], (real32_t)color[3]);
    draw_text_color(ctx, ncolor);
	dctx_text_raster(ctx, text, (real32_t)x, (real32_t)y);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_focus(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    unref(ctx);
    unref(x);
    unref(y);
    unref(width);
    unref(height);
    unref(state);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_line(DCtx *ctx, const int32_t x0, const int32_t y0, const int32_t x1, const int32_t y1)
{
    /* TODO */
    unref(ctx);
    unref(x0);
    unref(y0);
    unref(x1);
    unref(y1);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_image(DCtx *ctx, const Image *image, const int32_t x, const int32_t y, const ctrl_state_t state)
{
	dctx_image_raster(ctx, image, (real32_t)x, (real32_t)y);
    unref(state);
}

/*---------------------------------------------------------------------------*/

static void i_draw_checkbox(DCtx *ctx, const real32_t x, const real32_t y, const bool_t pressed, const ctrl_state_t state)
{
    NSImage *image = osglobals_checkbox_image(pressed, state);
    NSRect rect;
    cassert_no_null(ctx);

    dctx_set_raster_mode(ctx);
    rect.origin = NSMakePoint((CGFloat)x, (CGFloat)y);
    rect.size = osglobals_check_size();

    {
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
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_checkbox(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    cassert_unref(width == osglobals_check_width(), width);
    cassert_unref(height == osglobals_check_height(), height);
	i_draw_checkbox(ctx, (real32_t)x, (real32_t)y, TRUE, state);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_uncheckbox(DCtx *ctx, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height, const ctrl_state_t state)
{
    cassert_unref(width == osglobals_check_width(), width);
    cassert_unref(height == osglobals_check_height(), height);
	i_draw_checkbox(ctx, (real32_t)x, (real32_t)y, FALSE, state);
}
