/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx_osx.m
 *
 */

/* Draw context */

#include "nowarn.hxx"
#include <Cocoa/Cocoa.h>
#include "warn.hxx"

#include "dctx.h"
#include "dctx.inl"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "font.inl"
#include "heap.h"
#include "ptr.h"
#include "draw2d_osx.ixx"
#include "dctx_osx.inl"

#if !defined (__MACOS__)
#error This file is only for OSX
#endif

/*---------------------------------------------------------------------------*/

static void i_color(const color_t c, CGFloat *r, CGFloat *g, CGFloat *b, CGFloat *a)
{
    real32_t r1, g1, b1, a1;
    color_get_rgbaf(c, &r1, &g1, &b1, &a1);
    *r = (CGFloat)r1;
    *g = (CGFloat)g1;
    *b = (CGFloat)b1;
    *a = (CGFloat)a1;
}

/*---------------------------------------------------------------------------*/

NSTextAlignment dctx_text_alignment(const align_t halign)
{
#if defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    switch (halign)
    {
        case ekLEFT:
            return NSTextAlignmentLeft;
        case ekCENTER:
            return NSTextAlignmentCenter;
        case ekJUSTIFY:
            return NSTextAlignmentJustified;
        case ekRIGHT:
            return NSTextAlignmentRight;
        cassert_default();
    }
    return NSTextAlignmentLeft;
    
#else
    switch (halign)
    {
        case ekLEFT:
            return NSLeftTextAlignment;
        case ekCENTER:
            return NSCenterTextAlignment;
        case ekJUSTIFY:
            return NSJustifiedTextAlignment;
        case ekRIGHT:
            return NSRightTextAlignment;
        cassert_default();
    }

    return NSLeftTextAlignment;
#endif
}

/*---------------------------------------------------------------------------*/

static void i_init_text_attr(DCtx *ctx)
{
    cassert(ctx->text_parag == NULL);
    cassert(ctx->text_dict == NULL);
    ctx->text_parag = [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] retain];
    ctx->text_parag.lineBreakMode = NSLineBreakByWordWrapping;
    id objects[5];
    id keys[] = {
                NSUnderlineStyleAttributeName,
                NSStrikethroughStyleAttributeName,
                NSParagraphStyleAttributeName,
                NSForegroundColorAttributeName,
                NSFontAttributeName};
    
    objects[0] = kUNDERLINE_NONE;
    objects[1] = kUNDERLINE_NONE;
    objects[2] = ctx->text_parag;
    objects[3] = [NSColor blackColor];
    objects[4] = [NSFont systemFontOfSize:[NSFont systemFontSize]];

    ctx->text_dict = [[NSMutableDictionary alloc] initWithObjects:objects forKeys:keys count:5];
}

/*---------------------------------------------------------------------------*/

DCtx *dctx_create(void *custom_data)
{
    DCtx *ctx = heap_new0(DCtx);
    cassert(custom_data == NULL);
    unref(custom_data);
    ctx->line_width = 1;
    ctx->gradient_matrix = CGAffineTransformIdentity;
    i_init_text_attr(ctx);
    return ctx;
}

/*---------------------------------------------------------------------------*/

DCtx *dctx_bitmap(const uint32_t width, const uint32_t height, const pixformat_t format)
{
    DCtx *ctx = heap_new0(DCtx);
    CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    byte_t *pixdata = heap_malloc(width * height * 4, "OSXBitmapContextData");
    NSGraphicsContext *nscontext = nil;
    ctx->context = CGBitmapContextCreate((void*)pixdata, (size_t)width, (size_t)height, 8, (size_t)(width * 4), space, (CGBitmapInfo)kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(space);
    
#if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
	nscontext = [NSGraphicsContext graphicsContextWithCGContext:ctx->context flipped:YES];
#else
    nscontext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx->context flipped:YES];
#endif
    
    [NSGraphicsContext setCurrentContext:nscontext];
    ctx->format = format;
    ctx->width = (uint32_t)CGBitmapContextGetWidth(ctx->context);
    ctx->height = (uint32_t)CGBitmapContextGetHeight(ctx->context);
    ctx->is_flipped = YES;
    ctx->origin = CGAffineTransformMake(1, 0, 0, -1, 0, (CGFloat)height);
    ctx->line_width = 1;
    ctx->gradient_matrix = CGAffineTransformIdentity;
    CGContextConcatCTM(ctx->context, ctx->origin);
    i_init_text_attr(ctx);
    dctx_init(ctx);
    return ctx;
}

