/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dctx_gtk.c
 *
 */

/* Draw context */

#include "dctx.h"
#include "dctx.inl"
#include "dctx_gtk.inl"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "font.inl"
#include "heap.h"
#include "ptr.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

//#include <gtk/gtk.h>
#include "draw2d_gtk.ixx"

/*---------------------------------------------------------------------------*/

DCtx *dctx_create(void *custom_data)
{
    DCtx *ctx = heap_new0(DCtx);
    cassert_unref(custom_data == NULL, custom_data);
    //ctx->widget = (GtkWidget*)view;
//    GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(view));
//    int w = gtk_widget_get_allocated_width((GtkWidget*)view);
//    int h = gtk_widget_get_allocated_height((GtkWidget*)view);
//    ctx->surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, w, h);
    //ctx->font = font_system(font_regular_size(), 0);
    cairo_matrix_init_identity(&ctx->transform);
    cairo_matrix_init_identity(&ctx->pattern_matrix);
    return ctx;
}

/*---------------------------------------------------------------------------*/

void dctx_update_view(DCtx *ctx, void *view)
{
    cassert_no_null(ctx);
    unref(view);
    //ctx->widget = (GtkWidget*)view;
    /*GdkWindow *window = gtk_widget_get_window(GTK_WIDGET(view));
    int w = gtk_widget_get_allocated_width((GtkWidget*)view);
    int h = gtk_widget_get_allocated_height((GtkWidget*)view);
    cassert_no_null(ctx->surface);
    cassert(ctx->cairo == NULL);
    cairo_surface_destroy(ctx->surface);
    ctx->surface = gdk_window_create_similar_surface(window, CAIRO_CONTENT_COLOR, w, h);*/
}

/*---------------------------------------------------------------------------*/

DCtx *dctx_bitmap(const uint32_t width, const uint32_t height, const pixformat_t format)
{
    DCtx *ctx = heap_new0(DCtx);
    cairo_format_t pf = CAIRO_FORMAT_INVALID;

    switch (format) {
//    case ekINDEX1:
//    case ekINDEX4:
//    case ekINDEX8:
//    case ekGRAY4:
    case ekGRAY8:
    case ekRGB24:
        pf = CAIRO_FORMAT_RGB24;
        break;
    case ekRGBA32:
        pf = CAIRO_FORMAT_ARGB32;
        break;
    cassert_default();
    }

    ctx->format = format;
    ctx->width = width;
    ctx->height = height;
    ctx->surface = cairo_image_surface_create(pf, (int)width, (int)height);
    ctx->cairo = cairo_create(ctx->surface);
    cairo_matrix_init_identity(&ctx->origin);
    cairo_matrix_init_identity(&ctx->transform);
    cairo_matrix_init_identity(&ctx->pattern_matrix);
    dctx_init(ctx);
    return ctx;
}

/*---------------------------------------------------------------------------*/

void dctx_destroy(DCtx **ctx)
{
    cassert_no_null(ctx);
    cassert_no_null(*ctx);

    if ((*ctx)->surface != NULL)
    {
        cairo_surface_destroy((*ctx)->surface);
        cairo_destroy((*ctx)->cairo);
    }
    else
    {
        cassert((*ctx)->cairo == NULL);
    }

    if ((*ctx)->lpattern != NULL)
        cairo_pattern_destroy((*ctx)->lpattern);

    if ((*ctx)->layout != NULL)
        g_object_unref((*ctx)->layout);

    if ((*ctx)->font != NULL)
        font_destroy(&(*ctx)->font);

    heap_delete(ctx, DCtx);
}

/*---------------------------------------------------------------------------*/

void dctx_set_gcontext(DCtx *ctx, void *gcontext, const uint32_t width, const uint32_t height, const real32_t offset_x, const real32_t offset_y, const uint32_t background, const bool_t reset)
{
    void **dctx = (void**)gcontext;
    cassert_no_null(ctx);
    cassert_no_null(gcontext);
    unref(background);
    unref(offset_x);
    unref(offset_y);
    ctx->width = width;
    ctx->height = height;
    ctx->cairo = (cairo_t*)dctx[0];
    ctx->scroll_x = *(double*)dctx[1];
    ctx->scroll_y = *(double*)dctx[2];
    ctx->total_width = *(double*)dctx[3];
    ctx->total_height = *(double*)dctx[4];
    ctx->clip_width = *(double*)dctx[5];
    ctx->clip_height = *(double*)dctx[6];

    cairo_get_matrix(ctx->cairo, &ctx->origin);
    ctx->raster_mode = FALSE;
    if (reset == TRUE)
        dctx_init(ctx);
}

