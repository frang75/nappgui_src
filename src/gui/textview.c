/*
 * NAppGUI Cross-platform C SDK
 * 2015-2023 Francisco Garcia Collado
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
#include "guictx.h"

#include "bstd.h"
#include "cassert.h"
#include "event.h"
#include "font.h"
#include "heap.h"
#include "objh.h"
#include "s2d.h"

struct _textview_t
{
    GuiComponent component;
    S2Df size;
    Listener *OnChange;
};

/*---------------------------------------------------------------------------*/

TextView *textview_create(void)
{
    const GuiCtx *context = guictx_get_current();
    TextView *view = obj_new0(TextView);
    void *ositem = NULL;
    Font *font = font_monospace(font_regular_size(), 0);
    const char_t *family = font_family(font);
    uint32_t units = ekFPIXELS;
    real32_t size = 14;
    uint32_t fstyle = ekFNORMAL;
    align_t palign = ekLEFT;
    real32_t lspacing = 1;
    real32_t bfpspace = 0;
    real32_t afpspace = 0;
    cassert_no_null(context);
    ositem = context->func_create[ekGUI_TYPE_TEXTVIEW](ekTEXT_FLAG);
    view->size = s2df(256, 144);
    context->func_text_set_prop(ositem, ekGUI_PROP_FAMILY, (const void*)family);
    context->func_text_set_prop(ositem, ekGUI_PROP_UNITS, (const void*)&units);
    context->func_text_set_prop(ositem, ekGUI_PROP_SIZE, (const void*)&size);
    context->func_text_set_prop(ositem, ekGUI_PROP_STYLE, (const void*)&fstyle);
    context->func_text_set_prop(ositem, ekGUI_PROP_PARALIGN, (void*)&palign);
    context->func_text_set_prop(ositem, ekGUI_PROP_LSPACING, (void*)&lspacing);
    context->func_text_set_prop(ositem, ekGUI_PROP_BFPARSPACE, (void*)&bfpspace);
    context->func_text_set_prop(ositem, ekGUI_PROP_AFPARSPACE, (void*)&afpspace);
    _component_init(&view->component, context, PARAM(type, ekGUI_TYPE_TEXTVIEW), &ositem);
    font_destroy(&font);
    return view;
}

/*---------------------------------------------------------------------------*/

void _textview_destroy(TextView **view)
{
    cassert_no_null(view);
    cassert_no_null(*view);
    _component_destroy_imp(&(*view)->component);
    listener_destroy(&(*view)->OnChange);
    obj_delete(view, TextView);
}

/*---------------------------------------------------------------------------*/

/*static void i_OnTextChange(Text *view, Event *event)
{
    cassert_no_null(view);
    cassert_no_null(view->OnTextChange.object);
    listener_pass_event(&view->OnTextChange, view, event, Text);
}*/

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

uint32_t textview_printf(TextView *view, const char_t *format, ...)
{
    char_t ctext[1024];
    char_t *text_alloc = NULL;
    char_t *text = NULL;
    uint32_t length = 0;

    cassert_no_null(view);
    cassert_no_null(format);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_insert_text);

    /* Memory requeriments */
    {
        va_list args;
        va_start(args, format);
        length = 1 + bstd_vsprintf(NULL, 0, format, args);
        va_end(args);
    }

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
        text_alloc = (char_t*)heap_malloc(length, "TextViewPrintf");
        text = text_alloc;
    }

    /* Printf */
    {
        register uint32_t clength;
        va_list args;
        va_start(args, format);
        clength = bstd_vsprintf(text, length, format, args);
        va_end(args);
        cassert_unref(clength + 1 == length, clength);
    }

    cassert(text[length - 1] == '\0');

    view->component.context->func_text_insert_text(view->component.ositem, text);

    if (text_alloc != NULL)
        heap_free((byte_t**)&text_alloc, length, "TextViewPrintf");

    return length - 1;
}

/*---------------------------------------------------------------------------*/

void textview_writef(TextView *view, const char_t *text)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_insert_text);
    view->component.context->func_text_insert_text(view->component.ositem, text);
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
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_UNITS, (const void*)&units);
}

/*---------------------------------------------------------------------------*/

void textview_family(TextView *view, const char_t *family)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_FAMILY, (const void*)family);
}

/*---------------------------------------------------------------------------*/

void textview_fsize(TextView *view, const real32_t size)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_SIZE, (const void*)&size);
}

/*---------------------------------------------------------------------------*/

void textview_fstyle(TextView *view, const uint32_t fstyle)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_STYLE, (const void*)&fstyle);
}

/*---------------------------------------------------------------------------*/

void textview_color(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_COLOR, (void*)&color);
}

/*---------------------------------------------------------------------------*/

void textview_bgcolor(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_BGCOLOR, (void*)&color);
}

/*---------------------------------------------------------------------------*/

void textview_pgcolor(TextView *view, const color_t color)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_PGCOLOR, (void*)&color);
}

/*---------------------------------------------------------------------------*/

void textview_halign(TextView *view, const align_t align)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_PARALIGN, (void*)&align);
}

/*---------------------------------------------------------------------------*/

void textview_lspacing(TextView *view, const real32_t scale)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_LSPACING, (void*)&scale);
}

/*---------------------------------------------------------------------------*/

void textview_bfspace(TextView *view, const real32_t space)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_BFPARSPACE, (void*)&space);
}

/*---------------------------------------------------------------------------*/

void textview_afspace(TextView *view, const real32_t space)
{
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_AFPARSPACE, (void*)&space);
}

/*---------------------------------------------------------------------------*/

void textview_scroll_down(TextView *view)
{
    real32_t scroll = 1e10f;
    cassert_no_null(view);
    cassert_no_null(view->component.context);
    cassert_no_nullf(view->component.context->func_text_set_prop);
    view->component.context->func_text_set_prop(view->component.ositem, ekGUI_PROP_VSCROLL, (void*)&scroll);
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

void _textview_dimension(TextView *view, const uint32_t i, real32_t *dim0, real32_t *dim1)
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

