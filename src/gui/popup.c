/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: popup.c
 *
 */

/* Pop-up button */

#include "popup.h"
#include "popup.inl"
#include "cell.inl"
#include "component.inl"
#include "dbind.inl"
#include "gui.inl"
#include "guicontexth.inl"
#include "obj.inl"

#include "arrpt.h"
#include "arrst.h"
#include "cassert.h"
#include "event.h"
#include "font.h"
#include "image.h"
#include "ptr.h"
#include "v2d.h"
#include "s2d.h"
#include "strings.h"

typedef struct _pelem_t PElem;

struct _pelem_t
{
    ResId resid;
    String *text;
    Image *image;
};

struct _popup_t
{
    GuiComponent component;
    dtype_t dtype;
    S2Df size;
    ResId ttipid;
    ArrSt(PElem) *elems;
    Listener *OnChange;
};

/*---------------------------------------------------------------------------*/

static void i_remove_elem(PElem *elem)
{
    cassert_no_null(elem);
    str_destroy(&elem->text);
    ptr_destopt(image_destroy, &elem->image, Image);
}

/*---------------------------------------------------------------------------*/

void _popup_destroy(PopUp **popup)
{
    cassert_no_null(popup);
    cassert_no_null(*popup);
    _component_destroy_imp(&(*popup)->component);
    listener_destroy(&(*popup)->OnChange);
    arrst_destroy(&(*popup)->elems, i_remove_elem, PElem);
    obj_delete(popup, PopUp);
}

/*---------------------------------------------------------------------------*/

static void i_OnSelectionChange(PopUp *popup, Event *event)
{
    const EvButton *params = event_params(event, EvButton);
    cassert_no_null(popup);

    switch (popup->dtype) {
    case ekDTYPE_UNKNOWN:
        break;

    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
        _cell_upd_uint32(popup->component.parent, params->index);
        break;

    case ekDTYPE_ENUM:
        _cell_upd_enum_index(popup->component.parent, params->index);
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_REAL32:
    case ekDTYPE_REAL64:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    cassert_default();
    }

    if (popup->OnChange != NULL)
    {
        const PElem *elem = arrst_get(popup->elems, params->index, PElem);
        cassert(params->text == NULL);
        ((EvButton*)params)->text = tc(elem->text);
        listener_pass_event(popup->OnChange, event, popup, PopUp);
    }
}

/*---------------------------------------------------------------------------*/

PopUp *popup_create(void)
{
    const GuiContext *context = gui_context_get_current();
    Font *font = _gui_create_default_font();
    PopUp *popup = obj_new0(PopUp);
    void *ositem = context->func_popup_create((const enum_t)ekPUFLAG);
    context->func_popup_set_font(ositem, font);
    _component_init(&popup->component, context, PARAM(type, ekGUI_COMPONENT_POPUP), &ositem);
    popup->dtype = ekDTYPE_UNKNOWN;
    popup->elems = arrst_create(PElem);
    context->func_popup_OnChange(popup->component.ositem, obj_listener(popup, i_OnSelectionChange, PopUp));
    font_destroy(&font);
    return popup;
}

/*---------------------------------------------------------------------------*/

void popup_OnSelect(PopUp *popup, Listener *listener)
{
    cassert_no_null(popup);
    listener_update(&popup->OnChange, listener);
}

/*---------------------------------------------------------------------------*/

void popup_tooltip(PopUp *popup, const char_t *text)
{
    const char_t *ltext = NULL;
    cassert_no_null(popup);
    if (text != NULL)
        ltext = _gui_respack_text(text, &popup->ttipid);
    popup->component.context->func_set_tooltip[ekGUI_COMPONENT_POPUP](popup->component.ositem, ltext);
}

/*---------------------------------------------------------------------------*/

void popup_add_elem(PopUp *popup, const char_t *text, const Image *image)
{
    const char_t *ltext;
    const Image *limage;
    PElem *elem = NULL;
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_elem);
    cassert_no_nullf(popup->component.context->func_popup_list_height);
    elem = arrst_new(popup->elems, PElem);
    ltext = _gui_respack_text(text, &elem->resid);
    limage = _gui_respack_image((const ResId)image, NULL);
    elem->image = limage ? image_copy(limage) : NULL;
    elem->text = str_c(ltext);
    popup->component.context->func_popup_set_elem(popup->component.ositem, ekOPADD, UINT32_MAX, tc(elem->text), elem->image);
    popup->component.context->func_popup_list_height(popup->component.ositem, arrst_size(popup->elems, PElem));
}

/*---------------------------------------------------------------------------*/

