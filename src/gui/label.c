/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: label.c
 *
 */

/* Label control */

#include "label.h"
#include "label.inl"
#include "gui.h"
#include "gui.inl"
#include "drawctrl.inl"
#include "view.h"
#include "view.inl"
#include "cell.inl"
#include "layout.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <draw2d/dctx.h>
#include <draw2d/draw.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

typedef struct _lbdata_t LbData;

struct _lbdata_t
{
    real32_t fwidth;
    real32_t height;
    ResId textid;
    String *text;
    uint32_t flags;
    align_t halign;
    ellipsis_t ellipsis;
    Font *font;
    Font *over_font;
    color_t color;
    color_t over_color;
    color_t bg_color;
    color_t bg_over_color;
    bool_t mouse_sensitive;
    bool_t hover;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

static void i_destroy_data(LbData **data)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    listener_destroy(&(*data)->OnClick);
    str_destroy(&(*data)->text);
    font_destroy(&(*data)->font);
    ptr_destopt(font_destroy, &(*data)->over_font, Font);
    heap_delete(data, LbData);
}

/*---------------------------------------------------------------------------*/

static LbData *i_create_data(const uint32_t flags, const align_t halign, const ellipsis_t ellipsis)
{
    LbData *data = heap_new0(LbData);
    data->font = _gui_create_default_font();
    data->text = str_c("");
    data->flags = flags;
    data->halign = halign;
    data->ellipsis = ellipsis;
    data->color = gui_label_color();
    data->over_color = gui_label_color();
    data->bg_color = kCOLOR_TRANSPARENT;
    data->bg_over_color = kCOLOR_TRANSPARENT;
    return data;
}

/*---------------------------------------------------------------------------*/

static void i_OnAcceptFocus(Label *label, Event *e)
{
    bool_t *r = event_result(e, bool_t);
    unref(label);
    *r = FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_OnDraw(Label *label, Event *e)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    const EvDraw *p = event_params(e, EvDraw);
    color_t color = kCOLOR_TRANSPARENT;
    color_t bgcolor = kCOLOR_TRANSPARENT;
    const Font *font = NULL;
    cassert_no_null(data);
    if (data->hover == TRUE)
    {
        color = data->over_color;
        bgcolor = data->bg_over_color;
        if (data->over_font != NULL)
            font = data->over_font;
        else
            font = data->font;
    }
    else
    {
        color = data->color;
        bgcolor = data->bg_color;
        font = data->font;
    }

    if (bgcolor != kCOLOR_TRANSPARENT)
        draw_clear(p->ctx, bgcolor);

    draw_text_color(p->ctx, color);
    draw_font(p->ctx, font);
    draw_text_halign(p->ctx, data->halign);
    draw_text_width(p->ctx, p->width);

    switch (label_get_type(data->flags))
    {
    case ekLABEL_SINGLE:
        draw_text_trim(p->ctx, data->ellipsis);
        break;

    case ekLABEL_MULTI:
        draw_text_trim(p->ctx, ekELLIPMLINE);
        break;

    default:
        cassert_default(label_get_type(data->flags));
    }

    drawctrl_text(p->ctx, tc(data->text), 0, 0, ekCTRL_STATE_NORMAL);
}

/*---------------------------------------------------------------------------*/

static void i_OnEnter(Label *label, Event *e)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    unref(e);
    data->hover = data->mouse_sensitive;
    if (data->mouse_sensitive == TRUE)
        view_update(cast(label, View));
}

/*---------------------------------------------------------------------------*/

static void i_OnExit(Label *label, Event *e)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    unref(e);
    data->hover = FALSE;
    if (data->mouse_sensitive == TRUE)
        view_update(cast(label, View));
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(Label *label, Event *e)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (data->OnClick != NULL)
        listener_pass_event(data->OnClick, e, label, Label);
}

/*---------------------------------------------------------------------------*/

