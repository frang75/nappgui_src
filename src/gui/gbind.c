/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gbind.c
 *
 */

/* Gui data binding */

#include "gbind.inl"
#include "cell.h"
#include "cell.inl"
#include "component.inl"
#include "button.inl"
#include "view.inl"
#include "edit.inl"
#include "label.inl"
#include "layout.h"
#include "layout.inl"
#include "panel.inl"
#include "popup.inl"
#include "slider.inl"
#include <core/arrpt.h>
#include <core/dbindh.h>
#include <core/event.h>
#include <core/strings.h>
#include <sewer/bmath.h>
#include <sewer/bmem.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>

ArrStFuncs(Cell);

/*---------------------------------------------------------------------------*/

static void i_set_bool(GuiComponent *component, const bool_t value)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_LABEL:
        _label_text(cast(component, Label), value ? "True" : "False");
        break;

    case ekGUI_TYPE_BUTTON:
        _button_bool(cast(component, Button), value);
        break;

    case ekGUI_TYPE_SLIDER:
        _slider_real32(cast(component, Slider), value ? 1.f : 0.f);
        break;

    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_EDITBOX:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_CUSTOMVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_integer(GuiComponent *component, const int64_t value, const int64_t min, const int64_t max)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_LABEL:
    {
        char_t msg[64];
        bstd_sprintf(msg, 64, "%" PRId64, value);
        _label_text(cast(component, Label), msg);
        break;
    }

    case ekGUI_TYPE_BUTTON:
        _button_uint32(cast(component, Button), value >= 0 ? (uint32_t)value : 0);
        break;

    case ekGUI_TYPE_POPUP:
        _popup_uint32(cast(component, PopUp), (uint32_t)value);
        break;

    case ekGUI_TYPE_CUSTOMVIEW:
        _view_uint32(cast(component, View), (uint32_t)value);
        break;

    case ekGUI_TYPE_SLIDER:
    {
        real64_t range = (real64_t)max - (real64_t)min;
        real32_t norm = (real32_t)(((real64_t)value - (real64_t)min) / range);
        cassert(value >= min);
        _slider_real32(cast(component, Slider), norm);
        break;
    }

    case ekGUI_TYPE_EDITBOX:
    {
        char_t msg[64];
        bstd_sprintf(msg, 64, "%" PRId64, value);
        _edit_text(cast(component, Edit), msg);
        break;
    }

    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_real(GuiComponent *component, const real64_t value, const real64_t min, const real64_t max, const char_t *format)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_LABEL:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            char_t msg[64];
            bstd_sprintf(msg, 64, format, value);
            _label_text(cast(component, Label), msg);
        }
        else
        {
            _label_text(cast(component, Label), "");
        }
        break;
    }

    case ekGUI_TYPE_EDITBOX:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            char_t msg[64];
            bstd_sprintf(msg, 64, format, value);
            _edit_text(cast(component, Edit), msg);
        }
        else
        {
            _edit_text(cast(component, Edit), "");
        }
        break;
    }

    case ekGUI_TYPE_SLIDER:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            real64_t norm = bmath_clampd((value - min) / (max - min), 0, 1);
            _slider_real32(cast(component, Slider), (real32_t)norm);
        }
        else
        {
            _slider_real32(cast(component, Slider), .5f);
        }
        break;
    }

    case ekGUI_TYPE_BUTTON:
    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_CUSTOMVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_enum(GuiComponent *component, const uint32_t index, const uint32_t count, const char_t *alias)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_LABEL:
        _label_text(cast(component, Label), alias);
        break;

    case ekGUI_TYPE_BUTTON:
        _button_uint32(cast(component, Button), index);
        break;

    case ekGUI_TYPE_POPUP:
        _popup_uint32(cast(component, PopUp), index);
        break;

    case ekGUI_TYPE_SLIDER:
    {
        real32_t vnorm = 0;
        if (count > 1)
            vnorm = (real32_t)index / (count - 1);
        _slider_real32(cast(component, Slider), vnorm);
        break;
    }

    case ekGUI_TYPE_CUSTOMVIEW:
        _view_uint32(cast(component, View), index);
        break;

    case ekGUI_TYPE_EDITBOX:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_string(GuiComponent *component, const char_t *str)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_LABEL:
        _label_text(cast(component, Label), str);
        break;

    case ekGUI_TYPE_EDITBOX:
        _edit_text(cast(component, Edit), str);
        break;

    case ekGUI_TYPE_BUTTON:
    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_SLIDER:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_CUSTOMVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_image(GuiComponent *component, const Image *image)
{
    cassert_no_null(component);
    switch (component->type)
    {
    case ekGUI_TYPE_CUSTOMVIEW:
        _view_image(cast(component, View), image);
        break;

    case ekGUI_TYPE_LABEL:
    case ekGUI_TYPE_BUTTON:
    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_EDITBOX:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_SLIDER:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_empty(Cell *cell, const DBind *stbind, const uint32_t member_id)
{
    GuiComponent *component = _cell_component(cell);
    const DBind *mbind = dbind_st_member(stbind, member_id);

    switch (component->type)
    {
    case ekGUI_TYPE_EDITBOX:
        _edit_text(cast(component, Edit), "");
        break;

    case ekGUI_TYPE_CUSTOMVIEW:
        _view_empty(cast(component, View));
        break;

    case ekGUI_TYPE_LABEL:
        _label_text(cast(component, Label), "");
        break;

    case ekGUI_TYPE_BUTTON:
    case ekGUI_TYPE_POPUP:
    case ekGUI_TYPE_COMBOBOX:
    case ekGUI_TYPE_SLIDER:
    case ekGUI_TYPE_UPDOWN:
    case ekGUI_TYPE_PROGRESS:
        break;

    case ekGUI_TYPE_TEXTVIEW:
    case ekGUI_TYPE_WEBVIEW:
    case ekGUI_TYPE_TREEVIEW:
    case ekGUI_TYPE_BOXVIEW:
    case ekGUI_TYPE_SPLITVIEW:
    case ekGUI_TYPE_PANEL:
    case ekGUI_TYPE_LINE:
    case ekGUI_TYPE_HEADER:
    case ekGUI_TYPE_WINDOW:
    case ekGUI_TYPE_TOOLBAR:
        cassert_default();
    }

    switch (dbind_type(mbind))
    {
    case ekDTYPE_BINARY:
        if (str_equ_c(dbind_typename(mbind), "Image") == TRUE)
        {
            const Image *image = cast_const(dbind_st_binary_default(stbind, member_id), Image);
            i_set_image(component, image);
        }
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    cell_enabled(cell, FALSE);
}

/*---------------------------------------------------------------------------*/

void _gbind_update_control(Cell *cell, const DBind *stbind, const uint32_t member_id, const void *obj)
{
    if (obj != NULL)
    {
        const DBind *mbind = dbind_st_member(stbind, member_id);
        dtype_t mtype = dbind_type(mbind);
        GuiComponent *component = _cell_component(cell);
        cassert(component->type != ekGUI_TYPE_PANEL);

        switch (mtype)
        {
        case ekDTYPE_BOOL:
        {
            bool_t value = dbind_st_get_bool_value(stbind, member_id, cast_const(obj, byte_t));
            i_set_bool(component, value);
            break;
        }

        case ekDTYPE_INT:
        {
            int64_t min, max, value;
            dbind_st_int_range(stbind, member_id, &min, &max);
            value = dbind_st_get_int_value(stbind, member_id, cast_const(obj, byte_t));
            i_set_integer(component, value, min, max);
            break;
        }

        case ekDTYPE_REAL:
        {
            real64_t min, max, value;
            const char_t *format = NULL;
            dbind_st_real_range(stbind, member_id, &min, &max);
            format = dbind_st_real_format(stbind, member_id);
            value = dbind_st_get_real_value(stbind, member_id, cast_const(obj, byte_t));
            i_set_real(component, value, min, max, format);
            break;
        }

        case ekDTYPE_ENUM:
        {
            enum_t value = dbind_st_get_enum_value(stbind, member_id, obj);
            uint32_t index = dbind_enum_index(mbind, value);
            uint32_t count = dbind_enum_count(mbind);
            const char_t *alias = dbind_enum_alias(mbind, index);
            i_set_enum(component, index, count, alias);
            break;
        }

        case ekDTYPE_STRING:
        {
            const char_t *str = dbind_st_get_str_value(stbind, member_id, obj);
            i_set_string(component, str != NULL ? str : "");
            break;
        }

        case ekDTYPE_BINARY:
        {
            const char_t *typename = dbind_typename(mbind);
            if (str_equ_c(typename, "Image") == TRUE)
            {
                const Image *image = cast_const(dbind_st_get_binary_value(stbind, member_id, obj), Image);
                i_set_image(component, image);
            }
            break;
        }

        case ekDTYPE_STRUCT:
        case ekDTYPE_CONTAINER:
        case ekDTYPE_UNKNOWN:
            cassert_default();
        }

        cell_enabled(cell, TRUE);
    }
    else
    {
        i_set_empty(cell, stbind, member_id);
    }
}

/*---------------------------------------------------------------------------*/

void _gbind_update_layout(Layout *layout, const DBind *stbind, const uint32_t member_id, const void *obj)
{
    if (obj != NULL)
    {
        const DBind *mbind = dbind_st_member(stbind, member_id);
        dtype_t mtype = dbind_type(mbind);

        switch (mtype)
        {
        case ekDTYPE_STRUCT:
        {
            const byte_t *sobj = dbind_st_get_struct_value(stbind, member_id, cast_const(obj, byte_t));
            const char_t *typename = dbind_typename(mbind);
            layout_dbind_obj_imp(layout, cast(sobj, void), typename);
            break;
        }

        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
        case ekDTYPE_STRING:
        case ekDTYPE_BINARY:
        {
            ArrPt(Cell) *cells = _layout_cells(layout);
            arrpt_foreach(cell, cells, Cell)
                _gbind_update_control(cell, stbind, member_id, obj);
            arrpt_end()
            break;
        }

        case ekDTYPE_CONTAINER:
        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    }
    else
    {
        ArrPt(Cell) *cells = _layout_cells(layout);
        arrpt_foreach(cell, cells, Cell)
            i_set_empty(cell, stbind, member_id);
        arrpt_end()
    }
}

/*---------------------------------------------------------------------------*/

static void i_on_updated(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const bindset_t updated, byte_t *store, const uint32_t sizeof_store)
{
    /* The value has changed in 'obj' */
    if (updated == ekBINDSET_OK)
    {
        bool_t allow_change = TRUE;

        if (layout_notify != NULL)
        {
            EvBind params;
            Listener *listener = NULL;
            const DBind *mbind = dbind_st_member(stbind, member_id);
            _layout_dbind_notif_obj(layout_notify, &params.obj_main, &params.objtype_main, &params.size_main, &listener);
            params.obj_edit = obj;
            params.objtype_edit = dbind_typename(stbind);
            params.offset_edit = dbind_st_offset(stbind, member_id);
            params.size_edit = dbind_size(mbind);
            listener_event_imp(listener, ekGUI_EVENT_OBJCHANGE, layout_notify, &params, &allow_change, "Layout", "EvBind", "bool_t");
        }

        /* The updated has been reverted by event processing */
        if (allow_change == FALSE)
        {
            /* Restore the previous field value from a copy */
            dbind_st_restore_field(stbind, member_id, cast(obj, byte_t), store, sizeof_store);
        }
    }

    /* The stored field can have reserved dynamic memory (string, binaries, etc) */
    dbind_st_remove_field(stbind, member_id, store, sizeof_store);

    /* Update all controls related with this member */
    if (updated == ekBINDSET_OK)
        _layout_dbind_update(layout, member_id);
}

/*---------------------------------------------------------------------------*/

static void i_store_field(const DBind *stbind, const uint32_t member_id, void *obj, byte_t *store, const uint32_t sizeof_store)
{
    bmem_set_zero(store, sizeof_store);
    dbind_st_store_field(stbind, member_id, cast(obj, byte_t), store, sizeof_store);
}

/*---------------------------------------------------------------------------*/

void _gbind_update_bool(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const bool_t value)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_bool(stbind, member_id, cast(obj, byte_t), value);
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

void _gbind_update_u32(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const uint32_t value)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_int(stbind, member_id, cast(obj, byte_t), (int64_t)value);
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

void _gbind_update_norm32(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const real32_t value)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_norm(stbind, member_id, cast(obj, byte_t), (real64_t)value);
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

void _gbind_update_incr(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const bool_t positive_incr)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_incr(stbind, member_id, cast(obj, byte_t), positive_incr);
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

void _gbind_update_str(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const char_t *str)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_str(stbind, member_id, cast(obj, byte_t), str);
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

void _gbind_update_image(Layout *layout, const DBind *stbind, const uint32_t member_id, void *obj, Layout *layout_notify, const Image *image)
{
    byte_t store[16];
    bindset_t updated = ekBINDSET_NOT_ALLOWED;
    i_store_field(stbind, member_id, obj, store, sizeof(store));
    updated = dbind_st_set_value_binary(stbind, member_id, cast(obj, byte_t), cast(image, void));
    i_on_updated(layout, stbind, member_id, obj, layout_notify, updated, store, sizeof(store));
}

/*---------------------------------------------------------------------------*/

bool_t _gbind_field_modify(const EvBind *evbind, const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize)
{
    const DBind *stbind = dbind_from_typename(type, NULL);
    uint32_t member_id = dbind_st_member_id(stbind, mname);

#if defined(__ASSERTS__)
    {
        bool_t is_ptr = FALSE;
        const DBind *mbind = dbind_st_member(stbind, member_id);
        const DBind *mbindt = dbind_from_typename(mtype, &is_ptr);
        const char_t *typename = dbind_typename(mbind);
        cassert(mbind == mbindt);
        cassert_unref(dbind_size(stbind) == size, size);
        cassert_unref(dbind_st_offset(stbind, member_id) == moffset, moffset);
        cassert_unref((is_ptr == TRUE && msize == sizeof(void *)) || dbind_size(mbind) == msize, msize);
        if (is_ptr == TRUE)
        {
            uint32_t len = str_len_c(typename);
            cassert(str_equ_cn(typename, mtype, len) == TRUE);
        }
        else
        {
            cassert_unref(str_equ_c(dbind_typename(mbind), mtype) == TRUE, mtype);
        }
        cassert_no_null(evbind);
    }
#else
    unref(size);
    unref(mtype);
    unref(msize);
    unref(moffset);
#endif

    {
        const byte_t *data = NULL;
        cassert_no_null(evbind);
        data = cast(evbind->obj_edit, byte_t) + evbind->offset_edit;
        return dbind_st_overlaps_field(stbind, member_id, evbind->obj_main, data, evbind->size_edit);
    }
}