void popup_set_elem(PopUp *popup, const uint32_t index, const char_t *text, const Image *image)
{
    PElem *elem = NULL;
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_elem);
    elem = arrst_get(popup->elems, index, PElem);
    str_upd(&elem->text, text);
    popup->component.context->func_popup_set_elem(popup->component.ositem, ekOPSET, index, text, image);
}

/*---------------------------------------------------------------------------*/

void popup_clear(PopUp *popup)
{
    register uint32_t i, n;
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_elem);
    n = arrst_size(popup->elems, PElem);
    arrst_clear(popup->elems, i_remove_elem, PElem);
    for (i = 0; i < n; ++i)
        popup->component.context->func_popup_set_elem(popup->component.ositem, ekOPDEL, 0, NULL, NULL);
}

/*---------------------------------------------------------------------------*/

uint32_t popup_count(const PopUp *popup)
{
    cassert_no_null(popup);
    return arrst_size(popup->elems, PElem);
}

/*---------------------------------------------------------------------------*/

void popup_list_height(PopUp *popup, const uint32_t num_elems)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_list_height);
    popup->component.context->func_popup_list_height(popup->component.ositem, num_elems);
}

/*---------------------------------------------------------------------------*/

/*
void popup_set_font(PopUp *popup, const Font *font);
void popup_set_font(PopUp *popup, const Font *font)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_font);
    popup->component.context->func_popup_set_font(popup->component.ositem, font, _gui_font_family(font));
}*/

/*---------------------------------------------------------------------------*/

void popup_selected(PopUp *popup, const uint32_t index)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_selected);
    popup->component.context->func_popup_set_selected(popup->component.ositem, index);
}

/*---------------------------------------------------------------------------*/

uint32_t _popup_size(const PopUp *popup)
{
    cassert_no_null(popup);
    return arrst_size(popup->elems, PElem);
}

/*---------------------------------------------------------------------------*/

void _popup_set_dtype(PopUp *popup, const dtype_t dtype)
{
    cassert_no_null(popup);
    cassert(popup->dtype == ekDTYPE_UNKNOWN);
    popup->dtype = dtype;
}

/*---------------------------------------------------------------------------*/

void _popup_enum(PopUp *popup, const DBind *dbind, const enum_t value)
{
    uint32_t index = _dbind_enum_index(dbind, value);
    _popup_uint32(popup, index);
}

/*---------------------------------------------------------------------------*/

void _popup_uint32(PopUp *popup, const uint32_t value)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_selected);
    popup->component.context->func_popup_set_selected(popup->component.ositem, value);
}

/*---------------------------------------------------------------------------*/

void _popup_add_enum_item(PopUp *popup, const char_t *text)
{
    PElem *elem = NULL;
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_set_elem);
    elem = arrst_new(popup->elems, PElem);
    elem->text = str_c(text);
    elem->image = NULL;
    popup->component.context->func_popup_set_elem(popup->component.ositem, ekOPADD, UINT32_MAX, tc(elem->text), elem->image);
}

/*---------------------------------------------------------------------------*/

uint32_t popup_get_selected(const PopUp *popup)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_get_selected);
    return popup->component.context->func_popup_get_selected(popup->component.ositem);
}

/*---------------------------------------------------------------------------*/

void _popup_dimension(PopUp *popup, const uint32_t i, real32_t *dim0, real32_t *dim1)
{
    cassert_no_null(popup);
    cassert_no_null(popup->component.context);
    cassert_no_nullf(popup->component.context->func_popup_bounds);
    cassert_no_null(dim0);
    cassert_no_null(dim1);

    if (i == 0)
    {
        popup->size = kS2D_ZEROf;

        if (arrst_size(popup->elems, PElem) > 0)
        {
            arrst_foreach(elem, popup->elems, PElem)
                real32_t width, height;
                popup->component.context->func_popup_bounds(popup->component.ositem, tc(elem->text), &width, &height);
                if (width > popup->size.width)
                    popup->size.width = width;
                if (height > popup->size.height)
                    popup->size.height = height;
            arrst_end();
        }
        else
        {
            popup->component.context->func_popup_bounds(popup->component.ositem, "    ", &popup->size.width, &popup->size.height);
        }
        
        *dim0 = popup->size.width;
    }
    else
    {
        cassert(i == 1);
        *dim1 = popup->size.height;
    }
}

/*---------------------------------------------------------------------------*/

void _popup_locale(PopUp *popup)
{
    cassert_no_null(popup);
    arrst_foreach(elem, popup->elems, PElem)
        const char_t *text = _gui_respack_text(elem->resid, NULL);
        str_upd(&elem->text, text);
        popup->component.context->func_popup_set_elem(popup->component.ositem, ekOPSET, elem_i, text, elem->image);
    arrst_end();
}