/*---------------------------------------------------------------------------*/

void dctx_destroy(DCtx **ctx)
{
    cassert_no_null(ctx);
    cassert_no_null(*ctx);
        
    if ((*ctx)->gradient != NULL)
        CGGradientRelease((*ctx)->gradient);

    if ((*ctx)->context != NULL)
    {
        CGContextRelease((*ctx)->context);
    	(*ctx)->context = NULL;
    }
    
    [(*ctx)->text_dict release];
    [(*ctx)->text_parag release];

    heap_delete(ctx, DCtx);
}

/*---------------------------------------------------------------------------*/

static __INLINE CGContextRef i_CGContext(NSGraphicsContext *nscontext)
{
    #if defined (MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
    return [nscontext CGContext];
    #else
    return (CGContextRef)[nscontext graphicsPort];
    #endif
}

/*---------------------------------------------------------------------------*/

void dctx_set_gcontext(DCtx *ctx, void *gcontext, const uint32_t width, const uint32_t height, const real32_t offset_x, const real32_t offset_y, const uint32_t background, const bool_t reset)
{
    cassert_no_null(ctx);
    cassert(ctx->context == NULL);
    unref(offset_x);
    unref(offset_y);
    unref(background);
    ctx->width = width;
    ctx->height = height;
    ctx->context = i_CGContext((NSGraphicsContext*)gcontext);
    ctx->origin = CGContextGetCTM(ctx->context);
    ctx->raster_mode = FALSE;
    if (reset == TRUE)
        dctx_init(ctx);
}

/*---------------------------------------------------------------------------*/

void dctx_unset_gcontext(DCtx *ctx)
{
    cassert_no_null(ctx);
    cassert(ctx->context != NULL);
    ctx->context = NULL;
}

/*---------------------------------------------------------------------------*/

void dctx_size(const DCtx *ctx, uint32_t *width, uint32_t *height)
{
    cassert_no_null(ctx);
    ptr_assign(width, ctx->width);
    ptr_assign(height, ctx->height);
}

/*---------------------------------------------------------------------------*/

void dctx_transform(DCtx *ctx, const T2Df *t2d, const bool_t cartesian)
{
    CGAffineTransform transform;
    cassert_no_null(ctx);
    cassert_no_null(t2d);
    transform.a = (CGFloat)t2d->i.x;
    transform.b = (CGFloat)t2d->i.y;
    transform.c = (CGFloat)t2d->j.x;
    transform.d = (CGFloat)t2d->j.y;
    transform.tx = (CGFloat)t2d->p.x;
    transform.ty = (CGFloat)t2d->p.y;
    ctx->transform = transform;
    ctx->cartesian_system = cartesian;
    
    if (ctx->raster_mode == FALSE)
    {
        /* Invalidate previous transform. Equivalent to hypothetical SetIdentity() */
        CGAffineTransform curtrans = CGContextGetCTM(ctx->context);
        curtrans = CGAffineTransformInvert(curtrans);
        CGContextConcatCTM(ctx->context, curtrans);
    
        /* Apply new transform */
        CGContextConcatCTM(ctx->context, ctx->origin);
        CGContextConcatCTM(ctx->context, ctx->transform);
    }
}

/*---------------------------------------------------------------------------*/

void _dctx_gradient_transform(DCtx *ctx)
{
    unref(ctx);
}

/*---------------------------------------------------------------------------*/

void draw_clear(DCtx *ctx, const color_t color)
{
    cassert_no_null(ctx);
    if (color != 0)
    {
        uint32_t width, height;
        dctx_size(ctx, &width, &height);
        CGRect rect;
        CGFloat r, g, b, a;
        rect.origin.x = 0;
        rect.origin.y = 0;
        rect.size.width = (CGFloat)width;
        rect.size.height = (CGFloat)height;
        i_color(color, &r, &g, &b, &a);
        CGContextSetRGBFillColor(ctx->context, r, g, b, a);
        CGContextFillRect(ctx->context, rect);
        i_color(ctx->fillcolor, &r, &g, &b, &a);
        CGContextSetRGBFillColor(ctx->context, r, g, b, a);
    }
}

/*---------------------------------------------------------------------------*/

void draw_antialias(DCtx *ctx, const bool_t on)
{
    cassert_no_null(ctx);
    CGContextSetShouldAntialias(ctx->context, (bool)on);
}

