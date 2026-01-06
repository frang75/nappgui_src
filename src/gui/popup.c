/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: popup.c
 *
 */

/* Pop-up button */

#include "popup.h"
#include "popup.inl"
#include "items.inl"
#include "cell.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/font.h>
#include <draw2d/guictx.h>
#include <geom2d/s2d.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _popup_t
{
    GuiComponent component;
    S2Df size;
    ResId ttipid;
    Items *items;
};

/*---------------------------------------------------------------------------*/

void _popup_destroy(PopUp **popup)
{
    cassert_no_null(popup);
    cassert_no_null(*popup);
    _component_destroy_imp(&(*popup)->component);
    _items_destroy(&(*popup)->items);
    obj_delete(popup, PopUp);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(PopUp *popup, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(popup);
    cell = _component_cell(&popup->component);
    _items_OnSelect(popup->items, e, cell, cast(popup, void), "PopUp");
}

/*---------------------------------------------------------------------------*/

PopUp *popup_create(void)
{
    const GuiCtx *context = guictx_get_current();
    PopUp *popup = obj_new0(PopUp);
    void *ositem = context->func_create[ekGUI_TYPE_POPUP](ekPOPUP_FLAG);
    Font *font = _gui_create_default_font();
    popup->items = _items_from_popup(context, ositem);
    context->func_popup_set_font(ositem, font);
    _component_init(&popup->component, context, PARAM(type, ekGUI_TYPE_POPUP), &ositem);
    context->func_popup_OnSelect(popup->component.ositem, obj_listener(popup, i_OnSelect, PopUp));
    font_destroy(&font);
    return popup;
}

/*---------------------------------------------------------------------------*/

void popup_OnSelect(PopUp *popup, Listener *listener)
{
    cassert_no_null(popup);
    _items_set_OnSelect(popup->items, listener);
}

/*---------------------------------------------------------------------------*/

void popup_tooltip(PopUp *popup, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(popup);
    if (text != NULL)
        ltext = _gui_respack_text(text, &popup->ttipid);
    popup->component.context->func_set_tooltip[ekGUI_TYPE_POPUP](popup->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void popup_add_elem(PopUp *popup, const char_t *text, const Image *image)
{
    cassert_no_null(popup);
    _items_add_elem(popup->items, text, image);
}

/*---------------------------------------------------------------------------*/

void popup_set_elem(PopUp *popup, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(popup);
    _items_set_elem(popup->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void popup_ins_elem(PopUp *popup, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(popup);
    _items_ins_elem(popup->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void popup_del_elem(PopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    _items_del_elem(popup->items, index);
}

/*---------------------------------------------------------------------------*/

void popup_clear(PopUp *popup)
{
    cassert_no_null(popup);
    _items_clear(popup->items);
}

/*---------------------------------------------------------------------------*/

uint32_t popup_count(const PopUp *popup)
{
    cassert_no_null(popup);
    return _items_count(popup->items);
}

/*---------------------------------------------------------------------------*/

void popup_list_height(PopUp *popup, const uint32_t num_elems)
{
    cassert_no_null(popup);
    _items_list_height(popup->items, num_elems);
}

/*---------------------------------------------------------------------------*/

void popup_selected(PopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    _items_selected(popup->items, index);
}

/*---------------------------------------------------------------------------*/

uint32_t popup_get_selected(const PopUp *popup)
{
    cassert_no_null(popup);
    return _items_get_selected(popup->items);
}

/*---------------------------------------------------------------------------*/

const char_t *popup_get_text(const PopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    return _items_get_text(popup->items, index);
}

/*---------------------------------------------------------------------------*/

const Image *popup_get_image(const PopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    return _items_get_image(popup->items, index);
}

/*---------------------------------------------------------------------------*/
void _popup_natural(PopUp *popup, const uint32_t di, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_bounds);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    if (di == 0)
    {
        uint32_t n = _items_count(popup->items);
        popup->size = kS2D_ZEROf;

        if (n > 0)
        {
            uint32_t i = 0;
            for (i = 0; i < n; ++i)
            {
                real32_t width, height;
                const char_t *text = _items_get_text(popup->items, i);
                popup->component.context->func_popup_bounds(popup->component.ositem, text, &width, &height);
                if (width > popup->size.width)
                    popup->size.width = width;
                if (height > popup->size.height)
                    popup->size.height = height;
            }
        }
        else
        {
            popup->component.context->func_popup_bounds(popup->component.ositem, "    ", &popup->size.width, &popup->size.height);
        }

        *dim0 = popup->size.width;
    }
    else
    {
        cassert(di == 1);
        *dim1 = popup->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _popup_locale(PopUp *popup)
{
    cassert_no_null(popup);
    _items_locale(popup->items);

    if (popup->ttipid != NULL)
    {
        const char_t *text = _gui_respack_text(popup->ttipid, NULL);
        popup->component.context->func_set_tooltip[ekGUI_TYPE_POPUP](popup->component.ositem, text);
    }
}

/*---------------------------------------------------------------------------*/

uint32_t _popup_size(const PopUp *popup)
{
    cassert_no_null(popup);
    return _items_count(popup->items);
}

/*---------------------------------------------------------------------------*/

void _popup_list_height(PopUp *popup, const uint32_t elems)
{
    popup_list_height(popup, elems);
}

/*---------------------------------------------------------------------------*/

void _popup_uint32(PopUp *popup, const uint32_t value)
{
    cassert_no_null(popup);
    _items_uint32(popup->items, value);
}

/*---------------------------------------------------------------------------*/

void _popup_add_enum_item(PopUp *popup, const char_t *text)
{
    cassert_no_null(popup);
    _items_add_elem(popup->items, text, NULL);
}
