/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: gbind.c
 *
 */

/* Gui data binding */

#include "gbind.inl"
#include "dbind.inl"
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

#include "arrst.h"
#include "bmem.h"
#include "bstd.h"
#include "cassert.h"
#include "event.h"
#include "strings.h"

/*---------------------------------------------------------------------------*/

static void i_set_bool(GuiComponent *component, const bool_t value)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_BUTTON:
        _button_bool((Button*)component, value);
        break;

    case ekGUI_COMPONENT_LABEL:
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_EDITBOX:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_CUSTOMVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        cassert_msg(FALSE, "Not implemented");
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_enum(GuiComponent *component, const DBind *dbind, const enum_t value)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_POPUP:
        _popup_enum((PopUp*)component, dbind, value);
        break;

    case ekGUI_COMPONENT_BUTTON:
        _button_enum((Button*)component, dbind, value);
        break;

    case ekGUI_COMPONENT_LABEL:
    case ekGUI_COMPONENT_CUSTOMVIEW:
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_EDITBOX:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        cassert_msg(FALSE, "Not implemented");
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_integer(GuiComponent *component, const DBind *dbind, const int64_t value)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_LABEL:
    {
        char_t msg[64];
        bstd_sprintf(msg, 64, "%" PRIu64, value);
        _label_text((Label*)component, msg);
        break;
    }

    case ekGUI_COMPONENT_BUTTON:
        _button_uint32((Button*)component, (uint32_t)value);
        break;

    case ekGUI_COMPONENT_POPUP:
        _popup_uint32((PopUp*)component, (uint32_t)value);
        break;

    case ekGUI_COMPONENT_CUSTOMVIEW:
        _view_uint32((View*)component, (uint32_t)value);
        break;

    case ekGUI_COMPONENT_SLIDER:
    {
        int64_t v;
        int64_t min, max;
        v = _dbind_int(dbind, value);
        _dbind_int_range(dbind, &min, &max);
        cassert(v >= min);
        _slider_uint32((Slider*)component, (uint32_t)(v - min));
        break;
    }

    case ekGUI_COMPONENT_EDITBOX:
    {
        int64_t v = _dbind_int(dbind, value);
        char_t msg[64];
        bstd_sprintf(msg, 64, "%" PRIu64, v);
        _edit_text((Edit*)component, msg);
        break;
    }

    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        cassert_msg(FALSE, "Not implemented");
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_real(GuiComponent *component, const real64_t value, const real64_t min, const real64_t max, const char_t *format)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_LABEL:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            char_t msg[64];
            bstd_sprintf(msg, 64, format, value);
            _label_text((Label*)component, msg);
        }
        else
        {
            _label_text((Label*)component, "");
        }
        break;
    }

    case ekGUI_COMPONENT_EDITBOX:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            char_t msg[64];
            bstd_sprintf(msg, 64, format, value);
            _edit_text((Edit*)component, msg);
        }
        else
        {
            _edit_text((Edit*)component, "");
        }
        break;
    }

    case ekGUI_COMPONENT_SLIDER:
    {
        if (value != REAL32_MAX && value != REAL64_MAX)
        {
            real64_t norm = (value - min) / (max - min);
            _slider_real32((Slider*)component, (real32_t)norm);
        }
        else
        {
            _slider_real32((Slider*)component, .5f);
        }
        break;
    }
    
    case ekGUI_COMPONENT_BUTTON:
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_CUSTOMVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        break;
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_string(GuiComponent *component, const char_t *str)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_LABEL:
        _label_text((Label*)component, str);
        break;

    case ekGUI_COMPONENT_EDITBOX:
        _edit_text((Edit*)component, str);
        break;

    case ekGUI_COMPONENT_BUTTON:
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_CUSTOMVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        cassert_msg(FALSE, "Not implemented");
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_image(GuiComponent *component, const Image *image)
{
    cassert_no_null(component);
    switch (component->type) {
    case ekGUI_COMPONENT_CUSTOMVIEW:
        _view_image((View*)component, image);
        break;

    case ekGUI_COMPONENT_LABEL:
    case ekGUI_COMPONENT_BUTTON:
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_EDITBOX:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        cassert_msg(FALSE, "Not implemented");
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_empty(Cell *cell, DBind *dbind)
{
    GuiComponent *component = _cell_component(cell);
    switch (component->type) {
    case ekGUI_COMPONENT_EDITBOX:
        _edit_text((Edit*)component, "");
        break;

    case ekGUI_COMPONENT_CUSTOMVIEW:
        _view_empty((View*)component);
        break;

    case ekGUI_COMPONENT_LABEL:
    case ekGUI_COMPONENT_BUTTON:
    case ekGUI_COMPONENT_POPUP:
    case ekGUI_COMPONENT_COMBOBOX:
    case ekGUI_COMPONENT_SLIDER:
    case ekGUI_COMPONENT_UPDOWN:
    case ekGUI_COMPONENT_PROGRESS:
    case ekGUI_COMPONENT_TEXTVIEW:
    case ekGUI_COMPONENT_TABLEVIEW:
    case ekGUI_COMPONENT_TREEVIEW:
	case ekGUI_COMPONENT_BOXVIEW:
    case ekGUI_COMPONENT_SPLITVIEW:
    case ekGUI_COMPONENT_PANEL:
    case ekGUI_COMPONENT_LINE:
    case ekGUI_COMPONENT_HEADER:
    case ekGUI_COMPONENT_WINDOW:
    case ekGUI_COMPONENT_TOOLBAR:
    default:
        break;
    }

    switch (_dbind_member_type(dbind)) {
    case ekDTYPE_OBJECT_OPAQUE:
        if (str_equ_c(_dbind_member_subtype(dbind), "Image") == TRUE)
        {
            const Image *image = (const Image*)_dbind_default_opaque(dbind);
            i_set_image(component, image);
        }
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    case ekDTYPE_REAL32:
    case ekDTYPE_REAL64:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    cell_enabled(cell, FALSE);
}

/*---------------------------------------------------------------------------*/

static void i_set_empty_cells(ArrSt(Cell) *cells, DBind *dbind)
{
    register uint32_t i, n = arrst_size(cells, Cell);
    for (i = 0; i < n; ++i)
    {
        Cell *cell = arrst_get(cells, i, Cell);
        i_set_empty(cell, dbind);
    }
}

/*---------------------------------------------------------------------------*/

void gbind_set_component(Cell *cell, StBind *stbind, DBind *dbind, void *obj)
{
    cassert_unref(_dbind_get_stbind(dbind) == stbind, stbind);
    if (obj != NULL)
    {
        dtype_t mtype;
        uint16_t offset;
        uint16_t msize;
        const char_t *subtype;
        void *mdata;
        GuiComponent *component = _cell_component(cell);
        cassert(component->type != ekGUI_COMPONENT_PANEL);
        _dbind_member_get(dbind, &offset, &mtype, &msize, &subtype);
        mdata = (void*)((byte_t*)obj + offset);

        switch (mtype) {

        case ekDTYPE_BOOL:
            cassert_unref(msize == sizeof(bool_t), msize);
            i_set_bool(component, *((bool_t*)mdata));
            break;

        case ekDTYPE_ENUM:
            cassert_unref(msize == sizeof(enum_t), msize);
            i_set_enum(component, dbind, *((enum_t*)mdata));
            break;
        
        case ekDTYPE_INT8:
            cassert_unref(msize == sizeof(int8_t), msize);
            i_set_integer(component, dbind, (int64_t)*((int8_t*)mdata));
            break;

        case ekDTYPE_INT16:
            cassert_unref(msize == sizeof(int16_t), msize);
            i_set_integer(component, dbind, (int64_t)*((int16_t*)mdata));
            break;

        case ekDTYPE_INT32:
            cassert_unref(msize == sizeof(int32_t), msize);
            i_set_integer(component, dbind, (int64_t)*((int32_t*)mdata));
            break;

        case ekDTYPE_INT64:
            cassert_unref(msize == sizeof(int64_t), msize);
            i_set_integer(component, dbind, (int64_t)*((int64_t*)mdata));
            break;

        case ekDTYPE_UINT8:
            cassert_unref(msize == sizeof(uint8_t), msize);
            i_set_integer(component, dbind, (int64_t)*((uint8_t*)mdata));
            break;

        case ekDTYPE_UINT16:
            cassert_unref(msize == sizeof(uint16_t), msize);
            i_set_integer(component, dbind, (int64_t)*((uint16_t*)mdata));
            break;

        case ekDTYPE_UINT32:
            cassert_unref(msize == sizeof(uint32_t), msize);
            i_set_integer(component, dbind, (int64_t)*((uint32_t*)mdata));
            break;

        case ekDTYPE_UINT64:
            cassert_unref(msize == sizeof(uint64_t), msize);
            i_set_integer(component, dbind, (int64_t)*((uint64_t*)mdata));
            break;

        case ekDTYPE_REAL32:
        {
            real32_t min, max;
            real32_t *v = (real32_t*)mdata;
            cassert_unref(msize == sizeof(real32_t), msize);
            _dbind_real32_range(dbind, &min, &max);
            *v = _dbind_real32(dbind, *v);
            i_set_real(component, (real64_t)*v, (real64_t)min, (real64_t)max, _dbind_real32_format(dbind));
            break;
        }

        case ekDTYPE_REAL64:
        {
            real64_t min, max;
            real64_t *v = (real64_t*)mdata;
            cassert_unref(msize == sizeof(real64_t), msize);
            _dbind_real64_range(dbind, &min, &max);
            *v = _dbind_real64(dbind, *v);
            i_set_real(component, *v, min, max, _dbind_real64_format(dbind));
            break;
        }

        case ekDTYPE_STRING_PTR:
            cassert_unref(msize == sizeof(String*), msize);
            i_set_string(component, tc(*((String**)mdata)));
            break;

        case ekDTYPE_OBJECT_OPAQUE:
            if (str_equ_c(subtype, "Image") == TRUE)
                i_set_image(component, *((Image**)mdata));
            break;

        case ekDTYPE_STRING:
        case ekDTYPE_ARRAY:
        case ekDTYPE_ARRPTR:
        case ekDTYPE_OBJECT:
        case ekDTYPE_OBJECT_PTR:
        case ekDTYPE_UNKNOWN:
        cassert_default();
        }

        cell_enabled(cell, TRUE);
    }
    else
    {
        i_set_empty(cell, dbind);
    }
}

/*---------------------------------------------------------------------------*/

void gbind_set_layout(Layout *layout, StBind *stbind, DBind *dbind, void *obj)
{
    cassert_unref(_dbind_get_stbind(dbind) == stbind, stbind);
    if (obj != NULL)
    {
        dtype_t mtype;
        uint16_t offset;
        uint16_t msize;
        const char_t *subtype;
        void *mdata;
        _dbind_member_get(dbind, &offset, &mtype, &msize, &subtype);
        mdata = (void*)((byte_t*)obj + offset);

        switch (mtype) {

        case ekDTYPE_OBJECT:
            layout_dbind_obj_imp(layout, mdata, subtype);
            break;

        case ekDTYPE_OBJECT_PTR:
            layout_dbind_obj_imp(layout, *((void**)mdata), subtype);
            break;

        case ekDTYPE_BOOL:
        case ekDTYPE_ENUM:
        case ekDTYPE_INT8:
        case ekDTYPE_INT16:
        case ekDTYPE_INT32:
        case ekDTYPE_INT64:
        case ekDTYPE_UINT8:
        case ekDTYPE_UINT16:
        case ekDTYPE_UINT32:
        case ekDTYPE_UINT64:
        case ekDTYPE_REAL32:
        case ekDTYPE_REAL64:
        {
            ArrSt(Cell) *cells = _layout_cells(layout);
            uint32_t i, n = arrst_size(cells, Cell);
            for (i = 0; i < n; ++i)
            {
                Cell *cell = arrst_get(cells, i, Cell);
                gbind_set_component(cell, stbind, dbind, obj);
            }
            break;
        }

        case ekDTYPE_STRING:
        case ekDTYPE_STRING_PTR:
        case ekDTYPE_ARRAY:
        case ekDTYPE_ARRPTR:
        case ekDTYPE_UNKNOWN:
        case ekDTYPE_OBJECT_OPAQUE:
        cassert_default();
        }
    }
    else
    {
        ArrSt(Cell) *cells = _layout_cells(layout);
        i_set_empty_cells(cells, dbind);
    }
}

/*---------------------------------------------------------------------------*/

static __INLINE bool_t i_valid(void *obj, const char_t *objtype, const uint16_t moffset, Listener *listener)
{
    EvBind params;
    bool_t ok = TRUE;
    params.offset = moffset;
    listener_event_imp(listener, ekEVOBJVALIDATE, obj, &params, &ok, objtype, "EvBind", "bool_t");
    return ok;
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_on_change(void *obj, const char_t *objtype, const uint16_t moffset, Listener *listener)
{
    EvBind params;
    params.offset = moffset;
    listener_event_imp(listener, ekEVOBJCHANGE, obj, &params, NULL, objtype, "EvBind", "void");
}

/*---------------------------------------------------------------------------*/

static void i_upd_value(byte_t *data, const byte_t *value, const uint32_t n, void *obj, const char_t *objtype, const uint16_t moffset, Listener *listener)
{
    cassert(n <= 16);
    if (listener != NULL)
    {
        if (bmem_cmp(data, value, n) != 0)
        {
            byte_t cache[16];
            bmem_copy(cache, data, n);
            bmem_copy(data, value, n);

            if (i_valid(obj, objtype, moffset, listener) == TRUE)
                i_on_change(obj, objtype, moffset, listener);
            else
                bmem_copy(data, cache, n);
        }
    }
    else
    {
        bmem_copy(data, value, n);
    }
}

/*---------------------------------------------------------------------------*/

static void i_upd_str(String **data, const char_t *value, void *obj, const char_t *objtype, const uint16_t moffset, Listener *listener)
{
    if (str_equ(*data, value) == FALSE)
    {
        if (listener != NULL)
        {
            String *cache = str_copy(*data);
            str_upd(data, value);
            if (i_valid(obj, objtype, moffset, listener) == TRUE)
                i_on_change(obj, objtype, moffset, listener);
            else
                str_upd(data, tc(cache));

            str_destroy(&cache);
        }
        else
        {
            str_upd(data, value);
        }
    }
}

/*---------------------------------------------------------------------------*/

void gbind_upd_bool(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, bool_t value)
{
    uint16_t offset = _dbind_member_offset(dbind);
    byte_t *mdata = (byte_t*)(obj) + offset;
    dtype_t mtype = _dbind_member_type(dbind);
    const char_t *stname = st_notif ? _dbind_stbind_type(st_notif) : "";

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_BOOL:
        i_upd_value(mdata, (byte_t*)&value, sizeof(bool_t), obj_notif, stname, offset, listener);
        break;

    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    case ekDTYPE_REAL32:
    case ekDTYPE_REAL64:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        cassert_msg(FALSE, "Not implemented");
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_enum(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, enum_t value)
{
    uint16_t offset = _dbind_member_offset(dbind);
    byte_t *mdata = (byte_t*)(obj) + offset;
    dtype_t mtype = _dbind_member_type(dbind);
    const char_t *stname = st_notif ? _dbind_stbind_type(st_notif) : "";

    /* Update the object data */
    if (mtype == ekDTYPE_ENUM) 
    {
        i_upd_value(mdata, (byte_t*)&value, sizeof(enum_t), obj_notif, stname, offset, listener);
    }
    else
    {
        cassert_msg(FALSE, "Not implemented");
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_uint32(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, uint32_t value)
{
    uint16_t offset = _dbind_member_offset(dbind);
    byte_t *mdata = (byte_t*)(obj) + offset;
    dtype_t mtype = _dbind_member_type(dbind);
    int64_t dvalue = _dbind_int(dbind, value);
    const char_t *stname = st_notif ? _dbind_stbind_type(st_notif) : "";

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_INT8:
    {
        int8_t v = (int8_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(int8_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT16:
    {
        int16_t v = (int16_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(int16_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT32:
    {
        int32_t v = (int32_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(int32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT64:
    {
        int64_t v = (int64_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(int64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT8:
    {
        uint8_t v = (uint8_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(uint8_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT16:
    {
        uint16_t v = (uint16_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(uint16_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT32:
    {
        uint32_t v = (uint32_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(uint32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT64:
    {
        uint64_t v = (uint64_t)dvalue;
        i_upd_value(mdata, (byte_t*)&v, sizeof(uint64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_REAL32:
    {
        real32_t v = (real32_t)value;
        i_upd_value(mdata, (byte_t*)&v, sizeof(real32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_REAL64:
    {
        real64_t v = (real64_t)value;
        i_upd_value(mdata, (byte_t*)&v, sizeof(real64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_ENUM:
    {
        enum_t v = (enum_t)value;
        // The enum value in index 'value'? 
        cassert(FALSE);
        i_upd_value(mdata, (byte_t*)&v, sizeof(enum_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_STRING_PTR:
    {
        char_t text[64];
        bstd_sprintf(text, sizeof(text), "%u", value);
        i_upd_str((String**)mdata, text, obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_BOOL:
    case ekDTYPE_STRING:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        cassert_msg(FALSE, "Not implemented");
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_norm_real32(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, const real32_t value)
{
    uint16_t offset = _dbind_member_offset(dbind);
    byte_t *mdata = (byte_t*)(obj) + offset;
    dtype_t mtype = _dbind_member_type(dbind);
    const char_t *stname = st_notif ? _dbind_stbind_type(st_notif) : "";

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_REAL32:
    {
        real32_t min, max, v;
        _dbind_real32_range(dbind, &min, &max);
        v = min + (max - min) * value;
        v = _dbind_real32(dbind, v);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(real32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_REAL64:
    {
        real64_t min, max, v;
        _dbind_real64_range(dbind, &min, &max);
        v = min + (max - min) * value;
        v = _dbind_real64(dbind, v);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(real64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    {
        int64_t min, max, v;
        _dbind_int_range(dbind, &min, &max);
        v = (int64_t)((real32_t)min + ((real32_t)max - (real32_t)min) * value);
        v = _dbind_int(dbind, v);

        if (mtype == ekDTYPE_INT8)
        {
            int8_t vv = (int8_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(int8_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_INT16)
        {
            int16_t vv = (int16_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(int16_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_INT32)
        {
            int32_t vv = (int32_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(int32_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_INT64)
        {
            int64_t vv = (int64_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(int64_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_UINT8)
        {
            uint8_t vv = (uint8_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(uint8_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_UINT16)
        {
            uint16_t vv = (uint16_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(uint16_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_UINT32)
        {
            uint32_t vv = (uint32_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(uint32_t), obj_notif, stname, offset, listener);
        }
        else if (mtype == ekDTYPE_UINT64)
        {
            uint64_t vv = (uint64_t)v;
            i_upd_value(mdata, (const byte_t*)&vv, sizeof(uint64_t), obj_notif, stname, offset, listener);
        }
        break;
    }

    case ekDTYPE_BOOL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        cassert_msg(FALSE, "Not implemented");
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_string(Layout *layout, DBind *dbind, void *obj, void *obj_notif, StBind *st_notif, Listener *listener, const char_t *str)
{
    uint16_t offset = _dbind_member_offset(dbind);
    byte_t *mdata = (byte_t*)(obj) + offset;
    dtype_t mtype = _dbind_member_type(dbind);
    const char_t *stname = st_notif ? _dbind_stbind_type(st_notif) : "";

    /* Update the object data */
    switch (mtype) {

    case ekDTYPE_STRING_PTR:
        i_upd_str((String**)mdata, str, obj_notif, stname, offset, listener);
        break;

    case ekDTYPE_REAL32:
    {
        real32_t v = _dbind_string_to_real32(dbind, *((real32_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(real32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_REAL64:
    {
        real64_t v = _dbind_string_to_real64(dbind, *((real64_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(real64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT8:
    {
        int8_t v = _dbind_string_to_int8(dbind, *((int8_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(int8_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT16:
    {
        int16_t v = _dbind_string_to_int16(dbind, *((int16_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(int16_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT32:
    {
        int32_t v = _dbind_string_to_int32(dbind, *((int32_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(int32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_INT64:
    {
        int64_t v = _dbind_string_to_int64(dbind, *((int64_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(int64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT8:
    {
        uint8_t v = _dbind_string_to_uint8(dbind, *((uint8_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(uint8_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT16:
    {
        uint16_t v = _dbind_string_to_uint16(dbind, *((uint16_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(uint16_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT32:
    {
        uint32_t v = _dbind_string_to_uint32(dbind, *((uint32_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(uint32_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_UINT64:
    {
        uint64_t v = _dbind_string_to_uint64(dbind, *((uint64_t*)mdata), str);
        i_upd_value(mdata, (const byte_t*)&v, sizeof(uint64_t), obj_notif, stname, offset, listener);
        break;
    }

    case ekDTYPE_BOOL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_image(Layout *layout, DBind *dbind, void *objbind, const Image *image)
{
    byte_t *mdata = (byte_t*)(objbind) + _dbind_member_offset(dbind);
    dtype_t mtype = _dbind_member_type(dbind);

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_OBJECT_OPAQUE:
        cassert(str_equ_c(_dbind_member_subtype(dbind), "Image") == TRUE);
        _dbind_opaque_upd(_dbind_member_subtype(dbind), (void*)image, (void**)mdata);
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    case ekDTYPE_REAL32:
    case ekDTYPE_REAL64:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    /* Update all controls related with this member */
    /* Decoment if more imgview with same image */
    //_layout_dbind_update(layout, dbind);
    unref(layout);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_increment(Layout *layout, DBind *dbind, void *objbind)
{
    byte_t *mdata = (byte_t*)(objbind) + _dbind_member_offset(dbind);
    dtype_t mtype = _dbind_member_type(dbind);

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_REAL32:
        *((real32_t*)mdata) = _dbind_incr_real32(dbind, *((real32_t*)mdata));
        break;

    case ekDTYPE_REAL64:
        *((real64_t*)mdata) = _dbind_incr_real64(dbind, *((real64_t*)mdata));
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}

/*---------------------------------------------------------------------------*/

void gbind_upd_decrement(Layout *layout, DBind *dbind, void *objbind)
{
    byte_t *mdata = (byte_t*)(objbind) + _dbind_member_offset(dbind);
    dtype_t mtype = _dbind_member_type(dbind);

    /* Update the object data */
    switch (mtype) {
    case ekDTYPE_REAL32:
        *((real32_t*)mdata) = _dbind_decr_real32(dbind, *((real32_t*)mdata));
        break;

    case ekDTYPE_REAL64:
        *((real64_t*)mdata) = _dbind_decr_real64(dbind, *((real64_t*)mdata));
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT8:
    case ekDTYPE_INT16:
    case ekDTYPE_INT32:
    case ekDTYPE_INT64:
    case ekDTYPE_UINT8:
    case ekDTYPE_UINT16:
    case ekDTYPE_UINT32:
    case ekDTYPE_UINT64:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRING_PTR:
    case ekDTYPE_ARRAY:
    case ekDTYPE_ARRPTR:
    case ekDTYPE_OBJECT:
    case ekDTYPE_OBJECT_PTR:
    case ekDTYPE_OBJECT_OPAQUE:
    case ekDTYPE_UNKNOWN:
    default:
        break;
    }

    /* Update all controls related with this member */
    _layout_dbind_update(layout, dbind);
}