/*---------------------------------------------------------------------------*/

void dctx_unset_gcontext(DCtx *ctx)
{
    cassert_no_null(ctx);
    cassert_unref(ctx->cairo != NULL, ctx);
    ctx->cairo = NULL;
}

/*---------------------------------------------------------------------------*/

void dctx_size(const DCtx *ctx, uint32_t *width, uint32_t *height)
{
    cassert_no_null(ctx);
    ptr_assign(width, ctx->width);
    ptr_assign(height, ctx->height);

//    if (ctx->widget != NULL)
//    {
//        GtkAllocation alloc;
//
//#if GTK_CHECK_VERSION(3, 20, 0)
//        gtk_widget_get_allocated_size(ctx->widget, &alloc, NULL);
//#else
//        gtk_widget_get_allocation(ctx->widget, &alloc);
//#endif
//        if (width != NULL)
//            *width = (real32_t)alloc.width;
//
//        if (height != NULL)
//            *height = (real32_t)alloc.height;
//    }
//    else
//    {
//        cassert(ctx->surface != NULL);
//        if (width != NULL)
//            *width = (real32_t)cairo_image_surface_get_width(ctx->surface);
//
//        if (height != NULL)
//            *height = (real32_t)cairo_image_surface_get_height(ctx->surface);
//    }
}

/*---------------------------------------------------------------------------*/

void dctx_transform(DCtx *ctx, const T2Df *t2d, const bool_t cartesian)
{
    cairo_matrix_t transform;
    cassert_no_null(ctx);
    cassert_no_null(t2d);
    transform.xx = (double)t2d->i.x;
    transform.yx = (double)t2d->i.y;
    transform.xy = (double)t2d->j.x;
    transform.yy = (double)t2d->j.y;
    transform.x0 = (double)t2d->p.x;
    transform.y0 = (double)t2d->p.y;
    ctx->transform = transform;
    ctx->cartesian_system = cartesian;

    if (ctx->raster_mode == FALSE)
    {
        cairo_set_matrix(ctx->cairo, &ctx->origin);
        cairo_transform(ctx->cairo, &transform);
        _dctx_gradient_transform(ctx);
    }
}

/*---------------------------------------------------------------------------*/

void _dctx_gradient_transform(DCtx *ctx)
{
    cassert_no_null(ctx);
    if (ctx->lpattern != NULL)
    {
        cairo_matrix_t matrix = ctx->pattern_matrix;
        cairo_matrix_invert(&matrix);
        cairo_matrix_multiply(&matrix, &ctx->transform, &matrix);
        cairo_pattern_set_matrix(ctx->lpattern, &matrix);
    }
}

/*---------------------------------------------------------------------------*/

cairo_t *_dctx_cairo(DCtx *ctx)
{
    cassert_no_null(ctx);
    return ctx->cairo;
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_color(cairo_t *cairo, const color_t color, color_t *source_color)
{
    //if (color != *source_color)
    {
        real32_t r, g, b, a;
        color_get_rgbaf(color, &r, &g, &b, &a);
        cairo_set_source_rgba(cairo, (double)r, (double)g, (double)b, (double)a);
        *source_color = color;
    }
}

/*---------------------------------------------------------------------------*/

void draw_clear(DCtx *ctx, const color_t color)
{
    cassert_no_null(ctx);
    i_color(ctx->cairo, color, &ctx->source_color);
    cairo_paint(ctx->cairo);
}

/*---------------------------------------------------------------------------*/

void draw_antialias(DCtx *ctx, const bool_t on)
{
    cairo_antialias_t anti;
    cassert_no_null(ctx);

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 12, 0)
    anti = on ? CAIRO_ANTIALIAS_GOOD : CAIRO_ANTIALIAS_NONE;
#else
    anti = on ? CAIRO_ANTIALIAS_SUBPIXEL : CAIRO_ANTIALIAS_NONE;
#endif

    cairo_set_antialias(ctx->cairo, anti);
}


