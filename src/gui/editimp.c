/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: editimp.c
 *
 */

/* Shared implementation for Edit and Combo */

#include "editimp.inl"
#include "cell.inl"
#include "gui.inl"
#include <draw2d/color.h>
#include <draw2d/font.h>
#include <core/event.h>
#include <core/heap.h>
#include <core/strings.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>
#include <sewer/unicode.h>

struct _editimp_t
{
    bool_t is_focused;
    bool_t is_placeholder_active;
    bool_t passmode;
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
    void *ositem;
    FPtr_gctx_set_text func_set_text;
    FPtr_gctx_set_cptr func_set_font;
    FPtr_gctx_set_uint32 func_set_text_color;
    FPtr_gctx_set_uint32 func_set_bg_color;
    FPtr_gctx_set_bool func_set_passmode;
    Listener *OnFilter;
    Listener *OnChange;
    Listener *OnFocus;
};

/*---------------------------------------------------------------------------*/

static EditImp *i_create(void *ositem)
{
    EditImp *edit = heap_new0(EditImp);
    edit->passmode = FALSE;
    edit->text = str_c("");
    edit->font = _gui_create_default_font();
    edit->color = kCOLOR_DEFAULT;
    edit->focus_color = kCOLOR_DEFAULT;
    edit->bg_color = kCOLOR_DEFAULT;
    edit->bg_focus_color = kCOLOR_DEFAULT;
    edit->placeholder_color = kCOLOR_DEFAULT;
    edit->ositem = ositem;
    return edit;
}

/*---------------------------------------------------------------------------*/

EditImp *_editimp_from_edit(const GuiCtx *context, void *ositem)
{
    EditImp *edit = i_create(ositem);
    cassert_no_null(context);
    edit->func_set_text = context->func_edit_set_text;
    edit->func_set_font = context->func_edit_set_font;
    edit->func_set_text_color = context->func_edit_set_text_color;
    edit->func_set_bg_color = context->func_edit_set_bg_color;
    edit->func_set_passmode = context->func_edit_set_passmode;
    edit->func_set_font(edit->ositem, edit->font);
    return edit;
}

/*---------------------------------------------------------------------------*/

EditImp *_editimp_from_combo(const GuiCtx *context, void *ositem)
{
    EditImp *edit = i_create(ositem);
    cassert_no_null(context);
    edit->func_set_text = context->func_combo_set_text;
    edit->func_set_font = context->func_combo_set_font;
    edit->func_set_text_color = context->func_combo_set_text_color;
    edit->func_set_bg_color = context->func_combo_set_bg_color;
    edit->func_set_passmode = context->func_combo_set_passmode;
    edit->func_set_font(edit->ositem, edit->font);
    return edit;
}

/*---------------------------------------------------------------------------*/

void _editimp_destroy(EditImp **edit)
{
    cassert_no_null(edit);
    cassert_no_null(*edit);
    listener_destroy(&(*edit)->OnFilter);
    listener_destroy(&(*edit)->OnChange);
    listener_destroy(&(*edit)->OnFocus);
    str_destopt(&(*edit)->placeholder);
    str_destroy(&(*edit)->text);
    font_destroy(&(*edit)->font);
    ptr_destopt(font_destroy, &(*edit)->placeholder_font, Font);
    heap_delete(edit, EditImp);
}

/*---------------------------------------------------------------------------*/

static int32_t i_text_diff(const char_t *ctext, const char_t *ntext)
{
    int32_t n1 = (int32_t)unicode_nchars(ctext, ekUTF8);
    int32_t n2 = (int32_t)unicode_nchars(ntext, ekUTF8);
    return n2 - n1;
}

/*---------------------------------------------------------------------------*/

void _editimp_OnFilter(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type)
{
    const EvText *p = event_params(e, EvText);
    EvTextFilter *res = event_result(e, EvTextFilter);
    cassert_no_null(edit);
    cassert_no_null(p);
    cassert_no_null(res);
    cassert(event_type(e) == ekGUI_EVENT_TXTFILTER);

    res->apply = FALSE;

    /* Native edit doesn't known exactly the inserted or deleted text size */
    if (p->len == INT32_MAX)
        cast(p, EvText)->len = i_text_diff(tc(edit->text), p->text);

    if (cell != NULL)
        res->apply = _cell_filter_str(cell, p->text, res->text, sizeof(res->text));

    if (res->apply == FALSE)
    {
        if (cell != NULL)
            _cell_update_str(cell, p->text);

        if (edit->OnFilter != NULL)
            listener_pass_event_imp(edit->OnFilter, e, sender, sender_type);
    }

    if (res->apply == TRUE)
        str_upd(&edit->text, res->text);
    else
        str_upd(&edit->text, p->text);
}

/*---------------------------------------------------------------------------*/

void _editimp_OnChange(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type)
{
    const EvText *p = event_params(e, EvText);
    cassert_no_null(edit);
    cassert_no_null(p);
    cassert(event_type(e) == ekGUI_EVENT_TXTCHANGE);

    str_upd(&edit->text, p->text);

    if (cell != NULL)
        _cell_update_str(cell, p->text);

    if (edit->OnChange != NULL)
        listener_pass_event_imp(edit->OnChange, e, sender, sender_type);
}

/*---------------------------------------------------------------------------*/

