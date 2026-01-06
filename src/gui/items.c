/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: items.c
 *
 */

/* Shared implementation for PopUp and Combo lists */

#include "items.inl"
#include "cell.inl"
#include "gui.inl"
#include <draw2d/image.h>
#include <core/arrst.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

typedef struct _pelem_t PElem;

struct _pelem_t
{
    ResId resid;
    String *text;
    Image *image;
};

struct _items_t
{
    ArrSt(PElem) *elems;
    void *ositem;
    uint32_t max_visible_elems;
    FPtr_gctx_set_elem func_set_elem;
    FPtr_gctx_set_uint32 func_list_height;
    FPtr_gctx_set_uint32 func_set_selected;
    FPtr_gctx_get_uint32 func_get_selected;
    Listener *OnSelect;
};

DeclSt(PElem);

/*---------------------------------------------------------------------------*/

static void i_remove_elem(PElem *elem)
{
    cassert_no_null(elem);
    str_destroy(&elem->text);
    ptr_destopt(image_destroy, &elem->image, Image);
}

/*---------------------------------------------------------------------------*/

static Items *i_create(void *ositem)
{
    Items *items = heap_new0(Items);
    items->elems = arrst_create(PElem);
    items->ositem = ositem;
    items->max_visible_elems = 0;
    return items;
}

/*---------------------------------------------------------------------------*/

Items *_items_from_popup(const GuiCtx *context, void *ositem)
{
    Items *items = i_create(ositem);
    cassert_no_null(context);
    items->func_set_elem = context->func_popup_set_elem;
    items->func_list_height = context->func_popup_list_height;
    items->func_set_selected = context->func_popup_set_selected;
    items->func_get_selected = context->func_popup_get_selected;
    return items;
}

/*---------------------------------------------------------------------------*/

Items *_items_from_combo(const GuiCtx *context, void *ositem)
{
    Items *items = i_create(ositem);
    cassert_no_null(context);
    items->func_set_elem = context->func_combo_set_elem;
    items->func_list_height = context->func_combo_list_height;
    items->func_set_selected = context->func_combo_set_selected;
    items->func_get_selected = context->func_combo_get_selected;
    return items;
}

/*---------------------------------------------------------------------------*/

void _items_destroy(Items **items)
{
    cassert_no_null(items);
    cassert_no_null(*items);
    listener_destroy(&(*items)->OnSelect);
    arrst_destroy(&(*items)->elems, i_remove_elem, PElem);
    heap_delete(items, Items);
}

/*---------------------------------------------------------------------------*/

const char_t *_items_OnSelect(Items *items, Event *e, Cell *cell, void *sender, const char_t *sender_type)
{
    const EvButton *p = event_params(e, EvButton);
    const PElem *elem = NULL;
    cassert_no_null(items);
    cassert_no_null(p);

    if (cell != NULL)
        _cell_update_u32(cell, p->index);

    elem = arrst_get(items->elems, p->index, PElem);
    if (items->OnSelect != NULL)
    {
        cassert(p->text == NULL);
        cast(p, EvButton)->text = tc(elem->text);
        listener_pass_event_imp(items->OnSelect, e, sender, sender_type);
    }

    return tc(elem->text);
}

/*---------------------------------------------------------------------------*/

void _items_set_OnSelect(Items *items, Listener *listener)
{
    cassert_no_null(items);
    listener_update(&items->OnSelect, listener);
}

/*---------------------------------------------------------------------------*/

