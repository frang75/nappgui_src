/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: combo.c
 *
 */

/* Combo box */

#include "combo.h"
#include "combo.inl"
#include "editimp.inl"
#include "items.inl"
#include "component.inl"
#include "gui.inl"
#include <draw2d/guictx.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _combo_t
{
    GuiComponent component;
    S2Df size;
    ResId ttipid;
    EditImp *eimpl;
    Items *items;
};

/*---------------------------------------------------------------------------*/

void _combo_destroy(Combo **combo)
{
    cassert_no_null(combo);
    cassert_no_null(*combo);
    _component_destroy_imp(&(*combo)->component);
    _editimp_destroy(&(*combo)->eimpl);
    _items_destroy(&(*combo)->items);
    obj_delete(combo, Combo);
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(Combo *combo, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(combo);
    cell = _component_cell(&combo->component);
    _editimp_OnFilter(combo->eimpl, e, cell, cast(combo, void), "Combo");
}

/*---------------------------------------------------------------------------*/

static void i_OnChange(Combo *combo, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(combo);
    cell = _component_cell(&combo->component);
    _editimp_OnChange(combo->eimpl, e, cell, cast(combo, void), "Combo");
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(Combo *combo, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(combo);
    cell = _component_cell(&combo->component);
    _editimp_OnFocus(combo->eimpl, e, cell, cast(combo, void), "Combo");
}

/*---------------------------------------------------------------------------*/

static void i_OnSelect(Combo *combo, Event *e)
{
    Cell *cell = NULL;
    const char_t *text = NULL;
    cassert_no_null(combo);
    cell = _component_cell(&combo->component);
    text = _items_OnSelect(combo->items, e, cell, cast(combo, void), "Combo");
    _editimp_text(combo->eimpl, text);
}

/*---------------------------------------------------------------------------*/

Combo *combo_create(void)
{
    const GuiCtx *context = guictx_get_current();
    Combo *combo = obj_new0(Combo);
    void *ositem = context->func_create[ekGUI_TYPE_COMBOBOX](ekCOMBO_FLAG);
    combo->size.width = 100;
    combo->eimpl = _editimp_from_combo(context, ositem);
    combo->items = _items_from_combo(context, ositem);
    _component_init(&combo->component, context, PARAM(type, ekGUI_TYPE_COMBOBOX), &ositem);
    context->func_combo_OnFilter(combo->component.ositem, obj_listener(combo, i_OnFilter, Combo));
    context->func_combo_OnChange(combo->component.ositem, obj_listener(combo, i_OnChange, Combo));
    context->func_combo_OnFocus(combo->component.ositem, obj_listener(combo, i_OnFocus, Combo));
    context->func_combo_OnSelect(combo->component.ositem, obj_listener(combo, i_OnSelect, Combo));
    return combo;
}

/*---------------------------------------------------------------------------*/

void combo_OnFilter(Combo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _editimp_set_OnFilter(combo->eimpl, listener);
}

/*---------------------------------------------------------------------------*/

void combo_OnChange(Combo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _editimp_set_OnChange(combo->eimpl, listener);
}

/*---------------------------------------------------------------------------*/

void combo_OnFocus(Combo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _editimp_set_OnFocus(combo->eimpl, listener);
}

/*---------------------------------------------------------------------------*/

void combo_OnSelect(Combo *combo, Listener *listener)
{
    cassert_no_null(combo);
    _items_set_OnSelect(combo->items, listener);
}

/*---------------------------------------------------------------------------*/

void combo_min_width(Combo *combo, const real32_t width)
{
    cassert_no_null(combo);
    cassert(width > 0);
    combo->size.width = width;
}

/*---------------------------------------------------------------------------*/

void combo_text(Combo *combo, const char_t *text)
{
    cassert_no_null(combo);
    _editimp_text(combo->eimpl, text);
}

/*---------------------------------------------------------------------------*/

void combo_align(Combo *combo, const align_t align)
{
    cassert_no_null(combo);
    combo->component.context->func_combo_set_align(combo->component.ositem, (enum_t)align);
}

/*---------------------------------------------------------------------------*/

void combo_passmode(Combo *combo, const bool_t passmode)
{
    cassert_no_null(combo);
    _editimp_passmode(combo->eimpl, passmode);
}

/*---------------------------------------------------------------------------*/

void combo_editable(Combo *combo, const bool_t is_editable)
{
    cassert_no_null(combo);
    combo->component.context->func_combo_set_editable(combo->component.ositem, is_editable);
}

/*---------------------------------------------------------------------------*/

void combo_autoselect(Combo *combo, const bool_t autoselect)
{
    cassert_no_null(combo);
    combo->component.context->func_combo_set_autoselect(combo->component.ositem, autoselect);
}

/*---------------------------------------------------------------------------*/

void combo_select(Combo *combo, const int32_t start, const int32_t end)
{
    cassert_no_null(combo);
    combo->component.context->func_combo_set_select(combo->component.ositem, start, end);
}

/*---------------------------------------------------------------------------*/

void combo_tooltip(Combo *combo, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(combo);
    if (text != NULL)
        ltext = _gui_respack_text(text, &combo->ttipid);
    combo->component.context->func_set_tooltip[ekGUI_TYPE_COMBOBOX](combo->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void combo_color(Combo *combo, const color_t color)
{
    cassert_no_null(combo);
    _editimp_color(combo->eimpl, color);
}

/*---------------------------------------------------------------------------*/

void combo_color_focus(Combo *combo, const color_t color)
{
    cassert_no_null(combo);
    _editimp_color_focus(combo->eimpl, color);
}

/*---------------------------------------------------------------------------*/

void combo_bgcolor(Combo *combo, const color_t color)
{
    cassert_no_null(combo);
    _editimp_bgcolor(combo->eimpl, color);
}

/*---------------------------------------------------------------------------*/

void combo_bgcolor_focus(Combo *combo, const color_t color)
{
    cassert_no_null(combo);
    _editimp_bgcolor_focus(combo->eimpl, color);
}

/*---------------------------------------------------------------------------*/

void combo_phtext(Combo *combo, const char_t *text)
{
    cassert_no_null(combo);
    _editimp_phtext(combo->eimpl, text);
}

/*---------------------------------------------------------------------------*/

void combo_phcolor(Combo *combo, const color_t color)
{
    cassert_no_null(combo);
    _editimp_phcolor(combo->eimpl, color);
}

/*---------------------------------------------------------------------------*/

void combo_phstyle(Combo *combo, const uint32_t fstyle)
{
    cassert_no_null(combo);
    _editimp_phstyle(combo->eimpl, fstyle);
}

/*---------------------------------------------------------------------------*/

void combo_copy(const Combo *combo)
{
    cassert_no_null(combo);
    cassert_no_nullf(combo->component.context->func_combo_clipboard);
    combo->component.context->func_combo_clipboard(combo->component.ositem, ekCLIPBOARD_COPY);
}

/*---------------------------------------------------------------------------*/

void combo_cut(Combo *combo)
{
    cassert_no_null(combo);
    cassert_no_nullf(combo->component.context->func_combo_clipboard);
    combo->component.context->func_combo_clipboard(combo->component.ositem, ekCLIPBOARD_CUT);
}

/*---------------------------------------------------------------------------*/

void combo_paste(Combo *combo)
{
    cassert_no_null(combo);
    cassert_no_nullf(combo->component.context->func_combo_clipboard);
    combo->component.context->func_combo_clipboard(combo->component.ositem, ekCLIPBOARD_PASTE);
}

/*---------------------------------------------------------------------------*/

void combo_add_elem(Combo *combo, const char_t *text, const Image *image)
{
    cassert_no_null(combo);
    _items_add_elem(combo->items, text, image);
}

/*---------------------------------------------------------------------------*/

void combo_set_elem(Combo *combo, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(combo);
    _items_set_elem(combo->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void combo_ins_elem(Combo *combo, const uint32_t index, const char_t *text, const Image *image)
{
    cassert_no_null(combo);
    _items_ins_elem(combo->items, index, text, image);
}

/*---------------------------------------------------------------------------*/

void combo_del_elem(Combo *combo, const uint32_t index)
{
    cassert_no_null(combo);
    _items_del_elem(combo->items, index);
}

/*---------------------------------------------------------------------------*/

void combo_clear(Combo *combo)
{
    cassert_no_null(combo);
    _items_clear(combo->items);
}

/*---------------------------------------------------------------------------*/

uint32_t combo_count(const Combo *combo)
{
    cassert_no_null(combo);
    return _items_count(combo->items);
}

/*---------------------------------------------------------------------------*/

void combo_list_height(Combo *combo, const uint32_t elems)
{
    cassert_no_null(combo);
    _items_list_height(combo->items, elems);
}

/*---------------------------------------------------------------------------*/

void combo_selected(Combo *combo, const uint32_t index)
{
    cassert_no_null(combo);
    _items_selected(combo->items, index);
}

/*---------------------------------------------------------------------------*/

uint32_t combo_get_selected(const Combo *combo)
{
    cassert_no_null(combo);
    return _items_get_selected(combo->items);
}

/*---------------------------------------------------------------------------*/

const char_t *combo_get_text(const Combo *combo, const uint32_t index)
{
    cassert_no_null(combo);
    if (index != UINT32_MAX)
        return _items_get_text(combo->items, index);
    else
        return _editimp_get_text(combo->eimpl);
}

/*---------------------------------------------------------------------------*/

const Image *combo_get_image(const Combo *combo, const uint32_t index)
{
    cassert_no_null(combo);
    return _items_get_image(combo->items, index);
}

/*---------------------------------------------------------------------------*/

void _combo_natural(Combo *combo, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(combo);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        combo->component.context->func_combo_bounds(combo->component.ositem, combo->size.width, dim0, &combo->size.height);
    }
    else
    {
        cassert(i == 1);
        *dim1 = combo->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _combo_locale(Combo *combo)
{
    cassert_no_null(combo);
    _editimp_locale(combo->eimpl);
    _items_locale(combo->items);

    if (combo->ttipid != NULL)
    {
        const char_t *text = _gui_respack_text(combo->ttipid, NULL);
        combo->component.context->func_set_tooltip[ekGUI_TYPE_COMBOBOX](combo->component.ositem, text);
    }
}