static void i_update_placeholder(EditImp *edit)
{
    cassert_no_null(edit);
    if (edit->placeholder != NULL && edit->is_focused == FALSE && str_empty(edit->text) == TRUE)
    {
        if (_gui_effective_alt_font(edit->font, edit->placeholder_font) == TRUE)
            edit->func_set_font(edit->ositem, edit->placeholder_font);
        if (edit->placeholder_color != kCOLOR_DEFAULT)
            edit->func_set_text_color(edit->ositem, edit->placeholder_color);
        edit->func_set_text(edit->ositem, tc(edit->placeholder));

        if (edit->passmode == TRUE)
            edit->func_set_passmode(edit->ositem, FALSE);

        edit->is_placeholder_active = TRUE;
    }
    else if (edit->is_placeholder_active == TRUE)
    {
        if (_gui_effective_alt_font(edit->font, edit->placeholder_font) == TRUE)
            edit->func_set_font(edit->ositem, edit->font);

        if (edit->placeholder_color != kCOLOR_DEFAULT)
        {
            if (edit->is_focused == TRUE && edit->focus_color != kCOLOR_DEFAULT)
                edit->func_set_text_color(edit->ositem, edit->focus_color);
            else
                edit->func_set_text_color(edit->ositem, edit->color);
        }

        edit->func_set_text(edit->ositem, tc(edit->text));
        edit->is_placeholder_active = FALSE;
    }
    else
    {
        edit->func_set_passmode(edit->ositem, edit->passmode);
    }
}

/*---------------------------------------------------------------------------*/

void _editimp_OnFocus(EditImp *edit, Event *e, Cell *cell, void *sender, const char_t *sender_type)
{
    const bool_t *p = event_params(e, bool_t);
    cassert_no_null(edit);
    cassert_no_null(p);
    if (*p == TRUE)
    {
        char_t filter[128];

        if (edit->focus_color != kCOLOR_DEFAULT)
            edit->func_set_text_color(edit->ositem, edit->focus_color);

        if (edit->bg_focus_color != kCOLOR_DEFAULT)
            edit->func_set_bg_color(edit->ositem, edit->bg_focus_color);

        if (cell != NULL)
        {
            if (_cell_filter_str(cell, tc(edit->text), filter, sizeof(filter)) == TRUE)
                edit->func_set_text(edit->ositem, filter);
        }
    }
    else
    {
        if (edit->focus_color != kCOLOR_DEFAULT)
            edit->func_set_text_color(edit->ositem, edit->color);

        if (edit->bg_focus_color != kCOLOR_DEFAULT)
            edit->func_set_bg_color(edit->ositem, edit->bg_color);
    }

    edit->is_focused = *p;
    i_update_placeholder(edit);

    if (edit->OnFocus != NULL)
        listener_pass_event_imp(edit->OnFocus, e, sender, sender_type);
}

/*---------------------------------------------------------------------------*/

void _editimp_set_OnFilter(EditImp *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnFilter, listener);
}

/*---------------------------------------------------------------------------*/

void _editimp_set_OnChange(EditImp *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void _editimp_set_OnFocus(EditImp *edit, Listener *listener)
{
    cassert_no_null(edit);
    listener_update(&edit->OnFocus, listener);
}

/*---------------------------------------------------------------------------*/

void _editimp_text(EditImp *edit, const char_t *text)
{
    cassert_no_null(edit);
    str_upd(&edit->text, text);
    edit->func_set_text(edit->ositem, text);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void _editimp_font(EditImp *edit, const Font *font)
{
    cassert_no_null(edit);
    if (_gui_update_font(&edit->font, &edit->placeholder_font, font) == TRUE)
    {
        edit->func_set_font(edit->ositem, edit->font);
        i_update_placeholder(edit);
    }
}

/*---------------------------------------------------------------------------*/

void _editimp_passmode(EditImp *edit, const bool_t passmode)
{
    cassert_no_null(edit);
    if (edit->passmode != passmode)
    {
        edit->passmode = passmode;
        i_update_placeholder(edit);
    }
}

/*---------------------------------------------------------------------------*/

void _editimp_color(EditImp *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->color = color;
    edit->func_set_text_color(edit->ositem, color);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void _editimp_color_focus(EditImp *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->focus_color = color;
}

/*---------------------------------------------------------------------------*/

void _editimp_bgcolor(EditImp *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->bg_color = color;
    edit->func_set_bg_color(edit->ositem, color);
}

/*---------------------------------------------------------------------------*/

void _editimp_bgcolor_focus(EditImp *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->bg_focus_color = color;
    if (edit->is_focused)
        edit->func_set_bg_color(edit->ositem, color);
}

/*---------------------------------------------------------------------------*/

void _editimp_phtext(EditImp *edit, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(edit);
    if (text != NULL)
        ltext = _gui_respack_text(text, &edit->placid);
    str_upd(&edit->placeholder, ltext);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void _editimp_phcolor(EditImp *edit, const color_t color)
{
    cassert_no_null(edit);
    edit->placeholder_color = color;
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

void _editimp_phstyle(EditImp *edit, const uint32_t fstyle)
{
    cassert_no_null(edit);
    ptr_destopt(font_destroy, &edit->placeholder_font, Font);
    if (fstyle != font_style(edit->font))
        edit->placeholder_font = font_with_style(edit->font, fstyle);
    i_update_placeholder(edit);
}

/*---------------------------------------------------------------------------*/

const char_t *_editimp_get_text(const EditImp *edit)
{
    cassert_no_null(edit);
    return tc(edit->text);
}

/*---------------------------------------------------------------------------*/

void _editimp_locale(EditImp *edit)
{
    if (edit->placid != NULL)
    {
        const char_t *text = _gui_respack_text(edit->placid, NULL);
        str_upd(&edit->placeholder, text);
        i_update_placeholder(edit);
    }
}
