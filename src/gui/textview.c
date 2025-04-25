/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: textview.c
 *
 */

/* Text view */

#include "textview.h"
#include "textview.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/font.h>
#include <draw2d/guictx.h>
#include <geom2d/s2d.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/objh.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>

struct _textview_t
{
    GuiComponent component;
    S2Df size;
    Listener *OnFilter;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

static void i_OnFilter(TextView *view, Event *e)
{
    cassert_no_null(view);
    if (view->OnFilter != NULL)
        listener_pass_event(view->OnFilter, e, view, TextView);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(TextView *view, Event *e)
{
    cassert_no_null(view);
    if (view->OnFocus != NULL)
        listener_pass_event(view->OnFocus, e, view, TextView);
}

/*---------------------------------------------------------------------------*/

TextView *textview_create(void)
{
    const GuiCtx *context = guictx_get_current();
    TextView *view = obj_new0(TextView);
    void *ositem = NULL;
    Font *font = font_monospace(font_regular_size(), 0);
    const char_t *family = font_family(font);
    uint32_t units = ekFPIXELS;
    real32_t size = font_size(font);
    uint32_t fstyle = ekFNORMAL;
    align_t palign = ekLEFT;
    real32_t lspacing = 1;
    real32_t bfpspace = 0;
    real32_t afpspace = 0;
    uint32_t non_used = 0;
    cassert_no_null(context);
    ositem = context->func_create[ekGUI_TYPE_TEXTVIEW](ekTEXT_FLAG);
    view->size = s2df(256, 144);
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_FAMILY, cast_const(family, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_UNITS, cast_const(&units, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_SIZE, cast_const(&size, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_STYLE, cast_const(&fstyle, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_PARALIGN, cast(&palign, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_LSPACING, cast(&lspacing, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_BFPARSPACE, cast(&bfpspace, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_AFPARSPACE, cast(&afpspace, void));
    context->func_text_set_prop(ositem, (enum_t)ekGUI_TEXT_APPLY_ALL, cast(&non_used, void));
    _component_init(&view->component, context, PARAM(type, ekGUI_TYPE_TEXTVIEW), &ositem);
    context->func_text_OnFocus(view->component.ositem, obj_listener(view, i_OnFocus, TextView));
    font_destroy(&font);
    return view;
}

/*---------------------------------------------------------------------------*/

void _textview_destroy(TextView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    _component_destroy_imp(&(*view)->component);
    listener_destroy(&(*view)->OnFilter);
    listener_destroy(&(*view)->OnFocus);
    obj_delete(view, TextView);
}

/*---------------------------------------------------------------------------*/

void textview_OnFilter(TextView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFilter, listener);
    if (listener != NULL)
        view->component.context->func_text_OnFilter(view->component.ositem, obj_listener(view, i_OnFilter, TextView));
    else
        view->component.context->func_text_OnFilter(view->component.ositem, NULL);
}

/*---------------------------------------------------------------------------*/

void textview_OnFocus(TextView *view, Listener *listener)
{
    cassert_no_null(view);
    listener_update(&view->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void textview_size(TextView *view, const S2Df size)
{
    cassert_no_null(view);
    view->size = size;
}

/*---------------------------------------------------------------------------*/

void textview_clear(TextView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_text);
    view->component.context->func_text_set_text(view->component.ositem, "");
}

/*---------------------------------------------------------------------------*/

static uint32_t i_printf(FPtr_gctx_set_text func_text, void *ositem, const char_t *format, va_list args, va_list args_copy)
{
    char_t ctext[1024];
    char_t *text_alloc = NULL;
    char_t *text = NULL;
    uint32_t length = 0;

    cassert_no_nullf(func_text);
    cassert_no_null(ositem);
    cassert_no_null(format);

    /* Memory requeriments */
    length = 1 + bstd_vsprintf(NULL, 0, format, args);

    /* Nothing to write */
    if (length == 1)
        return 0;

    /* Temporal buffer */
    if (length < sizeof(ctext))
    {
        text = ctext;
    }
    else
    {
        text_alloc = cast(heap_malloc(length, "TextViewPrintf"), char_t);
        text = text_alloc;
    }

    /* Printf */
    {
        uint32_t clength = bstd_vsprintf(text, length, format, args_copy);
        cassert_unref(clength + 1 == length, clength);
    }

    cassert(text[length - 1] == '\0');

    func_text(ositem, text);

    if (text_alloc != NULL)
        heap_free(dcast(&text_alloc, byte_t), length, "TextViewPrintf");

    return length - 1;
}

/*---------------------------------------------------------------------------*/

uint32_t textview_printf(TextView *view, const char_t *format, ...)
{
    uint32_t length = 0;
    va_list args, args_copy;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    va_start(args, format);
    va_copy(args_copy, args);
    length = i_printf(view->component.context->func_text_add_text, view->component.ositem, format, args, args_copy);
    va_end(args);
    va_end(args_copy);
    return length;
}

/*---------------------------------------------------------------------------*/

void textview_writef(TextView *view, const char_t *text)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_add_text);
    view->component.context->func_text_add_text(view->component.ositem, text);
}

/*---------------------------------------------------------------------------*/

uint32_t textview_cpos_printf(TextView *view, const char_t *format, ...)
{
    uint32_t length = 0;
    va_list args, args_copy;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    va_start(args, format);
    va_copy(args_copy, args);
    length = i_printf(view->component.context->func_text_ins_text, view->component.ositem, format, args, args_copy);
    va_end(args);
    va_end(args_copy);
    return length;
}

/*---------------------------------------------------------------------------*/

void textview_cpos_writef(TextView *view, const char_t *text)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_ins_text);
    view->component.context->func_text_ins_text(view->component.ositem, text);
}

/*---------------------------------------------------------------------------*/

void textview_rtf(TextView *view, Stream *rtf_in)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_rtf);
    view->component.context->func_text_set_rtf(view->component.ositem, rtf_in);
}

/*---------------------------------------------------------------------------*/

void textview_units(TextView *view, const uint32_t units)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_UNITS, cast_const(&units, void));
}

