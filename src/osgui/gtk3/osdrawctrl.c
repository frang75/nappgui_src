/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl.c
 *
 */

/* Drawing custom GUI controls */

#include "draw.h"
#include "draw.inl"
#include "dctx.inl"
#include "osdrawctrl.h"
#include "osglobals.inl"
#include "cassert.h"
#include "font.h"
#include "image.inl"
#include "color.h"

#if !defined(__GTK3__)
#error This file is only for GTK Toolkit
#endif

#include "dctx_gtk.inl"
#include "draw_gtk.inl"
#include "draw2d_gtk.ixx"

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
    return 2;
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
    if (key == ekKEY_LCTRL || key == ekKEY_RCTRL)
        return ekMULTISEL_SINGLE;
    else if (key == ekKEY_LSHIFT || key == ekKEY_RSHIFT)
        return ekMULTISEL_BURST;
    return ekMULTISEL_NO;
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_clear(DCtx *ctx)
{
    GtkStyleContext *c = osglobals_entry_context();
    //uint32_t w, h;
    cassert_no_null(ctx);
    //dctx_size(ctx, &w, &h);
    gtk_style_context_save(c);
    gtk_style_context_set_state(c, GTK_STATE_FLAG_NORMAL);
    gtk_render_background(c, ctx->cairo, ctx->scroll_x, ctx->scroll_y, ctx->clip_width, ctx->clip_height);
    gtk_style_context_restore(c);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_fill(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    GtkStyleContext *c = osglobals_table_context();
    GtkStateFlags flags = 0;

    cassert_no_null(ctx);

    switch (state) {
    case ekCSTATE_NORMAL:
        flags = GTK_STATE_FLAG_NORMAL;
        break;

    case ekCSTATE_HOT:
        flags = GTK_STATE_FLAG_PRELIGHT;
        break;

    case ekCSTATE_PRESSED:
        flags = GTK_STATE_FLAG_SELECTED;
        break;

    case ekCSTATE_BKNORMAL:
        flags = GTK_STATE_FLAG_NORMAL | GTK_STATE_FLAG_BACKDROP;
        break;

    case ekCSTATE_BKHOT:
        flags = GTK_STATE_FLAG_PRELIGHT | GTK_STATE_FLAG_BACKDROP;
        break;

    case ekCSTATE_BKPRESSED:
        flags = GTK_STATE_FLAG_SELECTED | GTK_STATE_FLAG_BACKDROP;
        break;

    case ekCSTATE_DISABLED:
        flags = GTK_STATE_FLAG_INSENSITIVE;
        break;

    cassert_default();
    }

    gtk_style_context_save(c);
    gtk_style_context_set_state(c, flags);
    gtk_render_background(c, ctx->cairo, (double)x, (double)y, (double)width, (double)height);
    gtk_style_context_restore(c);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_focus(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    GtkStyleContext *c = osglobals_table_context();
    cassert_no_null(ctx);
    unref(state);
    gtk_style_context_save(c);
    gtk_style_context_set_state(c, GTK_STATE_FLAG_SELECTED);
    gtk_render_focus(c, ctx->cairo, (double)x, (double)y, (double)width - 2, (double)height);
    gtk_style_context_restore(c);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_text(DCtx *ctx, const char_t *text, const uint32_t x, const uint32_t y, const cstate_t state)
{
    color_t color = 0;

    cassert_no_null(ctx);
    if (ctx->raster_mode == FALSE)
        drawimp_raster_mode(ctx);

    switch (state) {
    case ekCSTATE_NORMAL:
        color = osglobals_text_color();
        break;

    case ekCSTATE_BKNORMAL:
        color = osglobals_textbackdrop_color();
        break;

    case ekCSTATE_HOT:
        color = osglobals_hottext_color();
        break;

    case ekCSTATE_BKHOT:
        color = osglobals_hottextbackdrop_color();
        break;

    case ekCSTATE_PRESSED:
        color = osglobals_seltext_color();
        break;

    case ekCSTATE_BKPRESSED:
        color = osglobals_seltextbackdrop_color();
        break;

    case ekCSTATE_DISABLED:
        color = osglobals_text_color();
        break;

    cassert_default();
    }

    drawimp_begin_text(ctx, text, x, y);
    drawimp_color(ctx->cairo, color, &ctx->source_color);
    pango_cairo_show_layout(ctx->cairo, ctx->layout);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_image(DCtx *ctx, const Image *image, const uint32_t x, const uint32_t y, const cstate_t state)
{
    const OSImage *osimage = osimage_from_image(image);
    unref(state);
    draw_imgimp(ctx, osimage, UINT32_MAX, x, y, TRUE);
}

/*---------------------------------------------------------------------------*/

static void i_draw_check(cairo_t *cairo, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state, const uint32_t start, const bool_t check)
{
    GdkPixbuf *bitmap = osglobals_checks_bitmap();
    uint32_t offset = start;

    switch(state) {
    case ekCSTATE_NORMAL:
    case ekCSTATE_BKNORMAL:
        offset += 0;
        break;

    case ekCSTATE_HOT:
    case ekCSTATE_BKHOT:
        offset += 1;
        break;

    case ekCSTATE_PRESSED:
        offset += 2;
        break;

    case ekCSTATE_BKPRESSED:
        offset += 3;
        break;

    case ekCSTATE_DISABLED:
        offset += 4;
        break;

    cassert_default();
    }

    cassert(width == osglobals_check_width());
    cassert(height == osglobals_check_height());
    gdk_cairo_set_source_pixbuf(cairo, bitmap, (gdouble)x - (gdouble)(offset * width), (gdouble)y);
    cairo_rectangle(cairo, (double)x, (double)y, (double)width, (double)height);
    cairo_fill(cairo);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_checkbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    cassert_no_null(ctx);
    i_draw_check(ctx->cairo, x, y, width, height, state, 5, TRUE);
}

/*---------------------------------------------------------------------------*/

void osdrawctrl_uncheckbox(DCtx *ctx, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const cstate_t state)
{
    cassert_no_null(ctx);
    i_draw_check(ctx->cairo, x, y, width, height, state, 0, FALSE);
}
