/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: edit.c
 *
 */

/* Edit Box */

#include "edit.h"
#include "edit.inl"
#include "cell.inl"
#include "component.inl"
#include "gui.inl"
#include "guictx.h"

#include "cassert.h"
#include "color.h"
#include "event.h"
#include "font.h"
#include "ptr.h"
#include "objh.h"
#include "strings.h"
#include "s2d.h"

struct _edit_t
{
    GuiComponent component;
    uint32_t flags;
    S2Df size;
    bool_t is_focused;
    bool_t is_placeholder_active;
    ResId ttipid;
    ResId placid;
    String *placeholder;
    String *text;
    Font *font;
    Font *placeholder_font;
    color_t color;
    color_t focus_color;
    color_t bg_color;
    color_t bg_focus_color;
    color_t placeholder_color;
    Listener *OnFilter;
    Listener *OnChange;
};

/*---------------------------------------------------------------------------*/

void _edit_destroy(Edit **edit)
{
    cassert_no_null(edit);
    cassert_no_null(*edit);
    _component_destroy_imp(&(*edit)->component);
    listener_destroy(&(*edit)->OnFilter);
    listener_destroy(&(*edit)->OnChange);
    ptr_destopt(str_destroy, &(*edit)->placeholder, String);
    str_destroy(&(*edit)->text);
    font_destroy(&(*edit)->font);
    ptr_destopt(font_destroy, &(*edit)->placeholder_font, Font);
    obj_delete(edit, Edit);
}

/*---------------------------------------------------------------------------*/

