/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: btext.c
 *
 */

/* Text formatted blocks */

#include "btexth.inl"
#include "btext.inl"
#include "dctx.h"
#include "arrst.h"
#include "cassert.h"
#include "color.h"
#include "font.h"
#include "heap.h"
#include "strings.h"

typedef struct _attr_t Attr;
typedef struct _text_t Text;
typedef struct _line_t Line;

struct _attr_t
{
    String *font_family;
    real32_t font_relsize;
    real32_t font_size;
    uint32_t font_style;
    color_t line_color;
    color_t fill_color;
    align_t halign;
    uint32_t track_id;
    drawop_t op;
};

struct _text_t
{
    Attr attr;
    String *text;
};

struct _line_t
{
    real32_t width;
};

struct _btext_t
{
    bool_t metrics;
    Attr attr;
    uint32_t track_id;
    ArrSt(Attr) *stack;
    ArrSt(Text) *texts;
};

DeclSt(Attr);
DeclSt(Text);

/*---------------------------------------------------------------------------*/

BText *btext_create(void)
{
    BText *block = heap_new0(BText);
    block->attr.halign = ekLEFT;
    block->attr.op = ekFILL;
    block->track_id = UINT32_MAX;
    block->stack = arrst_create(Attr);
    block->texts = arrst_create(Text);
    return block;
}

/*---------------------------------------------------------------------------*/

static void i_remove_attr(Attr *attr)
{
    str_destopt(&attr->font_family);
}

/*---------------------------------------------------------------------------*/

static void i_remove_text(Text *text)
{
    i_remove_attr(&text->attr);
    str_destroy(&text->text);
}

/*---------------------------------------------------------------------------*/

void btext_destroy(BText **block)
{
    cassert_no_null(block);
    cassert_no_null(*block);
    arrst_destroy(&(*block)->stack, i_remove_attr, Attr);
    arrst_destroy(&(*block)->texts, i_remove_text, Text);
    heap_delete(block, BText);
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_push(
                    ArrSt(Attr) *stack,
                    const char_t *ffamily,
                    const real32_t frelsize,
                    const real32_t fsize,
                    const uint32_t fstyle,
                    const color_t line_color,
                    const color_t fill_color,
                    const align_t halign,
                    const drawop_t op, 
                    const uint32_t track_id)
{
    Attr *attr = arrst_new(stack, Attr);
    attr->font_family = ffamily ? str_c(ffamily) : NULL;
    attr->font_relsize = frelsize;
    attr->font_size = fsize;
    attr->font_style = fstyle;
    attr->line_color = line_color;
    attr->fill_color = fill_color;
    attr->halign = halign;
    attr->op = op;
    attr->track_id = track_id;
}

/*---------------------------------------------------------------------------*/

void btext_push_font_family(BText *block, const char_t *family)
{
    cassert_no_null(block);
    cassert_no_null(family);
    i_push(block->stack, family, 0, 0, 0, 0, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_font_relsize(BText *block, const real32_t rsize)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, rsize, 0, 0, 0, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_font_size(BText *block, const real32_t size)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, size, 0, 0, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_font_style(BText *block, const uint32_t style)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, 0, style, 0, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_line_color(BText *block, const color_t color)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, 0, 0, color, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_fill_color(BText *block, const color_t color)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, 0, 0, 0, color, ENUM_MAX(align_t), ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_halign(BText *block, const align_t align)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, 0, 0, 0, 0, align, ENUM_MAX(drawop_t), UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

void btext_push_drawop(BText *block, const drawop_t op)
{
    cassert_no_null(block);
    i_push(block->stack, NULL, 0, 0, 0, 0, 0, ENUM_MAX(align_t), op, UINT32_MAX);
}

/*---------------------------------------------------------------------------*/

uint32_t btext_push_track(BText *block)
{
    cassert_no_null(block);
    block->track_id += 1;
    i_push(block->stack, NULL, 0, 0, 0, 0, 0, ENUM_MAX(align_t), ENUM_MAX(drawop_t), block->track_id);
    return block->track_id;
}

/*---------------------------------------------------------------------------*/

void btext_pop(BText *block)
{
    cassert_no_null(block);
    arrst_pop(block->stack, i_remove_attr, Attr);
}

/*---------------------------------------------------------------------------*/

static void i_copy_attr(Attr *dest, const Attr *src)
{
    cassert_no_null(dest);
    cassert_no_null(src);
    *dest = *src;
    if (src->font_family != NULL)
        dest->font_family = str_copy(src->font_family);
}

/*---------------------------------------------------------------------------*/

void btext_text(BText *block, const char_t *text)
{
    Text *ttext;
    cassert_no_null(block);
    ttext = arrst_new(block->texts, Text);
    i_copy_attr(&ttext->attr, &block->attr);
    ttext->text = str_c(text);
}

/*---------------------------------------------------------------------------*/

void btext_update(BText *block, const real32_t max_width, const real32_t max_height, const Font *font)
{
    unref(block);
    unref(max_width);
    unref(max_height);
    unref(font);
}

/*---------------------------------------------------------------------------*/

void btext_bounds(const BText *block, real32_t *width, real32_t *height)
{
    unref(block);
    unref(width);
    unref(height);
}

/*---------------------------------------------------------------------------*/

void btext_draw(DCtx *ctx, const BText *block, const real32_t x, const real32_t y, const align_t halign, const align_t valign)
{
    unref(ctx);
    unref(block);
    unref(x);
    unref(y);
    unref(halign);
    unref(valign);
}

/*---------------------------------------------------------------------------*/

void btext_draw_raster(DCtx *ctx, const BText *block, const real32_t x, const real32_t y, const align_t halign, const align_t valign)
{
    unref(ctx);
    unref(block);
    unref(x);
    unref(y);
    unref(halign);
    unref(valign);
}

/*---------------------------------------------------------------------------*/

uint32_t btext_track(const BText *block, const real32_t x, const real32_t y)
{
    unref(block);
    unref(x);
    unref(y);
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

void btext_track_line_color(BText *block, const uint32_t track_id, const color_t color)
{
    unref(block);
    unref(track_id);
    unref(color);
}

/*---------------------------------------------------------------------------*/

void btext_track_fill_color(BText *block, const uint32_t track_id, const color_t color)
{
    unref(block);
    unref(track_id);
    unref(color);
}

/*---------------------------------------------------------------------------*/

void btext_track_underline(BText *block, const uint32_t track_id, bool_t underline)
{
    unref(block);
    unref(track_id);
    unref(underline);
}