static void i_locale(Label *label)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (data->textid != NULL)
    {
        const char_t *text = _gui_respack_text(data->textid, NULL);
        str_upd(&data->text, text);
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

static void i_natural(Label *label, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (di == 0)
    {
        real32_t width;
        font_extents(data->font, tc(data->text), -1, &width, &data->height);
        if (data->fwidth > 0)
            *dim0 = data->fwidth;
        else
            *dim0 = width;
    }
    else
    {
        cassert(di == 1);
        switch (label_get_type(data->flags))
        {
        case ekLABEL_SINGLE:
            break;

        case ekLABEL_MULTI:
        {
            real32_t width;
            font_extents(data->font, tc(data->text), *dim0, &width, &data->height);
            break;
        }

        default:
            cassert_default(label_get_type(data->flags));
        }

        *dim1 = data->height;
    }
}

/*---------------------------------------------------------------------------*/

Label *label_create(void)
{
    View *view = _view_create(ekVIEW_CONTROL);
    LbData *data = i_create_data(ekLABEL_SINGLE, ekLEFT, ekELLIPEND);
    view_data(view, &data, i_destroy_data, LbData);
    view_OnAcceptFocus(view, listener(cast(view, Label), i_OnAcceptFocus, Label));
    view_OnDraw(view, listener(cast(view, Label), i_OnDraw, Label));
    view_OnEnter(view, listener(cast(view, Label), i_OnEnter, Label));
    view_OnExit(view, listener(cast(view, Label), i_OnExit, Label));
    view_OnClick(view, listener(cast(view, Label), i_OnClick, Label));
    _view_OnLocale(view, (FPtr_gctx_call)i_locale);
    _view_OnNatural(view, (FPtr_natural)i_natural);
    _view_set_subtype(view, "Label");
    return cast(view, Label);
}

/*---------------------------------------------------------------------------*/

void label_OnClick(Label *label, Listener *listener)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    listener_update(&data->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

void label_min_width(Label *label, const real32_t width)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    data->fwidth = width;
}

/*---------------------------------------------------------------------------*/

static bool_t i_is_mouse_sensible(LbData *data)
{
    cassert_no_null(data);
    if (data->over_color != kCOLOR_DEFAULT)
        return TRUE;
    if (data->bg_over_color != kCOLOR_TRANSPARENT)
        return TRUE;
    return _gui_effective_alt_font(data->font, data->over_font);
}

/*---------------------------------------------------------------------------*/

void label_text(Label *label, const char_t *text)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    const char_t *ltext;
    cassert_no_null(data);
    ltext = _gui_respack_text(text, &data->textid);
    str_upd(&data->text, ltext);
    view_update(cast(label, View));
}

/*---------------------------------------------------------------------------*/

void label_size_text(Label *label, const char_t *text)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    real32_t height = 0;
    cassert_no_null(data);
    font_extents(data->font, text, -1, &data->fwidth, &height);
}

/*---------------------------------------------------------------------------*/

void label_font(Label *label, const Font *font)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (_gui_update_font(&data->font, &data->over_font, font) == TRUE)
    {
        data->mouse_sensitive = i_is_mouse_sensible(data);
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

void label_style_over(Label *label, const uint32_t fstyle)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    ptr_destopt(font_destroy, &data->over_font, Font);
    if (fstyle != font_style(data->font))
        data->over_font = font_with_style(data->font, fstyle);
    data->mouse_sensitive = i_is_mouse_sensible(data);
}

/*---------------------------------------------------------------------------*/

void label_multiline(Label *label, const bool_t multiline)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if ((label_get_type(data->flags) == ekLABEL_MULTI) != multiline)
    {
        data->flags = multiline ? ekLABEL_MULTI : ekLABEL_SINGLE;
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

void label_align(Label *label, const align_t align)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (data->halign != align)
    {
        data->halign = align;
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

void label_color(Label *label, const color_t color)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (data->color != color)
    {
        data->color = color;
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

void label_color_over(Label *label, const color_t color)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    data->over_color = color;
    data->mouse_sensitive = i_is_mouse_sensible(data);
}

/*---------------------------------------------------------------------------*/

void label_bgcolor(Label *label, const color_t color)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    if (data->bg_color != color)
    {
        data->bg_color = color;
        view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

void label_bgcolor_over(Label *label, const color_t color)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    data->bg_over_color = color;
    data->mouse_sensitive = i_is_mouse_sensible(data);
}

/*---------------------------------------------------------------------------*/

void label_trim(Label *label, const ellipsis_t ellipsis)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    ellipsis_t sellip = ellipsis;
    cassert_no_null(data);
    switch (ellipsis)
    {
    case ekELLIPBEGIN:
    case ekELLIPMIDDLE:
    case ekELLIPEND:
        sellip = ellipsis;
        break;
    case ekELLIPNONE:
    case ekELLIPMLINE:
        sellip = ekELLIPEND;
        break;
    default:
        cassert_default(ellipsis);
    }

    if (sellip != data->ellipsis)
    {
        data->ellipsis = sellip;
        if (label_get_type(data->flags) == ekLABEL_SINGLE)
            view_update(cast(label, View));
    }
}

/*---------------------------------------------------------------------------*/

const char_t *label_get_text(const Label *label)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    return tc(data->text);
}

/*---------------------------------------------------------------------------*/

const Font *label_get_font(const Label *label)
{
    LbData *data = view_get_data(cast(label, View), LbData);
    cassert_no_null(data);
    return data->font;
}

/*---------------------------------------------------------------------------*/

void _label_text(Label *label, const char_t *text)
{
    label_text(label, text);
}
