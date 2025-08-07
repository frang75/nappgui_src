/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: edit.c
 *
 */

/* Edit Box */

#include "edit.h"
#include "edit.inl"
#include "editimp.inl"
#include "component.inl"
#include "gui.inl"
#include <geom2d/s2d.h>
#include <draw2d/guictx.h>
#include <core/event.h>
#include <core/objh.h>
#include <sewer/cassert.h>

struct _edit_t
{
    GuiComponent component;
    uint32_t flags;
    real32_t width;
    real32_t height;
    S2Df size;
    ResId ttipid;
    EditImp *impl;
};

/*---------------------------------------------------------------------------*/

void _edit_destroy(Edit **edit)
{
    cassert_no_null(edit);
    cassert_no_null(*edit);
    _component_destroy_imp(&(*edit)->component);
    _editimp_destroy(&(*edit)->impl);
    obj_delete(edit, Edit);
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(Edit *edit, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(edit);
    cell = _component_cell(&edit->component);
    _editimp_OnFilter(edit->impl, e, cell, cast(edit, void), "Edit");
}

/*---------------------------------------------------------------------------*/

static void i_OnChange(Edit *edit, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(edit);
    cell = _component_cell(&edit->component);
    _editimp_OnChange(edit->impl, e, cell, cast(edit, void), "Edit");
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(Edit *edit, Event *e)
{
    Cell *cell = NULL;
    cassert_no_null(edit);
    cell = _component_cell(&edit->component);
    _editimp_OnFocus(edit->impl, e, cell, cast(edit, void), "Edit");
}

/*---------------------------------------------------------------------------*/

static Edit *i_create(const align_t halign, const uint32_t flags)
{
    const GuiCtx *context = guictx_get_current();
    Edit *edit = obj_new0(Edit);
    void *ositem = context->func_create[ekGUI_TYPE_EDITBOX](flags);
    edit->flags = flags;
    edit->width = 100;
    edit->height = -1;
    edit->impl = _editimp_from_edit(context, ositem);
    context->func_edit_set_align(ositem, (enum_t)halign);
    _component_init(&edit->component, context, PARAM(type, ekGUI_TYPE_EDITBOX), &ositem);
    context->func_edit_OnFilter(edit->component.ositem, obj_listener(edit, i_OnFilter, Edit));
    context->func_edit_OnChange(edit->component.ositem, obj_listener(edit, i_OnChange, Edit));
    context->func_edit_OnFocus(edit->component.ositem, obj_listener(edit, i_OnFocus, Edit));
    return edit;
}

/*---------------------------------------------------------------------------*/

Edit *edit_create(void)
{
    return i_create(ekLEFT, ekEDIT_SINGLE);
}

/*---------------------------------------------------------------------------*/

Edit *edit_multiline(void)
{
    return i_create(ekLEFT, ekEDIT_MULTI);
}

/*---------------------------------------------------------------------------*/

void edit_OnFilter(Edit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _editimp_set_OnFilter(edit->impl, listener);
}

/*---------------------------------------------------------------------------*/

void edit_OnChange(Edit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _editimp_set_OnChange(edit->impl, listener);
}

/*---------------------------------------------------------------------------*/

void edit_OnFocus(Edit *edit, Listener *listener)
{
    cassert_no_null(edit);
    _editimp_set_OnFocus(edit->impl, listener);
}

/*---------------------------------------------------------------------------*/

void edit_min_width(Edit *edit, const real32_t width)
{
    cassert_no_null(edit);
    cassert(width > 0);
    edit->width = width;
}

/*---------------------------------------------------------------------------*/

void edit_min_height(Edit *edit, const real32_t height)
{
    cassert_no_null(edit);
    cassert(height > 0);
    cassert(edit_get_type(edit->flags) == ekEDIT_MULTI);
    edit->height = height;
}

/*---------------------------------------------------------------------------*/

void edit_text(Edit *edit, const char_t *text)
{
    cassert_no_null(edit);
    _editimp_text(edit->impl, text);
}

/*---------------------------------------------------------------------------*/

void edit_font(Edit *edit, const Font *font)
{
    cassert_no_null(edit);
    _editimp_font(edit->impl, font);
}

/*---------------------------------------------------------------------------*/

void edit_align(Edit *edit, const align_t align)
{
    cassert_no_null(edit);
    edit->component.context->func_edit_set_align(edit->component.ositem, (enum_t)align);
}

/*---------------------------------------------------------------------------*/

void edit_passmode(Edit *edit, const bool_t passmode)
{
    cassert_no_null(edit);
    _editimp_passmode(edit->impl, passmode);
}

/*---------------------------------------------------------------------------*/

void edit_editable(Edit *edit, const bool_t is_editable)
{
    cassert_no_null(edit);
    edit->component.context->func_edit_set_editable(edit->component.ositem, is_editable);
}

/*---------------------------------------------------------------------------*/

void edit_autoselect(Edit *edit, const bool_t autoselect)
{
    cassert_no_null(edit);
    edit->component.context->func_edit_set_autoselect(edit->component.ositem, autoselect);
}

/*---------------------------------------------------------------------------*/

void edit_select(Edit *edit, const int32_t start, const int32_t end)
{
    cassert_no_null(edit);
    edit->component.context->func_edit_set_select(edit->component.ositem, start, end);
}

/*---------------------------------------------------------------------------*/

void edit_tooltip(Edit *edit, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(edit);
    if (text != NULL)
        ltext = _gui_respack_text(text, &edit->ttipid);
    edit->component.context->func_set_tooltip[ekGUI_TYPE_EDITBOX](edit->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void edit_color(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    _editimp_color(edit->impl, color);
}

/*---------------------------------------------------------------------------*/

void edit_color_focus(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    _editimp_color_focus(edit->impl, color);
}

/*---------------------------------------------------------------------------*/

void edit_bgcolor(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    _editimp_bgcolor(edit->impl, color);
}

/*---------------------------------------------------------------------------*/

void edit_bgcolor_focus(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    _editimp_bgcolor_focus(edit->impl, color);
}

/*---------------------------------------------------------------------------*/

void edit_phtext(Edit *edit, const char_t *text)
{
    cassert_no_null(edit);
    _editimp_phtext(edit->impl, text);
}

/*---------------------------------------------------------------------------*/

void edit_phcolor(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    _editimp_phcolor(edit->impl, color);
}

/*---------------------------------------------------------------------------*/

void edit_phstyle(Edit *edit, const uint32_t fstyle)
{
    cassert_no_null(edit);
    _editimp_phstyle(edit->impl, fstyle);
}

/*---------------------------------------------------------------------------*/

void edit_vpadding(Edit *edit, const real32_t padding)
{
    cassert_no_null(edit);
    cassert_no_nullf(edit->component.context->func_edit_set_vpadding);
    edit->component.context->func_edit_set_vpadding(edit->component.ositem, padding);
}

/*---------------------------------------------------------------------------*/

void edit_copy(const Edit *edit)
{
    cassert_no_null(edit);
    cassert_no_nullf(edit->component.context->func_edit_clipboard);
    edit->component.context->func_edit_clipboard(edit->component.ositem, ekCLIPBOARD_COPY);
}

/*---------------------------------------------------------------------------*/

void edit_cut(Edit *edit)
{
    cassert_no_null(edit);
    cassert_no_nullf(edit->component.context->func_edit_clipboard);
    edit->component.context->func_edit_clipboard(edit->component.ositem, ekCLIPBOARD_CUT);
}

/*---------------------------------------------------------------------------*/

void edit_paste(Edit *edit)
{
    cassert_no_null(edit);
    cassert_no_nullf(edit->component.context->func_edit_clipboard);
    edit->component.context->func_edit_clipboard(edit->component.ositem, ekCLIPBOARD_PASTE);
}

/*---------------------------------------------------------------------------*/

const char_t *edit_get_text(const Edit *edit)
{
    cassert_no_null(edit);
    return _editimp_get_text(edit->impl);
}

/*---------------------------------------------------------------------------*/

real32_t edit_get_height(const Edit *edit)
{
    real32_t width, height;
    cassert_no_null(edit);
    _edit_natural(cast(edit, Edit), 0, &width, &height);
    _edit_natural(cast(edit, Edit), 1, &width, &height);
    return height;
}

/*---------------------------------------------------------------------------*/

void _edit_natural(Edit *edit, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(edit);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        edit->component.context->func_edit_bounds(edit->component.ositem, edit->width, 1, &edit->size.width, &edit->size.height);
        *dim0 = edit->size.width;
    }
    else
    {
        /* The height set by the user cannot be lower than the height of a single - line control */
        cassert(i == 1);
        if (edit_get_type(edit->flags) == ekEDIT_MULTI && edit->height > edit->size.height)
            *dim1 = edit->height;
        else
            *dim1 = edit->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _edit_locale(Edit *edit)
{
    cassert_no_null(edit);
    _editimp_locale(edit->impl);
    if (edit->ttipid != NULL)
    {
        const char_t *text = _gui_respack_text(edit->ttipid, NULL);
        edit->component.context->func_set_tooltip[ekGUI_TYPE_EDITBOX](edit->component.ositem, text);
    }
}

/*---------------------------------------------------------------------------*/

bool_t _edit_is_multiline(const Edit *edit)
{
    cassert_no_null(edit);
    if (edit_get_type(edit->flags) == ekEDIT_MULTI)
        return TRUE;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

void _edit_text(Edit *edit, const char_t *text)
{
    edit_text(edit, text);
}