static Image *i_image(const Image *image)
{
    const Image *limage = _gui_respack_image((ResId)image, NULL);
    if (limage != NULL)
        return image_scale(limage, 16, 16);
    else
        return NULL;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_list_height(Items *items)
{
    uint32_t n = 0;
    cassert_no_null(items);
    n = arrst_size(items->elems, PElem);
    if (items->max_visible_elems > 0 && items->max_visible_elems < 1000 && items->max_visible_elems < n)
        return items->max_visible_elems;
    else if (n > 0)
        return n;
    else
        return 2;
}

/*---------------------------------------------------------------------------*/

void _items_add_elem(Items *items, const char_t *text, const Image *image)
{
    PElem *elem = NULL;
    const char_t *ltext = NULL;
    cassert_no_null(items);
    elem = arrst_new0(items->elems, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    elem->text = str_c(ltext);
    elem->image = i_image(image);
    items->func_set_elem(items->ositem, ekCTRL_OP_ADD, UINT32_MAX, tc(elem->text), elem->image);
    items->func_list_height(items->ositem, i_list_height(items));
}

/*---------------------------------------------------------------------------*/

void _items_set_elem(Items *items, const uint32_t index, const char_t *text, const Image *image)
{
    PElem *elem = NULL;
    const char_t *ltext = NULL;
    cassert_no_null(items);
    elem = arrst_get(items->elems, index, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    ptr_destopt(image_destroy, &elem->image, Image);
    elem->image = i_image(image);
    str_upd(&elem->text, ltext);
    items->func_set_elem(items->ositem, ekCTRL_OP_SET, index, tc(elem->text), elem->image);
}

/*---------------------------------------------------------------------------*/

void _items_ins_elem(Items *items, const uint32_t index, const char_t *text, const Image *image)
{
    PElem *elem = NULL;
    const char_t *ltext = NULL;
    cassert_no_null(items);
    elem = arrst_insert_n0(items->elems, index, 1, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    elem->text = str_c(ltext);
    elem->image = i_image(image);
    items->func_set_elem(items->ositem, ekCTRL_OP_INS, index, tc(elem->text), elem->image);
    items->func_list_height(items->ositem, i_list_height(items));
}

/*---------------------------------------------------------------------------*/

void _items_del_elem(Items *items, const uint32_t index)
{
    cassert_no_null(items);
    arrst_delete(items->elems, index, i_remove_elem, PElem);
    items->func_set_elem(items->ositem, ekCTRL_OP_DEL, index, NULL, NULL);
    items->func_list_height(items->ositem, i_list_height(items));
}

/*---------------------------------------------------------------------------*/

void _items_clear(Items *items)
{
    uint32_t i, n;
    cassert_no_null(items);
    n = arrst_size(items->elems, PElem);
    arrst_clear(items->elems, i_remove_elem, PElem);
    for (i = 0; i < n; ++i)
        items->func_set_elem(items->ositem, ekCTRL_OP_DEL, 0, NULL, NULL);
    items->func_list_height(items->ositem, i_list_height(items));
}

/*---------------------------------------------------------------------------*/

uint32_t _items_count(const Items *items)
{
    cassert_no_null(items);
    return arrst_size(items->elems, PElem);
}

/*---------------------------------------------------------------------------*/

void _items_list_height(Items *items, const uint32_t num_elems)
{
    cassert_no_null(items);
    items->max_visible_elems = num_elems;
    items->func_list_height(items->ositem, i_list_height(items));
}

/*---------------------------------------------------------------------------*/

void _items_selected(Items *items, const uint32_t index)
{
    cassert_no_null(items);
    cassert(index < arrst_size(items->elems, PElem));
    items->func_set_selected(items->ositem, index);
}

/*---------------------------------------------------------------------------*/

uint32_t _items_get_selected(const Items *items)
{
    cassert_no_null(items);
    return items->func_get_selected(items->ositem);
}

/*---------------------------------------------------------------------------*/

const char_t *_items_get_text(const Items *items, const uint32_t index)
{
    const PElem *elem = NULL;
    cassert_no_null(items);
    elem = arrst_get_const(items->elems, index, PElem);
    return tc(elem->text);
}

/*---------------------------------------------------------------------------*/

const Image *_items_get_image(const Items *items, const uint32_t index)
{
    const PElem *elem = NULL;
    cassert_no_null(items);
    elem = arrst_get_const(items->elems, index, PElem);
    return elem->image;
}

/*---------------------------------------------------------------------------*/

void _items_locale(Items *items)
{
    cassert_no_null(items);
    arrst_foreach(elem, items->elems, PElem)
        const char_t *text = _gui_respack_text(elem->resid, NULL);
        str_upd(&elem->text, text);
        items->func_set_elem(items->ositem, ekCTRL_OP_SET, elem_i, text, elem->image);
    arrst_end()
}

/*---------------------------------------------------------------------------*/

void _items_uint32(Items *items, const uint32_t value)
{
    cassert_no_null(items);
    if (value < arrst_size(items->elems, PElem))
        items->func_set_selected(items->ositem, value);
    else
        items->func_set_selected(items->ositem, UINT32_MAX);
}