static void i_OnFilter(Edit *edit, Event *e)
{
    EvTextFilter *result = event_result(e, EvTextFilter);
    const EvText *params = event_params(e, EvText);
    cassert_no_null(edit);
    result->apply = _cell_filter_str(edit->component.parent, params->text, result->text, sizeof(result->text));
    if (result->apply == FALSE)
    {
        _cell_upd_string(edit->component.parent, params->text);

        if (edit->OnFilter != NULL)
            listener_pass_event(edit->OnFilter, e, edit, Edit);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnChange(Edit *edit, Event *e)
{
    const EvText *params = event_params(e, EvText);
    cassert_no_null(edit);
    cassert_no_null(params);
    cassert(event_type(e) == ekGUI_EVENT_TXTCHANGE);
    cassert(event_sender_imp(e, NULL) == edit->component.ositem);
    str_upd(&edit->text, params->text);

    _cell_upd_string(edit->component.parent, params->text);

    if (edit->OnChange != NULL)
        listener_pass_event(edit->OnChange, e, edit, Edit);
}

/*---------------------------------------------------------------------------*/

static Edit *i_create(const align_t halign, const uint32_t flags)
{
    const GuiCtx *context = guictx_get_current();
    Edit *edit = obj_new0(Edit);
    void *ositem = NULL;
    edit->flags = flags;
    edit->text = str_c("");
    edit->font = _gui_create_default_font();
    edit->color = kCOLOR_TRANSPARENT;
    edit->focus_color = kCOLOR_TRANSPARENT;
    edit->bg_color = kCOLOR_TRANSPARENT;
    edit->bg_focus_color = kCOLOR_TRANSPARENT;
    edit->placeholder_color = kCOLOR_TRANSPARENT;
    ositem = context->func_create[ekGUI_TYPE_EDITBOX](flags);
    context->func_edit_set_font(ositem, edit->font);
    context->func_edit_set_align(ositem, (enum_t)halign);
    _component_init(&edit->component, context, PARAM(type, ekGUI_TYPE_EDITBOX), &ositem);
    context->func_edit_OnFilter(edit->component.ositem, obj_listener(edit, i_OnFilter, Edit));
    context->func_edit_OnChange(edit->component.ositem, obj_listener(edit, i_OnChange, Edit));
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
    listener_update(&edit->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void edit_OnChange(Edit *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

static void i_update_placeholder(Edit *edit)
{
    cassert_no_null(edit);
    if (edit->placeholder != NULL && edit->is_focused == FALSE && str_equ(edit->text, "") == TRUE)
    {
        if (_gui_effective_alt_font(edit->font, edit->placeholder_font) == TRUE)
            edit->component.context->func_edit_set_font(edit->component.ositem, edit->placeholder_font);
        if (edit->placeholder_color != UINT32_MAX)
            edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->placeholder_color);
        edit->component.context->func_edit_set_text(edit->component.ositem, tc(edit->placeholder));
        edit->is_placeholder_active = TRUE;
    }
    else if (edit->is_placeholder_active == TRUE)
    {
        if (_gui_effective_alt_font(edit->font, edit->placeholder_font) == TRUE)
            edit->component.context->func_edit_set_font(edit->component.ositem, edit->font);

        if (edit->placeholder_color != UINT32_MAX)
        {
            if (edit->is_focused == TRUE && edit->focus_color != UINT32_MAX)
                edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->focus_color);
            else
                edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->color);
        }

        edit->component.context->func_edit_set_text(edit->component.ositem, tc(edit->text));
        edit->is_placeholder_active = FALSE;
    }
    else
    {
        edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->color);
    }
}

/*---------------------------------------------------------------------------*/

void edit_text(Edit *edit, const char_t *text)
{
    cassert_no_null(edit);
    str_upd(&edit->text, text);
    edit->component.context->func_edit_set_text(edit->component.ositem, text);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void edit_font(Edit *edit, const Font *font)
{
    cassert_no_null(edit);
    if (_gui_update_font(&edit->font, &edit->placeholder_font, font) == TRUE)
    {
        edit->component.context->func_edit_set_font(edit->component.ositem, edit->font);
        i_update_placeholder(edit);
    }
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
    edit->component.context->func_edit_set_passmode(edit->component.ositem, passmode);
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

void edit_tooltip(Edit *edit, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(edit);
    if (text != NULL)
        ltext = _gui_respack_text(text, &edit->ttipid);
    edit->component.context->func_set_tooltip[ekGUI_TYPE_EDITBOX](edit->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

static void i_OnFocus(Edit *edit, Event *event)
{
    const bool_t *params = event_params(event, bool_t);
    cassert_no_null(edit);
    cassert_no_null(params);
    if (*params == TRUE)
    {
        char_t filter[128];

        if (edit->focus_color != UINT32_MAX)
            edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->focus_color);
        if (edit->bg_focus_color != UINT32_MAX)
            edit->component.context->func_edit_set_bg_color(edit->component.ositem, edit->bg_focus_color);

        if (_cell_filter_str(edit->component.parent, tc(edit->text), filter, sizeof(filter)) == TRUE)
            edit->component.context->func_edit_set_text(edit->component.ositem, filter);
    }
    else
    {
        if (edit->focus_color != UINT32_MAX)
            edit->component.context->func_edit_set_text_color(edit->component.ositem, edit->color);
        if (edit->bg_focus_color != UINT32_MAX)
            edit->component.context->func_edit_set_bg_color(edit->component.ositem, edit->bg_color);
    }

    edit->is_focused = *params;
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

static void i_update_focus_listener(Edit *edit)
{
    cassert_no_null(edit);
    if (edit->focus_color != UINT32_MAX || edit->bg_focus_color != UINT32_MAX || edit->placeholder != NULL)
    {
        edit->component.context->func_edit_OnFocus(edit->component.ositem, obj_listener(edit, i_OnFocus, Edit));
    }
    else
    {
        edit->component.context->func_edit_OnFocus(edit->component.ositem, NULL);
    }
}

/*---------------------------------------------------------------------------*/

void edit_color(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->color = color;
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void edit_color_focus(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->focus_color = color;
    i_update_focus_listener(edit);
}

/*---------------------------------------------------------------------------*/

void edit_bgcolor(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->bg_color = color;
    edit->component.context->func_edit_set_bg_color(edit->component.ositem, color);
}

/*---------------------------------------------------------------------------*/

void edit_bgcolor_focus(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->bg_focus_color = color;
    i_update_focus_listener(edit);
    if (edit->is_focused)
        edit->component.context->func_edit_set_bg_color(edit->component.ositem, color);
}

/*---------------------------------------------------------------------------*/

void edit_phtext(Edit *edit, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(edit);
    if (text != NULL)
        ltext = _gui_respack_text(text, &edit->placid);
    str_upd(&edit->placeholder, ltext);
    i_update_focus_listener(edit);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void edit_phcolor(Edit *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->placeholder_color = color;
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void edit_phstyle(Edit *edit, const uint32_t fstyle)
{
    cassert_no_null(edit);
    ptr_destopt(font_destroy, &edit->placeholder_font, Font);
    if (fstyle != font_style(edit->font))
        edit->placeholder_font = font_with_style(edit->font, fstyle);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

const char_t *edit_get_text(const Edit *edit)
{
    cassert_no_null(edit);
    return tc(edit->text);
}

/*---------------------------------------------------------------------------*/

void _edit_dimension(Edit *edit, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(edit);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        edit->component.context->func_edit_bounds(edit->component.ositem, 100.f, 1, &edit->size.width, &edit->size.height);
        *dim0 = edit->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = edit->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _edit_locale(Edit *edit)
{
    cassert_no_null(edit);
    if (edit->placid != NULL)
    {
        const char_t *text = _gui_respack_text(edit->placid, NULL);
        str_upd(&edit->placeholder, text);
        i_update_placeholder(edit);
    }

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
