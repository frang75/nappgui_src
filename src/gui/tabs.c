/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: tabs.c
 *
 */

/* Tab control */

#include "tabs.h"
#include "tabs.inl"
#include "items.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/font.h>
#include <draw2d/guictx.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _tabs_t
{
    GuiComponent component;
    uint32_t flags;
    real32_t length;
    S2Df size;
    ResId ttipid;
    Items *items;
};

/*---------------------------------------------------------------------------*/

void _tabs_destroy(Tabs **tabs)
{
    cassert_no_null(tabs);
    cassert_no_null(*tabs);
    _component_destroy_imp(&(*tabs)->component);
    _items_destroy(&(*tabs)->items);
    obj_delete(tabs, Tabs);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(Tabs *tabs, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(tabs);
    cell = _component_cell(&tabs->component);
    _items_OnSelect(tabs->items, e, cell, cast(tabs, void), "Tabs");
}

/*---------------------------------------------------------------------------*/

Tabs *tabs_create(const gui_pos_t pos)
{
    const GuiCtx *context = guictx_get_current();
    Tabs *tabs = obj_new0(Tabs);
    void *ositem = context->func_create[ekGUI_TYPE_TABLIST]((uint32_t)pos);
    Font *font = _gui_create_default_font();
    tabs->flags = (uint32_t)pos;
    tabs->length = 100;
    tabs->items = _items_from_tabs(context, ositem);
    context->func_tabs_set_font(ositem, font);
    _component_init(&tabs->component, context, PARAM(type, ekGUI_TYPE_TABLIST), &ositem);
    context->func_tabs_OnSelect(tabs->component.ositem, obj_listener(tabs, i_OnSelect, Tabs));
    font_destroy(&font);
    return tabs;
}

/*---------------------------------------------------------------------------*/

void tabs_OnSelect(Tabs *tabs, Listener *listener)
{
    cassert_no_null(tabs);
    _items_set_OnSelect(tabs->items, listener);
}

/*---------------------------------------------------------------------------*/

void tabs_length(Tabs *tabs, const real32_t length)
{
    cassert_no_null(tabs);
    cassert(length > 0);
    tabs->length = length;
}

/*---------------------------------------------------------------------------*/

void tabs_tooltip(Tabs *tabs, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(tabs);
    if (text != NULL)
        ltext = _gui_respack_text(text, &tabs->ttipid);
    tabs->component.context->func_set_tooltip[ekGUI_TYPE_TABLIST](tabs->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void tabs_add_elem(Tabs *tabs, const char_t *text, const Image *image)
{
    cassert_no_null(tabs);
    _items_add_elem(tabs->items, text, image);
}

/*---------------------------------------------------------------------------*/

void tabs_set_elem(Tabs *tabs, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(tabs);
    _items_set_elem(tabs->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void tabs_ins_elem(Tabs *tabs, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(tabs);
    _items_ins_elem(tabs->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void tabs_del_elem(Tabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    _items_del_elem(tabs->items, index);
}

/*---------------------------------------------------------------------------*/

void tabs_clear(Tabs *tabs)
{
    cassert_no_null(tabs);
    _items_clear(tabs->items);
}

/*---------------------------------------------------------------------------*/

uint32_t tabs_count(const Tabs *tabs)
{
    cassert_no_null(tabs);
    return _items_count(tabs->items);
}

/*---------------------------------------------------------------------------*/

void tabs_selected(Tabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    _items_selected(tabs->items, index);
}

/*---------------------------------------------------------------------------*/

uint32_t tabs_get_selected(const Tabs *tabs)
{
    cassert_no_null(tabs);
    return _items_get_selected(tabs->items);
}

/*---------------------------------------------------------------------------*/

const char_t *tabs_get_text(const Tabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    return _items_get_text(tabs->items, index);
}

/*---------------------------------------------------------------------------*/

const Image *tabs_get_image(const Tabs *tabs, const uint32_t index)
{
    cassert_no_null(tabs);
    return _items_get_image(tabs->items, index);
}

/*---------------------------------------------------------------------------*/

void _tabs_natural(Tabs *tabs, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(tabs);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        cassert_no_nullf(tabs->component.context->func_tabs_bounds);
        tabs->component.context->func_tabs_bounds(tabs->component.ositem, tabs->length, &tabs->size.width, &tabs->size.height);
        *dim0 = tabs->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = tabs->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _tabs_expand(Tabs *tabs, const uint32_t i, const real32_t current_size, const real32_t required_size, real32_t *final_size)
{
    unref(tabs);
    unref(i);
    unref(current_size);
    cassert_no_null(final_size);
    *final_size = required_size;
}

/*---------------------------------------------------------------------------*/

void _tabs_locale(Tabs *tabs)
{
    cassert_no_null(tabs);
    _items_locale(tabs->items);

    if (tabs->ttipid != NULL)
    {
        const char_t *text = _gui_respack_text(tabs->ttipid, NULL);
        tabs->component.context->func_set_tooltip[ekGUI_TYPE_TABLIST](tabs->component.ositem, text);
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _tabs_flags(const Tabs *tabs)
{
    cassert_no_null(tabs);
    return tabs->flags;
}
