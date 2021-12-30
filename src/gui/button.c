/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: button.c
 *
 */

/* Button */

#include "button.h"
#include "button.inl"
#include "component.inl"
#include "cell.inl"
#include "gui.inl"
#include "guicontexth.inl"
#include "gbind.inl"
#include "obj.inl"
#include "panel.inl"

#include "cassert.h"
#include "event.h"
#include "font.h"
#include "image.h"
#include "ptr.h"
#include "s2d.h"
#include "strings.h"
#include "v2d.h"

struct _button_t
{
    GuiComponent component;
    button_flag_t flags;
    S2Df size;
    ResId textid;
    ResId taltid;
    ResId ttipid;
    String *text;
    String *talt;
    Image *image;
    Image *imalt;
    Listener *OnClick;
};

/*---------------------------------------------------------------------------*/

void _button_destroy(Button **button)
{
    cassert_no_null(button);
    cassert_no_null(*button);
    _component_destroy_imp(&(*button)->component);
    listener_destroy(&(*button)->OnClick);
    str_destroy(&(*button)->text);
    ptr_destopt(str_destroy, &(*button)->talt, String);
    ptr_destopt(image_destroy, &(*button)->image, Image);
    ptr_destopt(image_destroy, &(*button)->imalt, Image);
    obj_delete(button, Button);
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_update_button(Button *button, const state_t state)
{
    cassert_no_null(button);
    if (button_type(button->flags) == ekBTFLATGLE)
    {
        if (button->talt != NULL)
        {
            if (state == ekON)
            {
                button->component.context->func_set_tooltip[ekGUI_COMPONENT_BUTTON](button->component.ositem, tc(button->talt));
            }
            else
            {
                cassert(state == ekOFF);
                button->component.context->func_set_tooltip[ekGUI_COMPONENT_BUTTON](button->component.ositem, tc(button->text));
            }
        }

        if (button->imalt != NULL)
        {
            if (state == ekON)
            {
                button->component.context->func_button_set_image(button->component.ositem, button->imalt);
            }
            else
            {
                cassert(state == ekOFF);
                button->component.context->func_button_set_image(button->component.ositem, button->image);
            }
        }
    }
    else if (button_type(button->flags) == ekBTRADIO)
    {
        _cell_set_radio(button->component.parent);
    }
}

/*---------------------------------------------------------------------------*/

static void i_OnClick(Button *button, Event *event)
{
    EvButton *params = NULL;
    Button *sender = button;

    cassert_no_null(button);
    cassert(button->component.ositem == event_sender_imp(event, NULL));
    cassert_no_null(event);
    cassert(event_type(event) == ekEVBUTTON);
    params = event_params(event, EvButton);
    cassert_no_null(params);
    cassert(params->index == 0);

    i_update_button(button, params->state);

    switch (button_type(button->flags)) {
    case ekBTRADIO:
    {
        Cell* cell = _cell_radio_dbind_cell(button->component.parent);
        params->index = _cell_radio_index(button->component.parent);

        if (cell != NULL)
            _cell_upd_uint32(cell, params->index);

        if (button->OnClick == NULL)
            sender = _cell_radio_listener(button->component.parent);

        break;
    }

    case ekBTCHECK2:
    case ekBTFLATGLE:
        _cell_upd_bool(button->component.parent, params->state == ekOFF ? FALSE : TRUE);
        break;

    case ekBTCHECK3:
    {
        uint32_t v = 0;
        switch (params->state) {
        case ekOFF:
            v = 0;
            break;
        case ekON:
            v = 1;
            break;
        case ekMIXED:
            v = 2;
            break;
            cassert_default();
        }

        _cell_upd_uint32(button->component.parent, v);
        break;
    }}

    if (sender && sender->OnClick)
    {
        cassert(params->text == NULL);
        if (button_type(button->flags) == ekBTFLATGLE && params->state == ekON && button->talt != NULL)
            ((EvButton*)params)->text = tc(button->talt);
        else
            ((EvButton*)params)->text = tc(button->text);

        listener_pass_event(sender->OnClick, event, sender, Button);
    }
}

/*---------------------------------------------------------------------------*/

static Button *i_create(const button_flag_t flags, const align_t halign)
{
    const GuiContext *context = gui_context_get_current();
    Button *button = obj_new0(Button);
    void *ositem = context->func_button_create((enum_t)flags);         
    _component_init(&button->component, context, PARAM(type, ekGUI_COMPONENT_BUTTON), &ositem);
    button->flags = flags;
    button->text = str_c("");

    if (button_type(flags) != ekBTFLAT && button_type(flags) != ekBTFLATGLE)
        context->func_button_set_align(button->component.ositem, (enum_t)halign);

    context->func_button_OnClick(button->component.ositem, obj_listener(button, i_OnClick, Button));    
    return button;
}

/*---------------------------------------------------------------------------*/

Button *button_push(void)
{
    return i_create(ekBTPUSH, ekCENTER);
}

/*---------------------------------------------------------------------------*/

Button *button_check(void)
{
    return i_create(ekBTCHECK2, ekLEFT);
}

/*---------------------------------------------------------------------------*/

Button *button_check3(void)
{
    return i_create(ekBTCHECK3, ekLEFT);
}

/*---------------------------------------------------------------------------*/

Button *button_radio(void)
{
    return i_create(ekBTRADIO, ekLEFT);
}

/*---------------------------------------------------------------------------*/

Button *button_flat(void)
{
    return i_create(ekBTFLAT, ekLEFT);
}

/*---------------------------------------------------------------------------*/

Button *button_flatgle(void)
{
    return i_create(ekBTFLATGLE, ekCENTER);
}

/*---------------------------------------------------------------------------*/

//Button *button_header(void);
//Button *button_header(void)
//{
//    return i_create(ekBTHEADER, ekCENTER);
//}

/*---------------------------------------------------------------------------*/

void button_OnClick(Button *button, Listener *listener)
{
    cassert_no_null(button);
    listener_update(&button->OnClick, listener);
}

/*---------------------------------------------------------------------------*/

static void i_update_text(Button *button)
{
    switch (button_type(button->flags)) {
    case ekBTPUSH:
    case ekBTHEADER:
    case ekBTCHECK2:
    case ekBTCHECK3:
    case ekBTRADIO:
        button->component.context->func_button_set_text(button->component.ositem, tc(button->text));
        break;
    case ekBTFLAT:
    case ekBTFLATGLE:
        button->component.context->func_set_tooltip[ekGUI_COMPONENT_BUTTON](button->component.ositem, tc(button->text));
        break;
    cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void button_text(Button *button, const char_t *text)
{
    const char_t *ltext;
    cassert_no_null(button);
    ltext = _gui_respack_text(text, &button->textid);
    str_upd(&button->text, ltext);
    i_update_text(button);
}

/*---------------------------------------------------------------------------*/

void button_text_alt(Button *button, const char_t *text)
{
    const char_t *ltext;
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTFLATGLE);
    ltext = _gui_respack_text(text, &button->taltid);
    str_upd(&button->talt, ltext);
}

/*---------------------------------------------------------------------------*/

void button_tooltip(Button *button, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(button);
    if (text != NULL)
        ltext = _gui_respack_text(text, &button->ttipid);
    button->component.context->func_set_tooltip[ekGUI_COMPONENT_BUTTON](button->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void button_font(Button *button, const Font *font)
{
    cassert_no_null(button);
    button->component.context->func_button_set_font(button->component.ositem, font);
}

/*---------------------------------------------------------------------------*/

void button_image(Button *button, const Image *image)
{
    const Image *limage = _gui_respack_image((const ResId)image, NULL);
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTPUSH || button_type(button->flags) == ekBTFLAT || button_type(button->flags) == ekBTFLATGLE);
    ptr_destopt(image_destroy, &button->image, Image);
    button->image = ptr_copyopt(image_copy, limage, Image);
    button->component.context->func_button_set_image(button->component.ositem, limage);
}

/*---------------------------------------------------------------------------*/

void button_image_alt(Button *button, const Image *image)
{
    const Image *limage = _gui_respack_image((const ResId)image, NULL);
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTFLATGLE);
    ptr_destopt(image_destroy, &button->imalt, Image);
    button->imalt = ptr_copyopt(image_copy, limage, Image);
}

/*---------------------------------------------------------------------------*/

void button_state(Button *button, const state_t state)
{
    cassert_no_null(button);
    if (button_type(button->flags) != ekBTRADIO)
    {
        button->component.context->func_button_set_state(button->component.ositem, (enum_t)state);
    }
    else
    {
        if (state == ekON)
        {
            button->component.context->func_button_set_state(button->component.ositem, (enum_t)state);
            if (button->component.parent)
                _cell_set_radio(button->component.parent);
        }
    }
}

/*---------------------------------------------------------------------------*/

state_t button_get_state(const Button *button)
{
    cassert_no_null(button);
    return (state_t)button->component.context->func_button_get_state(button->component.ositem);
}

/*---------------------------------------------------------------------------*/

void _button_dimension(Button *button, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(button);
    cassert_no_null(dim0);
    cassert_no_null(dim1);
    if (i == 0)
    {
        if (button_type(button->flags) != ekBTFLAT
            && button_type(button->flags) != ekBTFLATGLE)
        {
            real32_t width = -1, height = -1;
            if (button->image != NULL)
            {
                width = (real32_t)image_width(button->image);
                height = (real32_t)image_height(button->image);
            }

            button->component.context->func_button_bounds(button->component.ositem, tc(button->text), width, height, &button->size.width, &button->size.height);
        }
        else 
        {
            real32_t width = (real32_t)image_width(button->image);
            real32_t height = (real32_t)image_height(button->image);
            button->component.context->func_button_bounds(button->component.ositem, NULL, width, height, &button->size.width, &button->size.height);
        }

        *dim0 = button->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = button->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _button_locale(Button *button)
{
    cassert_no_null(button);
    if (button->textid != NULL)
    {
        const char_t *text = _gui_respack_text(button->textid, NULL);
        str_upd(&button->text, text);
    }

    if (button->taltid != NULL)
    {
        const char_t *text = _gui_respack_text(button->taltid, NULL);
        str_upd(&button->talt, text);
    }

    i_update_text(button);

    if (button->ttipid != NULL)
    {
        const char_t *text = _gui_respack_text(button->ttipid, NULL);
        button->component.context->func_set_tooltip[ekGUI_COMPONENT_BUTTON](button->component.ositem, text);
    }
}

/*---------------------------------------------------------------------------*/

button_flag_t _button_flags(const Button *button)
{
    cassert_no_null(button);
    return button->flags;
}

/*---------------------------------------------------------------------------*/

bool_t _button_is_pushbutton(const Button *button)
{
    cassert_no_null(button);
    return (bool_t)(button_type(button->flags) == ekBTPUSH);
}

/*---------------------------------------------------------------------------*/

bool_t _button_is_radio(const Button *button)
{
    cassert_no_null(button);
    return (bool_t)(button_type(button->flags) == ekBTRADIO);
}

/*---------------------------------------------------------------------------*/

Listener *_button_radio_listener(const Button *button)
{
    cassert_no_null(button);
    if (button_type(button->flags) == ekBTRADIO)
        return button->OnClick;
    return NULL;
}

/*---------------------------------------------------------------------------*/

void _button_radio_state(Button *button, const state_t state)
{
    cassert_no_null(button);
    cassert(button_type(button->flags) == ekBTRADIO);
    button->component.context->func_button_set_state(button->component.ositem, (enum_t)state);
}

/*---------------------------------------------------------------------------*/

void _button_bool(Button *button, const bool_t value)
{
    _button_uint32(button, (uint32_t)value);
}

/*---------------------------------------------------------------------------*/

void _button_uint32(Button *button, const uint32_t value)
{
    cassert_no_null(button);
    switch (button_type(button->flags)) {
    case ekBTRADIO:
        _cell_set_radio_index(button->component.parent, value);
        break;

    case ekBTFLATGLE:
    case ekBTCHECK2:
        button_state(button, value == 0 ? ekOFF : ekON);
        break;

    case ekBTCHECK3:
    {
        state_t st = ekMIXED;
        if (value == 0)
            st = ekOFF;
        else if (value == 1)
            st = ekON;
        button_state(button, st);
        break;
    }

    cassert_default();
    }
}