/*---------------------------------------------------------------------------*/

void textview_family(TextView *view, const char_t *family)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_FAMILY, cast_const(family, void));
}

/*---------------------------------------------------------------------------*/

void textview_fsize(TextView *view, const real32_t size)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_SIZE, cast_const(&size, void));
}

/*---------------------------------------------------------------------------*/

void textview_fstyle(TextView *view, const uint32_t fstyle)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_STYLE, cast_const(&fstyle, void));
}

/*---------------------------------------------------------------------------*/

void textview_color(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_COLOR, cast(&color, void));
}

/*---------------------------------------------------------------------------*/

void textview_bgcolor(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_BGCOLOR, cast(&color, void));
}

/*---------------------------------------------------------------------------*/

void textview_pgcolor(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_PGCOLOR, cast(&color, void));
}

/*---------------------------------------------------------------------------*/

void textview_halign(TextView *view, const align_t align)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_PARALIGN, cast(&align, void));
}

/*---------------------------------------------------------------------------*/

void textview_lspacing(TextView *view, const real32_t scale)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_LSPACING, cast(&scale, void));
}

/*---------------------------------------------------------------------------*/

void textview_bfspace(TextView *view, const real32_t space)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_BFPARSPACE, cast(&space, void));
}

/*---------------------------------------------------------------------------*/

void textview_afspace(TextView *view, const real32_t space)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_AFPARSPACE, cast(&space, void));
}

/*---------------------------------------------------------------------------*/

void textview_apply_all(TextView *view)
{
    uint32_t nonused = 0;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_APPLY_ALL, cast(&nonused, void));
}

/*---------------------------------------------------------------------------*/

void textview_apply_select(TextView *view)
{
    uint32_t nonused = 0;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_APPLY_SEL, cast(&nonused, void));
}

/*---------------------------------------------------------------------------*/

void textview_scroll_visible(TextView *view, const bool_t horizontal, const bool_t vertical)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_scroller_visible);
    view->component.context->func_text_scroller_visible(view->component.ositem, horizontal, vertical);
}

/*---------------------------------------------------------------------------*/

void textview_editable(TextView *view, const bool_t is_editable)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_editable);
    view->component.context->func_text_set_editable(view->component.ositem, is_editable);
}

/*---------------------------------------------------------------------------*/

void textview_select(TextView *view, const int32_t start, const int32_t end)
{
    int32_t range[2];
    range[0] = start;
    range[1] = end;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_SELECT, cast(range, void));
}

/*---------------------------------------------------------------------------*/

void textview_show_select(TextView *view, const bool_t show)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_SHOW_SELECT, cast(&show, void));
}

/*---------------------------------------------------------------------------*/

void textview_del_select(TextView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_ins_text);
    view->component.context->func_text_ins_text(view->component.ositem, "");
}

/*---------------------------------------------------------------------------*/

void textview_scroll_caret(TextView *view)
{
    uint32_t nonused = 0;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_SCROLL, &nonused);
}

/*---------------------------------------------------------------------------*/

const char_t *textview_get_text(const TextView *view)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_get_text);
    return view->component.context->func_text_get_text(view->component.ositem);
}

/*---------------------------------------------------------------------------*/

void textview_copy(const TextView *view)
{
    cassert_no_null(view);
    cassert_no_nullf(view->component.context->func_text_clipboard);
    view->component.context->func_text_clipboard(view->component.ositem, ekCLIPBOARD_COPY);
}

/*---------------------------------------------------------------------------*/

void textview_cut(TextView *view)
{
    cassert_no_null(view);
    cassert_no_nullf(view->component.context->func_text_clipboard);
    view->component.context->func_text_clipboard(view->component.ositem, ekCLIPBOARD_CUT);
}

/*---------------------------------------------------------------------------*/

void textview_paste(TextView *view)
{
    cassert_no_null(view);
    cassert_no_nullf(view->component.context->func_text_clipboard);
    view->component.context->func_text_clipboard(view->component.ositem, ekCLIPBOARD_PASTE);
}

/*---------------------------------------------------------------------------*/

void textview_wrap(TextView *view, const bool_t wrap)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, (enum_t)ekGUI_TEXT_WRAP_MODE, cast_const(&wrap, void));
}

/*---------------------------------------------------------------------------*/

void _textview_natural(TextView *view, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(view);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        *dim0 = view->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = view->size.height;
    }
}
