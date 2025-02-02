/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dbind.c
 *
 */

/* Data bindings */

#include "dbind.h"
#include "dbindh.h"
#include "dbind.inl"
#include "tfilter.inl"
#include "arrpt.h"
#include "arrst.h"
#include "buffer.h"
#include "heap.h"
#include "stream.h"
#include "strings.h"
#include <osbs/log.h>
#include <sewer/bmath.h>
#include <sewer/bmem.h>
#include <sewer/bstd.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

typedef struct _memberattr_t MemberAttr;
typedef struct _enummember_t EnumMember;
typedef struct _structmember_t StructMember;
typedef struct _boolprops_t BoolProps;
typedef struct _intprops_t IntProps;
typedef struct _realprops_t RealProps;
typedef struct _enumprops_t EnumProps;
typedef struct _stringprops_t StringProps;
typedef struct _structprops_t StructProps;
typedef struct _containerprops_t ContainerProps;
typedef struct _binaryprops_t BinaryProps;
typedef union _dbindprops_t DBindProps;
typedef struct _alias_t Alias;
typedef struct _databind_t DataBind;

struct _memberattr_t
{
    struct _bool_
    {
        bool_t def;
    } boolt;

    struct _int_
    {
        int64_t def;
        int64_t min;
        int64_t max;
        int64_t incr;
        String *format;
    } intt;

    struct _real_
    {
        real64_t def;
        real64_t min;
        real64_t max;
        real64_t prec;
        real64_t incr;
        uint32_t dec;
        String *format;
    } realt;

    struct _enum_
    {
        enum_t def;
    } enumt;

    struct _string_
    {
        void *def;
    } stringt;

    struct _struct_
    {
        byte_t *def;
        bool_t is_pointer;
    } structt;

    struct _binary_
    {
        void *def;
    } binaryt;

    struct _container_
    {
        byte_t *def;
        DBind *bind;
    } containert;
};

struct _enummember_t
{
    String *name;
    String *alias;
    enum_t value;
};

struct _structmember_t
{
    DBind *bind;
    String *name;
    uint16_t offset;
    MemberAttr attr;
};

struct _boolprops_t
{
    bool_t def;
};

struct _intprops_t
{
    bool_t is_signed;
    int64_t def;
};

struct _realprops_t
{
    real64_t def;
};

struct _enumprops_t
{
    ArrSt(EnumMember) *members;
};

struct _stringprops_t
{
    FPtr_str_create func_create;
    FPtr_destroy func_destroy;
    FPtr_str_get func_get;
    FPtr_read func_read;
    FPtr_write func_write;
    void *def;
};

struct _structprops_t
{
    bool_t is_union;
    ArrSt(StructMember) *members;
};

struct _containerprops_t
{
    char_t sep_st[16];
    char_t sep_ed[16];
    bool_t store_pointers;
    FPtr_container_create func_create;
    FPtr_container_size func_size;
    FPtr_container_get func_get;
    FPtr_container_insert func_insert;
    FPtr_container_delete func_delete;
    FPtr_container_destroy func_destroy;
};

struct _binaryprops_t
{
    FPtr_copy func_copy;
    FPtr_read func_read;
    FPtr_write func_write;
    FPtr_destroy func_destroy;
};

union _dbindprops_t
{
    BoolProps boolp;
    IntProps intp;
    RealProps realp;
    EnumProps enump;
    StringProps stringp;
    StructProps structp;
    ContainerProps contp;
    BinaryProps binaryp;
};

struct _dbind_t
{
    String *name;
    dtype_t type;
    uint16_t size;
    DBindProps props;
};

struct _alias_t
{
    String *name;
    DBind *bind;
};

struct _databind_t
{
    ArrPt(DBind) *binds;
    ArrSt(Alias) *alias;
};

/*---------------------------------------------------------------------------*/

DeclSt(EnumMember);
DeclSt(StructMember);
DeclSt(Alias);
DeclPt(DBind);
static DataBind i_DATABIND = {0, 0};
static real64_t i_EPSILON = 0.00001;

/*---------------------------------------------------------------------------*/

static dbindst_t i_try_unreg(const DBind *bind, const uint32_t alias_id);
static void i_destroy_struct_data(byte_t **data, const StructProps *props, const char_t *name, const uint16_t esize);
static void i_copy_struct_data(byte_t *dest, const byte_t *src, const StructProps *props);
static void i_remove_struct_data(byte_t *data, const StructProps *props);
static int i_cmp_struct_data(const byte_t *data1, const byte_t *data2, const StructProps *props);
static void i_read_struct_data(Stream *stm, byte_t *data, const StructProps *props);
static byte_t *i_read_container(Stream *stm, const DBind *bind, const DBind *ebind);
static void i_write_struct_data(Stream *stm, const byte_t *data, const StructProps *props);
static void i_write_container(Stream *stm, const byte_t *data, const DBind *bind, const DBind *ebind);
static void i_destroy_data(byte_t **data, const DBind *bind, const DBind *ebind);

/*---------------------------------------------------------------------------*/

static void i_remove_data(byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_STRUCT:
        i_remove_struct_data(data, &bind->props.structp);
        break;

    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_valid_container_type(const DBind *ebind, const bool_t store_pointers)
{
    cassert_no_null(ebind);
    switch (ebind->type)
    {
    /* Basic types only allowed in non-pointer containers */
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
        return !store_pointers;

    /* Struct types allowed in both (pointer and non-pointer containers) */
    case ekDTYPE_STRUCT:
        return TRUE;

    /* String and Binary types are opaque (only allowed in pointer containers) */
    case ekDTYPE_STRING:
    case ekDTYPE_BINARY:
        return store_pointers;

    /* Nested containers are not allowed */
    case ekDTYPE_CONTAINER:
        return FALSE;

    case ekDTYPE_UNKNOWN:
        cassert(FALSE);
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_destroy_container(byte_t **data, const DBind *bind, const DBind *ebind)
{
    uint32_t i = 0, n = 0;
    cassert_no_null(bind);
    cassert_no_null(ebind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    cassert(i_valid_container_type(ebind, bind->props.contp.store_pointers) == TRUE);
    cassert_no_null(data);
    n = bind->props.contp.func_size(*data);
    if (bind->props.contp.store_pointers == TRUE)
    {
        for (i = 0; i < n; ++i)
        {
            byte_t **elem = dcast(bind->props.contp.func_get(*data, i, tc(ebind->name), ebind->size), byte_t);
            if (*elem != NULL)
                i_destroy_data(elem, ebind, NULL);
        }
    }
    else
    {
        for (i = 0; i < n; ++i)
        {
            byte_t *elem = bind->props.contp.func_get(*data, i, tc(ebind->name), ebind->size);
            i_remove_data(elem, ebind);
        }
    }

    bind->props.contp.func_destroy(data, tc(ebind->name));
}

/*---------------------------------------------------------------------------*/

static void i_destroy_data(byte_t **data, const DBind *bind, const DBind *ebind)
{
    cassert_no_null(data);
    cassert_no_null(*data);
    cassert_no_null(bind);
    i_remove_data(*data, bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
        heap_free(data, bind->size, tc(bind->name));
        break;

    case ekDTYPE_STRING:
        bind->props.stringp.func_destroy(dcast(data, void));
        break;

    case ekDTYPE_STRUCT:
        heap_free(data, bind->size, tc(bind->name));
        break;

    case ekDTYPE_BINARY:
        bind->props.binaryp.func_destroy(dcast(data, void));
        break;

    case ekDTYPE_CONTAINER:
        i_destroy_container(data, bind, ebind);
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_remove_struct_data(byte_t *data, const StructProps *props)
{
    cassert_no_null(props);
    arrst_foreach(member, props->members, StructMember)
        cassert_no_null(member->bind);
        switch (member->bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            break;

        case ekDTYPE_STRING:
        {
            void **str = dcast(data + member->offset, void);
            if (*str != NULL)
                member->bind->props.stringp.func_destroy(str);
            break;
        }

        case ekDTYPE_STRUCT:
            if (member->attr.structt.is_pointer == TRUE)
            {
                byte_t **obj = dcast(data + member->offset, byte_t);
                if (*obj != NULL)
                    i_destroy_struct_data(obj, &member->bind->props.structp, tc(member->bind->name), member->bind->size);
            }
            else
            {
                i_remove_struct_data(data + member->offset, &member->bind->props.structp);
            }
            break;

        case ekDTYPE_BINARY:
        {
            void **bin = dcast(data + member->offset, void);
            if (*bin != NULL)
                member->bind->props.binaryp.func_destroy(bin);
            break;
        }

        case ekDTYPE_CONTAINER:
        {
            byte_t **cont = dcast(data + member->offset, byte_t);
            if (*cont != NULL)
                i_destroy_container(cont, member->bind, member->attr.containert.bind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }

    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_destroy_struct_data(byte_t **data, const StructProps *props, const char_t *name, const uint16_t esize)
{
    cassert_no_null(data);
    if (*data != NULL)
    {
        i_remove_struct_data(*data, props);
        heap_free(data, esize, name);
    }
}

/*---------------------------------------------------------------------------*/

static void i_remove_member_attr(MemberAttr *attr, const DBind *bind)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        break;

    case ekDTYPE_INT:
        str_destroy(&attr->intt.format);
        break;

    case ekDTYPE_REAL:
        str_destroy(&attr->realt.format);
        break;

    case ekDTYPE_ENUM:
        break;

    case ekDTYPE_STRING:
        if (attr->stringt.def != NULL)
            bind->props.stringp.func_destroy(&attr->stringt.def);
        break;

    case ekDTYPE_STRUCT:
        attr->structt.is_pointer = FALSE;
        if (attr->structt.def != NULL)
            i_destroy_struct_data(&attr->structt.def, &bind->props.structp, tc(bind->name), bind->size);
        break;

    case ekDTYPE_BINARY:
        if (attr->binaryt.def != NULL)
            bind->props.binaryp.func_destroy(&attr->binaryt.def);
        break;

    case ekDTYPE_CONTAINER:
        if (attr->containert.def != NULL)
            i_destroy_container(&attr->containert.def, bind, attr->containert.bind);
        attr->containert.bind = NULL;
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_remove_enum_member(EnumMember *member)
{
    cassert_no_null(member);
    str_destroy(&member->name);
    str_destroy(&member->alias);
}

/*---------------------------------------------------------------------------*/

static void i_remove_struct_member(StructMember *member)
{
    cassert_no_null(member);
    cassert_no_null(member->bind);
    str_destroy(&member->name);
    i_remove_member_attr(&member->attr, member->bind);
}

/*---------------------------------------------------------------------------*/

static void i_remove_enum_props(EnumProps *props)
{
    cassert_no_null(props);
    arrst_destroy(&props->members, i_remove_enum_member, EnumMember);
}

/*---------------------------------------------------------------------------*/

static void i_remove_string_props(StringProps *props)
{
    cassert_no_null(props);
    props->func_destroy(&props->def);
}

/*---------------------------------------------------------------------------*/

static void i_remove_struct_props(StructProps *props)
{
    cassert_no_null(props);
    arrst_destroy(&props->members, i_remove_struct_member, StructMember);
}

/*---------------------------------------------------------------------------*/

static void i_remove_dbind(DBind *dbind)
{
    cassert_no_null(dbind);
    switch (dbind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
        break;
    case ekDTYPE_ENUM:
        i_remove_enum_props(&dbind->props.enump);
        break;
    case ekDTYPE_STRING:
        i_remove_string_props(&dbind->props.stringp);
        break;
    case ekDTYPE_STRUCT:
        i_remove_struct_props(&dbind->props.structp);
        break;
    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
        break;
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    str_destroy(&dbind->name);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_dbind_full(DBind **dbind)
{
    cassert_no_null(dbind);
    i_remove_dbind(*dbind);
    heap_delete(dbind, DBind);
}

/*---------------------------------------------------------------------------*/

static void i_remove_alias(Alias *alias)
{
    cassert_no_null(alias);
    str_destroy(&alias->name);
}

/*---------------------------------------------------------------------------*/

void _dbind_start(void)
{
    if (i_DATABIND.binds == NULL)
    {
        i_DATABIND.binds = arrpt_create(DBind);
        i_DATABIND.alias = arrst_create(Alias);
    }
}

/*---------------------------------------------------------------------------*/

void _dbind_finish(void)
{
    if (i_DATABIND.binds != NULL)
    {
        /* Destroy non-dependent bindings firsts */
        while (arrpt_size(i_DATABIND.binds, DBind) > 0)
        {
            arrpt_foreach(bind, i_DATABIND.binds, DBind)
                if (i_try_unreg(bind, UINT32_MAX) == ekDBIND_OK)
                    break;
            arrpt_end()
        }

        arrpt_destroy(&i_DATABIND.binds, NULL, DBind);
        arrst_destroy(&i_DATABIND.alias, i_remove_alias, Alias);
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_get_bool(const byte_t *data, const uint16_t size)
{
    cassert_no_null(data);
    switch (size)
    {
    case 1:
        return *cast(data, uint8_t) == 0 ? FALSE : TRUE;
    case 2:
        return *cast(data, uint16_t) == 0 ? FALSE : TRUE;
    case 4:
        return *cast(data, uint32_t) == 0 ? FALSE : TRUE;
        /* Bool types more than 4 bytes are not allowed */
        cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_set_bool(byte_t *data, const uint16_t size, bool_t value)
{
    cassert_no_null(data);
    cassert(value == 0 || value == 1);
    switch (size)
    {
    case 1:
        *cast(data, bool_t) = value;
        break;
    case 2:
        *cast(data, uint16_t) = value;
        break;
    case 4:
        *cast(data, uint32_t) = value;
        break;
        /* Bool types more than 4 bytes are not allowed */
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static int64_t i_get_int(const byte_t *data, const uint16_t size, const bool_t is_signed)
{
    cassert_no_null(data);
    switch (size)
    {
    case 1:
        if (is_signed == TRUE)
            return (int64_t)*cast_const(data, int8_t);
        else
            return (int64_t)*cast_const(data, uint8_t);
        break;

    case 2:
        if (is_signed == TRUE)
            return (int64_t)*cast_const(data, int16_t);
        else
            return (int64_t)*cast_const(data, uint16_t);
        break;

    case 4:
        if (is_signed == TRUE)
            return (int64_t)*cast_const(data, int32_t);
        else
            return (int64_t)*cast_const(data, uint32_t);
        break;

    case 8:
        if (is_signed == TRUE)
            return *cast_const(data, int64_t);
        else
            return (int64_t)*cast_const(data, uint64_t);
        break;

        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_set_int(byte_t *data, const uint16_t size, const bool_t is_signed, int64_t value)
{
    cassert_no_null(data);
    switch (size)
    {
    case 1:
        if (is_signed == TRUE)
            *cast(data, int8_t) = (int8_t)value;
        else
            *cast(data, uint8_t) = (uint8_t)value;
        break;

    case 2:
        if (is_signed == TRUE)
            *cast(data, int16_t) = (int16_t)value;
        else
            *cast(data, uint16_t) = (uint16_t)value;
        break;

    case 4:
        if (is_signed == TRUE)
            *cast(data, int32_t) = (int32_t)value;
        else
            *cast(data, uint32_t) = (uint32_t)value;
        break;

    case 8:
        if (is_signed == TRUE)
            *cast(data, int64_t) = (int64_t)value;
        else
            *cast(data, uint64_t) = (uint64_t)value;
        break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static real64_t i_get_real(const byte_t *data, const uint16_t size)
{
    cassert_no_null(data);
    switch (size)
    {
    case 4:
        return (real64_t)*cast(data, real32_t);
    case 8:
        return *cast(data, real64_t);
        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static enum_t i_get_enum(const byte_t *data, const uint16_t size)
{
    cassert_no_null(data);
    cassert_unref(size == sizeof(enum_t), size);
    return *cast(data, enum_t);
}

/*---------------------------------------------------------------------------*/

static void i_set_real(byte_t *data, const uint16_t size, real64_t value)
{
    cassert_no_null(data);
    switch (size)
    {
    case 4:
        *cast(data, real32_t) = (real32_t)value;
        break;
    case 8:
        *cast(data, real64_t) = value;
        break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_set_enum(byte_t *data, const uint16_t size, enum_t value)
{
    cassert_no_null(data);
    cassert_unref(size == sizeof(enum_t), size);
    *cast(data, enum_t) = value;
}

/*---------------------------------------------------------------------------*/

static uint32_t i_clean_spaces(char_t *dest, const uint32_t size, const char_t *src)
{
    uint32_t i = 0;
    cassert_no_null(dest);
    cassert_no_null(src);
    while (i < size - 1 && *src != 0)
    {
        if (*src != ' ')
        {
            dest[i] = *src;
            i += 1;
        }

        src += 1;
    }

    dest[i] = 0;
    return i - 1;
}

/*---------------------------------------------------------------------------*/

static DBind *i_dbind_from_typename(const char_t *name, bool_t *is_pointer, uint32_t *alias_id)
{
    char_t mtype[256];
    uint32_t len = 0;

    len = i_clean_spaces(mtype, sizeof(mtype), name);
    cassert(len > 0);
    cassert_no_null(is_pointer);

    if (mtype[len] == '*')
    {
        *is_pointer = TRUE;
        mtype[len] = '\0';
    }
    else
    {
        *is_pointer = FALSE;
    }

    arrst_foreach(alias, i_DATABIND.alias, Alias)
        if (str_equ(alias->name, mtype) == TRUE)
        {
            ptr_assign(alias_id, alias_i);
            return alias->bind;
        }
    arrst_end()

    ptr_assign(alias_id, UINT32_MAX);
    arrpt_foreach(bind, i_DATABIND.binds, DBind)
        switch (bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
        case ekDTYPE_STRING:
        case ekDTYPE_STRUCT:
        case ekDTYPE_BINARY:
            if (str_equ(bind->name, mtype) == TRUE)
                return bind;
            break;

        case ekDTYPE_CONTAINER:
        {
            /* Check if typename begins with container name */
            const char_t *prefix = str_str(mtype, bind->props.contp.sep_st);
            if (prefix != NULL)
            {
                uint32_t nc = (uint32_t)(prefix - mtype);
                if (str_equ_cn(mtype, tc(bind->name), nc) == TRUE)
                    return bind;
            }
            break;
        }
        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    arrpt_end()
    return NULL;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_bool_imp(const char_t *type, const uint16_t size)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_BOOL;
        bind->size = size;
        bind->props.boolp.def = FALSE;
    }
    else
    {
        cassert(bind->type == ekDTYPE_BOOL);
        st = ekDBIND_TYPE_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_int_imp(const char_t *type, const uint16_t size, const bool_t is_signed)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_INT;
        bind->size = size;
        bind->props.intp.is_signed = is_signed;
        bind->props.intp.def = 0;
    }
    else
    {
        cassert(bind->type == ekDTYPE_INT);
        st = ekDBIND_TYPE_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_real_imp(const char_t *type, const uint16_t size)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_REAL;
        bind->size = size;
        bind->props.realp.def = 0;
    }
    else
    {
        cassert(bind->type == ekDTYPE_REAL);
        st = ekDBIND_TYPE_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_string_imp(const char_t *type, FPtr_str_create func_create, FPtr_destroy func_destroy, FPtr_str_get func_get, FPtr_read func_read, FPtr_write func_write, const char_t *def)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    cassert_no_nullf(func_create);
    cassert_no_nullf(func_destroy);
    cassert_no_nullf(func_get);
    cassert_no_nullf(func_read);
    cassert_no_nullf(func_write);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_STRING;
        bind->size = sizeofptr;
        bind->props.stringp.func_create = func_create;
        bind->props.stringp.func_destroy = func_destroy;
        bind->props.stringp.func_get = func_get;
        bind->props.stringp.func_read = func_read;
        bind->props.stringp.func_write = func_write;
        bind->props.stringp.def = func_create(def);
    }
    else
    {
        cassert(bind->type == ekDTYPE_STRING);
        st = ekDBIND_TYPE_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_container_imp(const char_t *type, const bool_t store_pointers, FPtr_container_create func_create, FPtr_container_size func_size, FPtr_container_get func_get, FPtr_container_insert func_insert, FPtr_container_delete func_delete, FPtr_container_destroy func_destroy)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    cassert_no_nullf(func_create);
    cassert_no_nullf(func_size);
    cassert_no_nullf(func_get);
    cassert_no_nullf(func_insert);
    cassert_no_nullf(func_delete);
    cassert_no_nullf(func_destroy);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_CONTAINER;
        bind->size = sizeofptr;
        bind->props.contp.store_pointers = store_pointers;
        str_copy_c(bind->props.contp.sep_st, sizeof(bind->props.contp.sep_st), "(");
        str_copy_c(bind->props.contp.sep_ed, sizeof(bind->props.contp.sep_ed), ")");
        bind->props.contp.func_create = func_create;
        bind->props.contp.func_size = func_size;
        bind->props.contp.func_get = func_get;
        bind->props.contp.func_insert = func_insert;
        bind->props.contp.func_delete = func_delete;
        bind->props.contp.func_destroy = func_destroy;
    }
    else
    {
        cassert(bind->type == ekDTYPE_CONTAINER);
        st = ekDBIND_TYPE_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

static EnumMember *i_enum_member(ArrSt(EnumMember) *members, const char_t *name, const enum_t value, uint32_t *index)
{
    /* We look for the member by name */
    arrst_foreach(member, members, EnumMember)
        if (str_equ(member->name, name) == TRUE)
            return member;
    arrst_end()

    /* The member doesn't exist. We look for the insert position (ordered by value)	*/
    cassert_no_null(index);
    *index = UINT32_MAX;

    arrst_foreach(member, members, EnumMember)
        if (member->value > value)
        {
            *index = member_i;
            break;
        }
    arrst_end()

    if (*index == UINT32_MAX)
        *index = arrst_size(members, EnumMember);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_enum_imp(const char_t *type, const char_t *name, const enum_t value, const char_t *alias)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    EnumMember *member = NULL;
    uint32_t index = UINT32_MAX;
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_ENUM;
        bind->size = sizeof(enum_t);
        bind->props.enump.members = arrst_create(EnumMember);
    }
    else
    {
        cassert(bind->type == ekDTYPE_ENUM);
    }

    member = i_enum_member(bind->props.enump.members, name, value, &index);
    if (member == NULL)
    {
        member = arrst_insert_n(bind->props.enump.members, index, 1, EnumMember);
        member->name = str_c(name);
        member->value = value;
        member->alias = NULL;
    }
    else
    {
        cassert(str_equ(member->name, name) == TRUE);
        cassert(member->value == value);
        st = ekDBIND_MEMBER_EXISTS;
    }

    if (str_empty_c(alias) == TRUE)
        str_upd(&member->alias, tc(member->name));
    else
        str_upd(&member->alias, alias);

    return st;
}

/*---------------------------------------------------------------------------*/

static StructMember *i_find_member(ArrSt(StructMember) *members, const char_t *mname, const uint16_t moffset, uint32_t *index, bool_t *is_union)
{
    /* We look for the member by name */
    arrst_foreach(member, members, StructMember)
        if (str_equ(member->name, mname) == TRUE)
            return member;
    arrst_end()

    /* The member doesn't exist. We look for the insert position */
    ptr_assign(index, UINT32_MAX);

    arrst_foreach(member, members, StructMember)
        if (member->offset == moffset)
        {
            /* The union new members will be added at end */
            ptr_assign(is_union, TRUE);
            cassert(moffset == 0);
            break;
        }
        else if (member->offset > moffset)
        {
            ptr_assign(index, member_i);
            break;
        }
    arrst_end()

    if (index != NULL)
    {
        if (*index == UINT32_MAX)
            *index = arrst_size(members, StructMember);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static String *i_inner_elem_type(const char_t *name, const DBind *bind)
{
    cassert_no_null(bind);
    if (bind->type == ekDTYPE_CONTAINER)
    {
        const char_t *st = NULL, *ed = NULL;
        char_t mtype[256];
        uint32_t len = i_clean_spaces(mtype, sizeof(mtype), name);
        cassert_unref(len > 0, len);
        st = str_str(mtype, bind->props.contp.sep_st);
        if (st != NULL)
        {
            st += 1;
            ed = str_str(st, bind->props.contp.sep_ed);
        }

        if (st != NULL && ed != NULL)
            return str_cn(st, (uint32_t)(ed - st));
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

static ___INLINE int64_t i_clamp_int(const int64_t value, const int64_t min, const int64_t max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

/*---------------------------------------------------------------------------*/

static ___INLINE int64_t i_member_clamp_int(const StructMember *member, const int64_t value)
{
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_INT);
    return i_clamp_int(value, member->attr.intt.min, member->attr.intt.max);
}

/*---------------------------------------------------------------------------*/

static ___INLINE real64_t i_clamp_real(const real64_t value, const real64_t min, const real64_t max, const real64_t prec)
{
    real64_t v = bmath_clampd(value, min, max);
    return bmath_round_stepd(v, prec);
}

/*---------------------------------------------------------------------------*/

static ___INLINE real64_t i_member_clamp_real(const StructMember *member, const real64_t value)
{
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_REAL);
    return i_clamp_real(value, member->attr.realt.min, member->attr.realt.max, member->attr.realt.prec);
}

/*---------------------------------------------------------------------------*/

static byte_t *i_create_container(const DBind *bind, const DBind *ebind)
{
    cassert_no_null(bind);
    cassert_no_null(ebind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    if (i_valid_container_type(ebind, bind->props.contp.store_pointers) == TRUE)
        return bind->props.contp.func_create(tc(ebind->name), ebind->size);

    cassert_msg(FALSE, "Non allowed element type for this container");
    return NULL;
}

/*---------------------------------------------------------------------------*/

static ___INLINE byte_t *i_dbind_calloc(const DBind *bind)
{
    cassert_no_null(bind);
    return heap_calloc_imp(bind->size, tc(bind->name), TRUE);
}

/*---------------------------------------------------------------------------*/

static byte_t *i_copy_container(const byte_t *src, const DBind *bind, const DBind *ebind)
{
    byte_t *dest = i_create_container(bind, ebind);
    uint32_t n = 0;
    cassert_no_null(bind);
    cassert_no_null(ebind);
    n = bind->props.contp.func_size(src);
    if (n > 0)
    {
        byte_t *dest_data = bind->props.contp.func_insert(dest, 0, n, tc(ebind->name), ebind->size);
        const byte_t *src_data = bind->props.contp.func_get(cast(src, byte_t), 0, tc(ebind->name), ebind->size);

        if (bind->props.contp.store_pointers == TRUE)
        {
            uint32_t i;
            for (i = 0; i < n; ++i)
            {
                byte_t **dest_pdata = dcast(dest_data, byte_t);
                const byte_t **src_pdata = dcast_const(src_data, byte_t);
                if (*src_pdata != NULL)
                {
                    switch (ebind->type)
                    {
                    case ekDTYPE_BOOL:
                    case ekDTYPE_INT:
                    case ekDTYPE_REAL:
                    case ekDTYPE_ENUM:
                        *dest_pdata = i_dbind_calloc(ebind);
                        bmem_copy(*dest_pdata, *src_pdata, ebind->size);
                        break;

                    case ekDTYPE_STRUCT:
                        *dest_pdata = i_dbind_calloc(ebind);
                        i_copy_struct_data(*dest_pdata, *src_pdata, &ebind->props.structp);
                        break;

                    case ekDTYPE_STRING:
                    {
                        const char_t *value = ebind->props.stringp.func_get(*dcast(src_pdata, void));
                        *dest_pdata = cast(ebind->props.stringp.func_create(value), byte_t);
                        break;
                    }

                    case ekDTYPE_BINARY:
                        *dest_pdata = cast(ebind->props.binaryp.func_copy(*dcast(src_pdata, void)), byte_t);
                        break;

                    /* Nested containers are not allowed */
                    case ekDTYPE_CONTAINER:
                    case ekDTYPE_UNKNOWN:
                        cassert_default();
                    }
                }
                else
                {
                    *dest_pdata = NULL;
                }

                dest_data += sizeofptr;
                src_data += sizeofptr;
            }
        }
        else
        {
            switch (ebind->type)
            {
            case ekDTYPE_BOOL:
            case ekDTYPE_INT:
            case ekDTYPE_REAL:
            case ekDTYPE_ENUM:
                bmem_copy(dest_data, src_data, n * ebind->size);
                break;

            case ekDTYPE_STRUCT:
            {
                uint32_t i;
                bmem_set_zero(dest_data, n * ebind->size);
                for (i = 0; i < n; ++i)
                {
                    i_copy_struct_data(dest_data, src_data, &ebind->props.structp);
                    dest_data += ebind->size;
                    src_data += ebind->size;
                }
                break;
            }

            /* Not allowed for non-pointer cointainers */
            case ekDTYPE_STRING:
            case ekDTYPE_BINARY:
            case ekDTYPE_CONTAINER:
            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }

    return dest;
}

/*---------------------------------------------------------------------------*/

static void i_copy_struct_data(byte_t *dest, const byte_t *src, const StructProps *props)
{
    cassert_no_null(props);
    arrst_foreach(member, props->members, StructMember)
        cassert_no_null(member->bind);
        switch (member->bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            bmem_copy(dest + member->offset, src + member->offset, member->bind->size);
            break;

        case ekDTYPE_STRING:
        {
            void **dest_str = dcast(dest + member->offset, void);
            void **src_str = dcast(src + member->offset, void);
            cassert(*dest_str == NULL);
            if (*src_str != NULL)
            {
                const char_t *value = member->bind->props.stringp.func_get(*src_str);
                *dest_str = member->bind->props.stringp.func_create(value);
            }
            break;
        }

        case ekDTYPE_STRUCT:
            cassert(member->bind->props.structp.is_union == FALSE);
            if (member->attr.structt.is_pointer == TRUE)
            {
                byte_t **dest_data = dcast(dest + member->offset, byte_t);
                byte_t **src_data = dcast(src + member->offset, byte_t);
                cassert(*dest_data == NULL);
                if (*src_data != NULL)
                {
                    *dest_data = i_dbind_calloc(member->bind);
                    i_copy_struct_data(*dest_data, *src_data, &member->bind->props.structp);
                }
            }
            else
            {
                i_copy_struct_data(dest + member->offset, src + member->offset, &member->bind->props.structp);
            }
            break;

        case ekDTYPE_BINARY:
        {
            void **dest_obj = dcast(dest + member->offset, void);
            void **src_obj = dcast(src + member->offset, void);
            cassert(*dest_obj == NULL);
            if (*src_obj != NULL)
                *dest_obj = member->bind->props.binaryp.func_copy(*src_obj);
            break;
        }

        case ekDTYPE_CONTAINER:
        {
            byte_t **dest_cont = dcast(dest + member->offset, byte_t);
            byte_t **src_cont = dcast(src + member->offset, byte_t);
            cassert(*dest_cont == NULL);
            if (*src_cont != NULL)
                *dest_cont = i_copy_container(*src_cont, member->bind, member->attr.containert.bind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static void i_init_struct_data(byte_t *data, const StructProps *props)
{
    cassert_no_null(props);
    arrst_foreach(member, props->members, StructMember)
        cassert_no_null(member->bind);
        switch (member->bind->type)
        {
        case ekDTYPE_BOOL:
            i_set_bool(data + member->offset, member->bind->size, member->attr.boolt.def);
            break;

        case ekDTYPE_INT:
        {
            int64_t value = i_member_clamp_int(member, member->attr.intt.def);
            i_set_int(data + member->offset, member->bind->size, member->bind->props.intp.is_signed, value);
            break;
        }

        case ekDTYPE_REAL:
        {
            real64_t value = i_member_clamp_real(member, member->attr.realt.def);
            i_set_real(data + member->offset, member->bind->size, value);
            break;
        }

        case ekDTYPE_ENUM:
            i_set_enum(data + member->offset, member->bind->size, member->attr.enumt.def);
            break;

        case ekDTYPE_STRING:
        {
            const char_t *value = NULL;
            byte_t **nstr = dcast(data + member->offset, byte_t);
            cassert(*nstr == NULL);

            if (member->attr.stringt.def != NULL)
                value = member->bind->props.stringp.func_get(member->attr.stringt.def);

            if (value != NULL)
                *nstr = member->bind->props.stringp.func_create(value);
            break;
        }

        case ekDTYPE_STRUCT:
            cassert(member->bind->props.structp.is_union == FALSE);
            if (member->attr.structt.is_pointer == TRUE)
            {
                byte_t **ndata = dcast(data + member->offset, byte_t);
                cassert(*ndata == NULL);
                if (member->attr.structt.def != NULL)
                {
                    *ndata = i_dbind_calloc(member->bind);
                    i_copy_struct_data(*ndata, member->attr.structt.def, &member->bind->props.structp);
                }
            }
            else
            {
                cassert_no_null(member->attr.structt.def);
                i_copy_struct_data(data + member->offset, member->attr.structt.def, &member->bind->props.structp);
            }
            break;

        case ekDTYPE_BINARY:
        {
            void **obj = dcast(data + member->offset, void);
            cassert(*obj == NULL);
            if (member->attr.binaryt.def != NULL)
                *obj = member->bind->props.binaryp.func_copy(member->attr.binaryt.def);
            break;
        }

        case ekDTYPE_CONTAINER:
        {
            byte_t **cont = dcast(data + member->offset, byte_t);
            cassert(*cont == NULL);
            if (member->attr.containert.def != NULL)
                *cont = i_copy_container(member->attr.containert.def, member->bind, member->attr.containert.bind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

static DBind *i_inner_elem_bind(const DBind *bind, const char_t *type)
{
    String *etype = i_inner_elem_type(type, bind);
    cassert_no_null(bind);
    if (etype != NULL)
    {
        bool_t is_pointer = FALSE;
        DBind *ebind = i_dbind_from_typename(tc(etype), &is_pointer, NULL);
        cassert_unref(is_pointer == FALSE, is_pointer);
        str_destroy(&etype);
        return ebind;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

static dbindst_t i_add_member(DBind *bind, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize)
{
    dbindst_t st = ekDBIND_OK;
    StructMember *member = NULL;
    uint32_t index = UINT32_MAX;
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_STRUCT);
    member = i_find_member(bind->props.structp.members, mname, moffset, &index, &bind->props.structp.is_union);
    if (member == NULL)
    {
        bool_t is_pointer = FALSE;
        DBind *mbind = i_dbind_from_typename(mtype, &is_pointer, NULL);
        if (mbind != NULL)
        {
            member = arrst_insert_n(bind->props.structp.members, index, 1, StructMember);
            member->bind = mbind;
            member->name = str_c(mname);
            member->offset = moffset;
            cassert_unref((!is_pointer && mbind->size == msize) || (msize == sizeofptr), msize);

            /* Initialize member attributes */
            switch (mbind->type)
            {
            case ekDTYPE_BOOL:
                cassert(is_pointer == FALSE);
                member->attr.boolt.def = mbind->props.boolp.def;
                break;

            case ekDTYPE_INT:
                cassert(is_pointer == FALSE);
                member->attr.intt.def = mbind->props.intp.def;
                member->attr.intt.incr = 1;
                member->attr.intt.format = str_c(mbind->props.intp.is_signed ? "%%d" : "%%u");

                switch (mbind->size)
                {
                case 1:
                    member->attr.intt.min = mbind->props.intp.is_signed ? (int64_t)INT8_MIN : 0;
                    member->attr.intt.max = mbind->props.intp.is_signed ? (int64_t)INT8_MAX : (int64_t)UINT8_MAX;
                    break;
                case 2:
                    member->attr.intt.min = mbind->props.intp.is_signed ? (int64_t)INT16_MIN : 0;
                    member->attr.intt.max = mbind->props.intp.is_signed ? (int64_t)INT16_MAX : (int64_t)UINT16_MAX;
                    break;
                case 4:
                    member->attr.intt.min = mbind->props.intp.is_signed ? (int64_t)INT32_MIN : 0;
                    member->attr.intt.max = mbind->props.intp.is_signed ? (int64_t)INT32_MAX : (int64_t)UINT32_MAX;
                    break;
                case 8:
                    member->attr.intt.min = mbind->props.intp.is_signed ? INT64_MIN : 0;
                    member->attr.intt.max = mbind->props.intp.is_signed ? INT64_MAX : (int64_t)INT64_MAX;
                    break;
                    cassert_default();
                }
                break;

            case ekDTYPE_REAL:
                cassert(is_pointer == FALSE);
                member->attr.realt.def = mbind->props.realp.def;
                member->attr.realt.min = -1e8;
                member->attr.realt.max = 1e8;
                member->attr.realt.prec = .01;
                member->attr.realt.incr = 1;
                member->attr.realt.dec = 2;
                member->attr.realt.format = str_c("%.2f");
                break;

            case ekDTYPE_ENUM:
            {
                const EnumMember *first = arrst_get(mbind->props.enump.members, 0, EnumMember);
                cassert(is_pointer == FALSE);
                cassert_no_null(first);
                member->attr.enumt.def = first->value;
                break;
            }

            case ekDTYPE_STRING:
            {
                const char_t *def = mbind->props.stringp.def != NULL ? mbind->props.stringp.func_get(mbind->props.stringp.def) : NULL;
                cassert(is_pointer == TRUE);
                if (def != NULL)
                    member->attr.stringt.def = mbind->props.stringp.func_create(def);
                else
                    member->attr.stringt.def = NULL;
                break;
            }

            case ekDTYPE_STRUCT:
                member->attr.structt.is_pointer = is_pointer;
                member->attr.structt.def = i_dbind_calloc(member->bind);
                i_init_struct_data(member->attr.structt.def, &member->bind->props.structp);
                break;

            case ekDTYPE_BINARY:
                cassert(is_pointer == TRUE);
                member->attr.binaryt.def = NULL;
                break;

            case ekDTYPE_CONTAINER:
                cassert(is_pointer == TRUE);
                member->attr.containert.bind = i_inner_elem_bind(mbind, mtype);
                member->attr.containert.def = i_create_container(mbind, member->attr.containert.bind);
                break;

            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
        else
        {
            st = ekDBIND_TYPE_UNKNOWN;
        }
    }
    else
    {
        st = ekDBIND_MEMBER_EXISTS;
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_imp(
    const char_t *type,
    const uint16_t size,
    const char_t *mname,
    const char_t *mtype,
    const uint16_t moffset,
    const uint16_t msize)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_STRUCT;
        bind->size = size;
        bind->props.structp.members = arrst_create(StructMember);
    }
    else
    {
        cassert(bind->type == ekDTYPE_STRUCT);
        cassert_unref(bind->size == size, size);
    }

    return i_add_member(bind, mname, mtype, moffset, msize);
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_binary_imp(
    const char_t *type,
    FPtr_copy func_copy,
    FPtr_read func_read,
    FPtr_write func_write,
    FPtr_destroy func_destroy)
{
    dbindst_t st = ekDBIND_OK;
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_unref(alias_id == UINT32_MAX, alias_id);
    if (bind == NULL)
    {
        bind = heap_new0(DBind);
        arrpt_append(i_DATABIND.binds, bind, DBind);
        bind->name = str_c(type);
        bind->type = ekDTYPE_BINARY;
        bind->size = sizeofptr;
        bind->props.binaryp.func_copy = func_copy;
        bind->props.binaryp.func_read = func_read;
        bind->props.binaryp.func_write = func_write;
        bind->props.binaryp.func_destroy = func_destroy;
    }
    else
    {
        cassert(bind->type == ekDTYPE_BINARY);
    }

    return st;
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_alias_imp(const char_t *type, const char_t *alias, const uint16_t type_size, const uint16_t alias_size)
{
    bool_t is_pointer = FALSE;
    DBind *abind = i_dbind_from_typename(alias, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (abind == NULL)
    {
        DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
        cassert_unref(is_pointer == FALSE, is_pointer);
        if (bind != NULL)
        {
            cassert_unref(bind->size == type_size, type_size);
            if (bind->size == alias_size)
            {
                char_t mtype[256];
                Alias *nalias = arrst_new(i_DATABIND.alias, Alias);
                i_clean_spaces(mtype, sizeof(mtype), alias);
                nalias->name = str_c(mtype);
                nalias->bind = bind;
                return ekDBIND_OK;
            }
            else
            {
                return ekDBIND_ALIAS_SIZE;
            }
        }
        else
        {
            return ekDBIND_TYPE_UNKNOWN;
        }
    }
    else
    {
        return ekDBIND_TYPE_EXISTS;
    }
}

/*---------------------------------------------------------------------------*/

static bool_t i_dbind_in_struct(const DBind *stbind, const DBind *bind)
{
    cassert_no_null(stbind);
    cassert_no_null(bind);
    cassert(stbind->type == ekDTYPE_STRUCT);
    arrst_foreach_const(member, stbind->props.structp.members, StructMember)
        if (member->bind == bind)
            return TRUE;

        if (member->bind->type == ekDTYPE_CONTAINER)
        {
            if (member->attr.containert.bind == bind)
                return TRUE;
        }

        if (member->bind->type == ekDTYPE_STRUCT)
        {
            bool_t exists = i_dbind_in_struct(member->bind, bind);
            if (exists == TRUE)
                return TRUE;
        }
    arrst_end()
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_binary_defaults_destroy(const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BINARY);
    if (i_DATABIND.binds != NULL)
    {
        arrpt_foreach(dbind, i_DATABIND.binds, DBind)
            if (dbind->type == ekDTYPE_STRUCT)
            {
                arrst_foreach(member, dbind->props.structp.members, StructMember)
                    cassert_no_null(member->bind);
                    if (member->bind == bind)
                    {
                        if (member->attr.binaryt.def != NULL)
                        {
                            cassert_no_nullf(bind->props.binaryp.func_destroy);
                            bind->props.binaryp.func_destroy(&member->attr.binaryt.def);
                        }
                    }
                arrst_end()
            }
        arrpt_end()
    }
}

/*---------------------------------------------------------------------------*/

static dbindst_t i_try_unreg(const DBind *bind, const uint32_t alias_id)
{
    if (alias_id == UINT32_MAX)
    {
        cassert_no_null(bind);

        if (bind->type == ekDTYPE_BINARY)
            i_binary_defaults_destroy(bind);

        arrpt_foreach_const(cbind, i_DATABIND.binds, DBind)
            if (cbind != bind)
            {
                if (cbind->type == ekDTYPE_STRUCT)
                {
                    bool_t exists = i_dbind_in_struct(cbind, bind);
                    if (exists == TRUE)
                        return ekDBIND_TYPE_USED;
                }
            }
        arrpt_end()

        /* If we are here the type to unbind is not used by other types */
        {
            uint32_t i, n = arrst_size(i_DATABIND.alias, Alias);
            for (i = 0; i < n;)
            {
                const Alias *alias = arrst_get_const(i_DATABIND.alias, i, Alias);
                cassert_no_null(alias);
                if (alias->bind == bind)
                {
                    arrst_delete(i_DATABIND.alias, i, i_remove_alias, Alias);
                    n -= 1;
                }
                else
                {
                    i += 1;
                }
            }
        }

        {
            uint32_t pos = arrpt_find(i_DATABIND.binds, bind, DBind);
            arrpt_delete(i_DATABIND.binds, pos, i_destroy_dbind_full, DBind);
            return ekDBIND_OK;
        }
    }
    else
    {
        arrst_delete(i_DATABIND.alias, alias_id, i_remove_alias, Alias);
        return ekDBIND_OK;
    }
}

/*---------------------------------------------------------------------------*/

dbindst_t dbind_unreg_imp(const char_t *type)
{
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
        return i_try_unreg(bind, alias_id);
    else
        return ekDBIND_TYPE_UNKNOWN;
}

/*---------------------------------------------------------------------------*/

static void i_init_bind(byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        i_set_bool(data, bind->size, bind->props.boolp.def);
        break;
    case ekDTYPE_INT:
        i_set_int(data, bind->size, bind->props.intp.is_signed, bind->props.intp.def);
        break;
    case ekDTYPE_REAL:
        i_set_real(data, bind->size, bind->props.realp.def);
        break;

    case ekDTYPE_ENUM:
    {
        const EnumMember *first = arrst_get(bind->props.enump.members, 0, EnumMember);
        cassert_no_null(first);
        i_set_enum(data, bind->size, first->value);
        break;
    }

    case ekDTYPE_STRING:
    {
        const char_t *def = bind->props.stringp.func_get(bind->props.stringp.def);
        cassert(*dcast(data, void) == NULL);
        *dcast(data, void) = bind->props.stringp.func_create(def);
        break;
    }

    case ekDTYPE_STRUCT:
        i_init_struct_data(data, &bind->props.structp);
        break;

    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static byte_t *i_create(const DBind *bind, const DBind *ebind)
{
    byte_t *data = NULL;
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRUCT:
        data = i_dbind_calloc(bind);
        i_init_bind(data, bind);
        break;
    case ekDTYPE_STRING:
        i_init_bind(cast(&data, byte_t), bind);
        break;
    case ekDTYPE_BINARY:
        /* No way to create an 'empty' binary object */
        data = NULL;
        break;
    case ekDTYPE_CONTAINER:
        data = i_create_container(bind, ebind);
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return data;
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_create_imp(const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        DBind *ebind = i_inner_elem_bind(bind, type);
        return i_create(bind, ebind);
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_copy_imp(const byte_t *data, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_no_null(data);
    if (bind != NULL)
    {
        byte_t *ndata = NULL;
        switch (bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            ndata = i_dbind_calloc(bind);
            bmem_copy(ndata, data, bind->size);
            break;

        case ekDTYPE_STRING:
        {
            const char_t *str = bind->props.stringp.func_get(cast_const(data, void));
            ndata = cast(bind->props.stringp.func_create(str), byte_t);
            break;
        }

        case ekDTYPE_STRUCT:
            ndata = i_dbind_calloc(bind);
            i_copy_struct_data(ndata, data, &bind->props.structp);
            break;

        case ekDTYPE_BINARY:
            ndata = bind->props.binaryp.func_copy(cast_const(data, void));
            break;

        case ekDTYPE_CONTAINER:
        {
            DBind *ebind = i_inner_elem_bind(bind, type);
            ndata = i_copy_container(data, bind, ebind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }

        return ndata;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

void dbind_init_imp(byte_t *data, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        bmem_set_zero(data, bind->size);
        i_init_bind(data, bind);
    }
}

/*---------------------------------------------------------------------------*/

void dbind_remove_imp(byte_t *data, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
        i_remove_data(data, bind);
}

/*---------------------------------------------------------------------------*/

void dbind_destroy_imp(byte_t **data, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        DBind *ebind = i_inner_elem_bind(bind, type);
        i_destroy_data(data, bind, ebind);
    }
}

/*---------------------------------------------------------------------------*/

void dbind_destopt_imp(byte_t **data, const char_t *type)
{
    cassert_no_null(data);
    if (*data != NULL)
        dbind_destroy_imp(data, type);
}

/*---------------------------------------------------------------------------*/

static int i_compare_type(const DBind *bind, const DBind *ebind, const byte_t *data1, const byte_t *data2)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
    {
        bool_t value1 = i_get_bool(data1, bind->size);
        bool_t value2 = i_get_bool(data2, bind->size);
        return (int)value1 - (int)value2;
    }

    case ekDTYPE_INT:
    {
        int64_t value1 = i_get_int(data1, bind->size, bind->props.intp.is_signed);
        int64_t value2 = i_get_int(data2, bind->size, bind->props.intp.is_signed);
        if (value1 < value2)
            return -1;
        if (value1 > value2)
            return 1;
        return 0;
    }

    case ekDTYPE_REAL:
    {
        real64_t value1 = i_get_real(data1, bind->size);
        real64_t value2 = i_get_real(data2, bind->size);
        if (value2 - value1 > i_EPSILON)
            return -1;
        if (value1 - value2 > i_EPSILON)
            return 1;
        return 0;
    }

    case ekDTYPE_ENUM:
    {
        enum_t value1 = i_get_enum(data1, bind->size);
        enum_t value2 = i_get_enum(data2, bind->size);
        if (value1 < value2)
            return -1;
        if (value1 > value2)
            return 1;
        return 0;
    }

    case ekDTYPE_STRING:
        if (data1 != NULL && data2 != NULL)
        {
            const char_t *str1 = bind->props.stringp.func_get(data1);
            const char_t *str2 = bind->props.stringp.func_get(data2);
            int cmp = str_cmp_c(str1, str2);
            if (cmp < 0)
                return -1;
            else if (cmp > 0)
                return 1;
            else
                return 0;
        }
        else if (data1 != NULL)
        {
            return 1;
        }
        else if (data2 != NULL)
        {
            return -1;
        }
        else
        {
            return 0;
        }

    case ekDTYPE_STRUCT:
        if (data1 != NULL && data2 != NULL)
            return i_cmp_struct_data(data1, data2, &bind->props.structp);
        else if (data1 != NULL)
            return 1;
        else if (data2 != NULL)
            return -1;
        else
            return 0;

    case ekDTYPE_BINARY:
        /* At the moment we don't compare the binary content (write/read streams required) */
        if (data1 != NULL && data2 != NULL)
            return 0;
        else if (data1 != NULL)
            return 1;
        else if (data2 != NULL)
            return -1;
        else
            return 0;

    case ekDTYPE_CONTAINER:

        if (data1 != NULL && data2 != NULL)
        {
            uint32_t n1 = dbind_container_size(bind, data1);
            uint32_t n2 = dbind_container_size(bind, data2);
            if (n1 == n2)
            {
                uint32_t i = 0;
                for (i = 0; i < n1; ++i)
                {
                    const byte_t *elem1 = dbind_container_cget(bind, ebind, i, data1);
                    const byte_t *elem2 = dbind_container_cget(bind, ebind, i, data2);
                    int cmp = i_compare_type(ebind, NULL, elem1, elem2);
                    if (cmp != 0)
                        return cmp;
                }

                return 0;
            }
            else if (n1 < n2)
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
        else if (data1 != NULL)
        {
            return 1;
        }
        else if (data2 != NULL)
        {
            return -1;
        }
        else
        {
            return 0;
        }

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static int i_cmp_struct_data(const byte_t *data1, const byte_t *data2, const StructProps *props)
{
    cassert_no_null(data1);
    cassert_no_null(data2);
    arrst_foreach_const(member, props->members, StructMember)
        const DBind *bind = member->bind;
        const DBind *ebind = NULL;
        const byte_t *edata1 = NULL;
        const byte_t *edata2 = NULL;
        int cmp = 0;

        switch (bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            edata1 = data1 + member->offset;
            edata2 = data2 + member->offset;
            break;

        case ekDTYPE_STRING:
        case ekDTYPE_BINARY:
            edata1 = *dcast(data1 + member->offset, byte_t);
            edata2 = *dcast(data2 + member->offset, byte_t);
            break;

        case ekDTYPE_STRUCT:
            if (member->attr.structt.is_pointer == TRUE)
            {
                edata1 = *dcast(data1 + member->offset, byte_t);
                edata2 = *dcast(data2 + member->offset, byte_t);
            }
            else
            {
                edata1 = data1 + member->offset;
                edata2 = data2 + member->offset;
            }
            break;

        case ekDTYPE_CONTAINER:
            ebind = member->attr.containert.bind;
            edata1 = *dcast(data1 + member->offset, byte_t);
            edata2 = *dcast(data2 + member->offset, byte_t);
            break;

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }

        cmp = i_compare_type(bind, ebind, edata1, edata2);
        if (cmp != 0)
            return cmp;
    arrst_end()

    return 0;
}

/*---------------------------------------------------------------------------*/

int dbind_cmp_imp(const byte_t *data1, const byte_t *data2, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    DBind *ebind = i_inner_elem_bind(bind, type);
    cassert_unref(is_pointer == FALSE, is_pointer);
    cassert_no_null(data1);
    cassert_no_null(data2);
    return i_compare_type(bind, ebind, data1, data2);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_equ_imp(const byte_t *data1, const byte_t *data2, const char_t *type)
{
    return dbind_cmp_imp(data1, data2, type) == 0;
}

/*---------------------------------------------------------------------------*/

static bool_t i_read_bool(Stream *stm, const DBind *bind)
{
    cassert_no_null(bind);
    cassert_unref(bind->type == ekDTYPE_BOOL, bind);
    return stm_read_bool(stm);
}

/*---------------------------------------------------------------------------*/

static int64_t i_read_int(Stream *stm, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_INT);
    switch (bind->size)
    {
    case 1:
        if (bind->props.intp.is_signed == TRUE)
            return (int64_t)stm_read_i8(stm);
        else
            return (int64_t)stm_read_u8(stm);

    case 2:
        if (bind->props.intp.is_signed == TRUE)
            return (int64_t)stm_read_i16(stm);
        else
            return (int64_t)stm_read_u16(stm);

    case 4:
        if (bind->props.intp.is_signed == TRUE)
            return (int64_t)stm_read_i32(stm);
        else
            return (int64_t)stm_read_u32(stm);

    case 8:
        if (bind->props.intp.is_signed == TRUE)
            return (int64_t)stm_read_i64(stm);
        else
            return (int64_t)stm_read_u64(stm);

        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static real64_t i_read_real(Stream *stm, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_REAL);
    switch (bind->size)
    {
    case 4:
        return (real64_t)stm_read_r32(stm);

    case 8:
        return (real64_t)stm_read_r64(stm);

        cassert_default();
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_read_bind(Stream *stm, byte_t *data, const DBind *bind, const DBind *ebind)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
    {
        bool_t value = i_read_bool(stm, bind);
        i_set_bool(data, bind->size, value);
        break;
    }

    case ekDTYPE_INT:
    {
        int64_t value = i_read_int(stm, bind);
        i_set_int(data, bind->size, bind->props.intp.is_signed, value);
        break;
    }

    case ekDTYPE_REAL:
    {
        real64_t value = i_read_real(stm, bind);
        i_set_real(data, bind->size, value);
        break;
    }

    case ekDTYPE_ENUM:
    {
        enum_t value = stm_read_enum(stm, enum_t);
        i_set_enum(data, bind->size, value);
        break;
    }

    case ekDTYPE_STRING:
        cassert(*dcast(data, void) == NULL);
        *dcast(data, void) = bind->props.stringp.func_read(stm);
        break;

    case ekDTYPE_STRUCT:
        i_read_struct_data(stm, data, &bind->props.structp);
        break;

    case ekDTYPE_BINARY:
        cassert(*dcast(data, void) == NULL);
        *dcast(data, void) = bind->props.binaryp.func_read(stm);
        break;

    case ekDTYPE_CONTAINER:
        cassert(*dcast(data, byte_t) == NULL);
        *dcast(data, byte_t) = i_read_container(stm, bind, ebind);
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static byte_t *i_read_container(Stream *stm, const DBind *bind, const DBind *ebind)
{
    byte_t *cont = i_create_container(bind, ebind);
    uint32_t n = stm_read_u32(stm);
    cassert_no_null(bind);
    cassert_no_null(ebind);
    if (n > 0)
    {
        byte_t *data = bind->props.contp.func_insert(cont, 0, n, tc(ebind->name), ebind->size);
        if (bind->props.contp.store_pointers == TRUE)
        {
            uint32_t i;
            bmem_set_zero(data, n * sizeofptr);
            for (i = 0; i < n; ++i)
            {
                byte_t **pdata = dcast(data, byte_t);
                bool_t nonnull = stm_read_bool(stm);
                if (nonnull == TRUE)
                {
                    switch (ebind->type)
                    {
                    case ekDTYPE_BOOL:
                    case ekDTYPE_INT:
                    case ekDTYPE_REAL:
                    case ekDTYPE_ENUM:
                    case ekDTYPE_STRUCT:
                        *pdata = i_dbind_calloc(ebind);
                        i_read_bind(stm, *pdata, ebind, NULL);
                        break;

                    case ekDTYPE_STRING:
                    case ekDTYPE_BINARY:
                        i_read_bind(stm, cast(pdata, byte_t), ebind, NULL);
                        break;

                    /* Nested containers are not allowed */
                    case ekDTYPE_CONTAINER:
                    case ekDTYPE_UNKNOWN:
                        cassert_default();
                    }
                }
                else
                {
                    *pdata = NULL;
                }

                data += sizeofptr;
            }
        }
        else
        {
            uint32_t i;
            bmem_set_zero(data, n * ebind->size);
            for (i = 0; i < n; ++i)
            {
                switch (ebind->type)
                {
                case ekDTYPE_BOOL:
                case ekDTYPE_INT:
                case ekDTYPE_REAL:
                case ekDTYPE_ENUM:
                case ekDTYPE_STRUCT:
                    i_read_bind(stm, data, ebind, NULL);
                    break;

                /* Not allowed for non-pointer cointainers */
                case ekDTYPE_STRING:
                case ekDTYPE_BINARY:
                case ekDTYPE_CONTAINER:
                case ekDTYPE_UNKNOWN:
                    cassert_default();
                }

                data += ebind->size;
            }
        }
    }

    return cont;
}

/*---------------------------------------------------------------------------*/

static void i_read_struct_data(Stream *stm, byte_t *data, const StructProps *props)
{
    cassert_no_null(props);
    arrst_foreach(member, props->members, StructMember)
        cassert_no_null(member->bind);
        switch (member->bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            i_read_bind(stm, data + member->offset, member->bind, NULL);
            break;

        case ekDTYPE_STRING:
        {
            void **dstr = dcast(data + member->offset, void);
            bool_t nonull = stm_read_bool(stm);
            cassert(*dstr == NULL);
            if (nonull == TRUE)
                i_read_bind(stm, cast(dstr, byte_t), member->bind, NULL);
            break;
        }

        case ekDTYPE_STRUCT:
            cassert(member->bind->props.structp.is_union == FALSE);
            if (member->attr.structt.is_pointer == TRUE)
            {
                byte_t **sdata = dcast(data + member->offset, byte_t);
                bool_t nonull = stm_read_bool(stm);
                cassert(*sdata == NULL);
                if (nonull == TRUE)
                {
                    *sdata = i_dbind_calloc(member->bind);
                    i_read_bind(stm, *sdata, member->bind, NULL);
                }
            }
            else
            {
                i_read_bind(stm, data + member->offset, member->bind, NULL);
            }
            break;

        case ekDTYPE_BINARY:
        {
            void **odata = dcast(data + member->offset, void);
            bool_t nonull = stm_read_bool(stm);
            cassert(*odata == NULL);
            if (nonull == TRUE)
                i_read_bind(stm, cast(odata, byte_t), member->bind, NULL);
            break;
        }

        case ekDTYPE_CONTAINER:
        {
            byte_t **ocont = dcast(data + member->offset, byte_t);
            bool_t nonull = stm_read_bool(stm);
            cassert(*ocont == NULL);
            if (nonull == TRUE)
                i_read_bind(stm, cast(ocont, byte_t), member->bind, member->attr.containert.bind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_read_imp(Stream *stm, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        byte_t *data = NULL;
        switch (bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
        case ekDTYPE_STRUCT:
            data = i_dbind_calloc(bind);
            i_read_bind(stm, data, bind, NULL);
            break;

        case ekDTYPE_STRING:
        case ekDTYPE_BINARY:
            i_read_bind(stm, cast(&data, byte_t), bind, NULL);
            break;

        case ekDTYPE_CONTAINER:
        {
            DBind *ebind = i_inner_elem_bind(bind, type);
            i_read_bind(stm, cast(&data, byte_t), bind, ebind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }

        return data;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_bool(Stream *stm, const byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BOOL);
    switch (bind->size)
    {
    case 1:
    {
        uint8_t value = *cast(data, uint8_t);
        stm_write_bool(stm, value == 0 ? FALSE : TRUE);
        break;
    }

    case 2:
    {
        uint16_t value = *cast(data, uint16_t);
        stm_write_bool(stm, value == 0 ? FALSE : TRUE);
        break;
    }

    case 4:
    {
        uint32_t value = *cast(data, uint32_t);
        stm_write_bool(stm, value == 0 ? FALSE : TRUE);
        break;
    }

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_int(Stream *stm, const byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_INT);
    switch (bind->size)
    {
    case 1:
        if (bind->props.intp.is_signed == TRUE)
            stm_write_i8(stm, *cast(data, int8_t));
        else
            stm_write_u8(stm, *cast(data, uint8_t));
        break;

    case 2:
        if (bind->props.intp.is_signed == TRUE)
            stm_write_i16(stm, *cast(data, int16_t));
        else
            stm_write_u16(stm, *cast(data, uint16_t));
        break;

    case 4:
        if (bind->props.intp.is_signed == TRUE)
            stm_write_i32(stm, *cast(data, int32_t));
        else
            stm_write_u32(stm, *cast(data, uint32_t));
        break;

    case 8:
        if (bind->props.intp.is_signed == TRUE)
            stm_write_i64(stm, *cast(data, int64_t));
        else
            stm_write_u64(stm, *cast(data, uint64_t));
        break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_real(Stream *stm, const byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_REAL);
    switch (bind->size)
    {
    case 4:
        stm_write_r32(stm, *cast(data, real32_t));
        break;

    case 8:
        stm_write_r64(stm, *cast(data, real64_t));
        break;

        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_enum(Stream *stm, const byte_t *data, const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    cassert_unref(bind->size == sizeof(enum_t), bind);
    stm_write_enum(stm, *cast(data, enum_t), enum_t);
}

/*---------------------------------------------------------------------------*/

static void i_write_bind(Stream *stm, const byte_t *data, const DBind *bind, const DBind *ebind)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        i_write_bool(stm, data, bind);
        break;

    case ekDTYPE_INT:
        i_write_int(stm, data, bind);
        break;

    case ekDTYPE_REAL:
        i_write_real(stm, data, bind);
        break;

    case ekDTYPE_ENUM:
        i_write_enum(stm, data, bind);
        break;

    case ekDTYPE_STRING:
    {
        const void *str = *dcast_const(data, void);
        bind->props.stringp.func_write(stm, str);
        break;
    }

    case ekDTYPE_STRUCT:
        i_write_struct_data(stm, data, &bind->props.structp);
        break;

    case ekDTYPE_BINARY:
    {
        const void *obj = *dcast_const(data, void);
        bind->props.binaryp.func_write(stm, obj);
        break;
    }

    case ekDTYPE_CONTAINER:
    {
        const byte_t *cont = *dcast_const(data, byte_t);
        i_write_container(stm, cont, bind, ebind);
        break;
    }

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_container(Stream *stm, const byte_t *cont, const DBind *bind, const DBind *ebind)
{
    uint32_t n = UINT32_MAX;
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    cassert_no_null(ebind);
    n = bind->props.contp.func_size(cont);
    stm_write_u32(stm, n);

    if (n > 0)
    {
        if (bind->props.contp.store_pointers == TRUE)
        {
            uint32_t i;
            for (i = 0; i < n; ++i)
            {
                const byte_t **pdata = dcast_const(bind->props.contp.func_get(cast(cont, byte_t), i, tc(ebind->name), ebind->size), byte_t);
                if (*pdata != NULL)
                {
                    stm_write_bool(stm, TRUE);
                    switch (ebind->type)
                    {
                    case ekDTYPE_BOOL:
                    case ekDTYPE_INT:
                    case ekDTYPE_REAL:
                    case ekDTYPE_ENUM:
                    case ekDTYPE_STRUCT:
                        i_write_bind(stm, *pdata, ebind, NULL);
                        break;

                    case ekDTYPE_STRING:
                    case ekDTYPE_BINARY:
                        i_write_bind(stm, cast_const(pdata, byte_t), ebind, NULL);
                        break;

                    /* Nested containers are not allowed */
                    case ekDTYPE_CONTAINER:
                    case ekDTYPE_UNKNOWN:
                        cassert_default();
                    }
                }
                else
                {
                    stm_write_bool(stm, FALSE);
                }
            }
        }
        else
        {
            uint32_t i;
            for (i = 0; i < n; ++i)
            {
                const byte_t *pdata = bind->props.contp.func_get(cast(cont, byte_t), i, tc(ebind->name), ebind->size);
                switch (ebind->type)
                {
                case ekDTYPE_BOOL:
                case ekDTYPE_INT:
                case ekDTYPE_REAL:
                case ekDTYPE_ENUM:
                case ekDTYPE_STRUCT:
                    i_write_bind(stm, pdata, ebind, NULL);
                    break;

                /* Not allowed for non-pointer cointainers */
                case ekDTYPE_STRING:
                case ekDTYPE_BINARY:
                case ekDTYPE_CONTAINER:
                case ekDTYPE_UNKNOWN:
                    cassert_default();
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_write_struct_data(Stream *stm, const byte_t *data, const StructProps *props)
{
    cassert_no_null(props);
    arrst_foreach(member, props->members, StructMember)
        cassert_no_null(member->bind);
        switch (member->bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
            i_write_bind(stm, data + member->offset, member->bind, NULL);
            break;

        case ekDTYPE_STRING:
        {
            const void **dstr = dcast_const(data + member->offset, void);
            if (*dstr != NULL)
            {
                stm_write_bool(stm, TRUE);
                i_write_bind(stm, data + member->offset, member->bind, NULL);
            }
            else
            {
                stm_write_bool(stm, FALSE);
            }
            break;
        }

        case ekDTYPE_STRUCT:
            cassert(member->bind->props.structp.is_union == FALSE);
            if (member->attr.structt.is_pointer == TRUE)
            {
                const byte_t **sdata = dcast_const(data + member->offset, byte_t);
                if (*sdata != NULL)
                {
                    stm_write_bool(stm, TRUE);
                    i_write_bind(stm, *sdata, member->bind, NULL);
                }
                else
                {
                    stm_write_bool(stm, FALSE);
                }
            }
            else
            {
                i_write_bind(stm, data + member->offset, member->bind, NULL);
            }
            break;

        case ekDTYPE_BINARY:
        {
            const void **odata = dcast_const(data + member->offset, void);
            if (*odata != NULL)
            {
                stm_write_bool(stm, TRUE);
                i_write_bind(stm, cast(odata, byte_t), member->bind, NULL);
            }
            else
            {
                stm_write_bool(stm, FALSE);
            }
            break;
        }

        case ekDTYPE_CONTAINER:
        {
            const byte_t **ocont = dcast_const(data + member->offset, byte_t);
            if (*ocont != NULL)
            {
                stm_write_bool(stm, TRUE);
                i_write_bind(stm, cast(ocont, byte_t), member->bind, member->attr.containert.bind);
            }
            else
            {
                stm_write_bool(stm, FALSE);
            }
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    arrst_end()
}

/*---------------------------------------------------------------------------*/

void dbind_write_imp(Stream *stm, const void *data, const char_t *type)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        switch (bind->type)
        {
        case ekDTYPE_BOOL:
        case ekDTYPE_INT:
        case ekDTYPE_REAL:
        case ekDTYPE_ENUM:
        case ekDTYPE_STRUCT:
            i_write_bind(stm, cast_const(data, byte_t), bind, NULL);
            break;

        case ekDTYPE_STRING:
        case ekDTYPE_BINARY:
            i_write_bind(stm, cast_const(&data, byte_t), bind, NULL);
            break;

        case ekDTYPE_CONTAINER:
        {
            DBind *ebind = i_inner_elem_bind(bind, type);
            i_write_bind(stm, cast_const(&data, byte_t), bind, ebind);
            break;
        }

        case ekDTYPE_UNKNOWN:
            cassert_default();
        }
    }
}

/*---------------------------------------------------------------------------*/

void dbind_default_imp(const char_t *type, const char_t *mname, const byte_t *value)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        if (bind->type == ekDTYPE_STRUCT)
        {
            StructMember *member = i_find_member(bind->props.structp.members, mname, UINT16_MAX, NULL, NULL);
            cassert_no_null(member);
            cassert_no_null(member->bind);
            switch (member->bind->type)
            {
            case ekDTYPE_BOOL:
                member->attr.boolt.def = i_get_bool(value, member->bind->size);
                break;

            case ekDTYPE_INT:
                member->attr.intt.def = i_get_int(value, member->bind->size, member->bind->props.intp.is_signed);
                break;

            case ekDTYPE_REAL:
                member->attr.realt.def = i_get_real(value, member->bind->size);
                break;

            case ekDTYPE_ENUM:
                member->attr.enumt.def = i_get_enum(value, member->bind->size);
                break;

            case ekDTYPE_STRING:
            {
                const char_t *str = *dcast_const(value, char_t);
                if (member->attr.stringt.def != NULL)
                    member->bind->props.stringp.func_destroy(&member->attr.stringt.def);

                if (str != NULL)
                    member->attr.stringt.def = member->bind->props.stringp.func_create(str);
                break;
            }

            case ekDTYPE_STRUCT:
            {
                const byte_t *obj = *dcast_const(value, byte_t);
                if (member->attr.structt.def != NULL)
                    i_destroy_struct_data(&member->attr.structt.def, &member->bind->props.structp, tc(member->bind->name), member->bind->size);

                if (obj != NULL)
                {
                    member->attr.structt.def = i_dbind_calloc(member->bind);
                    i_copy_struct_data(member->attr.structt.def, obj, &member->bind->props.structp);
                }
                else
                {
                    cassert(member->attr.structt.is_pointer == TRUE);
                }
                break;
            }

            case ekDTYPE_BINARY:
            {
                const void *obj = *dcast_const(value, void);
                if (member->attr.binaryt.def != NULL)
                    member->bind->props.binaryp.func_destroy(&member->attr.binaryt.def);

                if (obj != NULL)
                    member->attr.binaryt.def = member->bind->props.binaryp.func_copy(obj);
                break;
            }

            case ekDTYPE_CONTAINER:
            {
                const byte_t *cont = *dcast_const(value, byte_t);
                if (member->attr.containert.def != NULL)
                    i_destroy_container(&member->attr.containert.def, member->bind, member->attr.containert.bind);

                if (cont != NULL)
                    member->attr.containert.def = i_copy_container(cont, member->bind, member->attr.containert.bind);
                break;
            }

            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void dbind_range_imp(const char_t *type, const char_t *mname, const byte_t *min, const byte_t *max)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        if (bind->type == ekDTYPE_STRUCT)
        {
            StructMember *member = i_find_member(bind->props.structp.members, mname, UINT16_MAX, NULL, NULL);
            cassert_no_null(member);
            cassert_no_null(member->bind);
            switch (member->bind->type)
            {
            case ekDTYPE_BOOL:
                break;
            case ekDTYPE_INT:
                member->attr.intt.min = i_get_int(min, member->bind->size, member->bind->props.intp.is_signed);
                member->attr.intt.max = i_get_int(max, member->bind->size, member->bind->props.intp.is_signed);
                break;
            case ekDTYPE_REAL:
                member->attr.realt.min = i_get_real(min, member->bind->size);
                member->attr.realt.max = i_get_real(max, member->bind->size);
                break;
            case ekDTYPE_ENUM:
            case ekDTYPE_STRING:
            case ekDTYPE_STRUCT:
            case ekDTYPE_BINARY:
            case ekDTYPE_CONTAINER:
                break;
            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void dbind_precision_imp(const char_t *type, const char_t *mname, const byte_t *prec)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        if (bind->type == ekDTYPE_STRUCT)
        {
            StructMember *member = i_find_member(bind->props.structp.members, mname, UINT16_MAX, NULL, NULL);
            cassert_no_null(member);
            cassert_no_null(member->bind);
            switch (member->bind->type)
            {
            case ekDTYPE_BOOL:
            case ekDTYPE_INT:
                break;
            case ekDTYPE_REAL:
                member->attr.realt.prec = i_get_real(prec, member->bind->size);
                member->attr.realt.dec = bmath_precd(member->attr.realt.prec);
                str_destroy(&member->attr.realt.format);
                member->attr.realt.format = str_printf("%%.%df", member->attr.realt.dec);
                break;
            case ekDTYPE_ENUM:
            case ekDTYPE_STRING:
            case ekDTYPE_STRUCT:
            case ekDTYPE_BINARY:
            case ekDTYPE_CONTAINER:
                break;
            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void dbind_increment_imp(const char_t *type, const char_t *mname, const byte_t *incr)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        if (bind->type == ekDTYPE_STRUCT)
        {
            StructMember *member = i_find_member(bind->props.structp.members, mname, UINT16_MAX, NULL, NULL);
            cassert_no_null(member);
            cassert_no_null(member->bind);
            switch (member->bind->type)
            {
            case ekDTYPE_BOOL:
                break;
            case ekDTYPE_INT:
                member->attr.intt.incr = i_get_int(incr, member->bind->size, member->bind->props.intp.is_signed);
                break;
            case ekDTYPE_REAL:
                member->attr.realt.incr = i_get_real(incr, member->bind->size);
                break;
            case ekDTYPE_ENUM:
            case ekDTYPE_STRING:
            case ekDTYPE_STRUCT:
            case ekDTYPE_BINARY:
            case ekDTYPE_CONTAINER:
                break;
            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

void dbind_suffix_imp(const char_t *type, const char_t *mname, const char_t *suffix)
{
    bool_t is_pointer = FALSE;
    DBind *bind = i_dbind_from_typename(type, &is_pointer, NULL);
    cassert_unref(is_pointer == FALSE, is_pointer);
    if (bind != NULL)
    {
        if (bind->type == ekDTYPE_STRUCT)
        {
            StructMember *member = i_find_member(bind->props.structp.members, mname, UINT16_MAX, NULL, NULL);
            cassert_no_null(member);
            cassert_no_null(member->bind);
            switch (member->bind->type)
            {
            case ekDTYPE_BOOL:
                break;

            case ekDTYPE_INT:
                str_destroy(&member->attr.intt.format);
                if (str_empty_c(suffix) == TRUE)
                    member->attr.intt.format = str_printf("%%u");
                else
                    member->attr.intt.format = str_printf("%%u %s", suffix);
                break;

            case ekDTYPE_REAL:
                str_destroy(&member->attr.realt.format);
                if (str_empty_c(suffix) == TRUE)
                    member->attr.realt.format = str_printf("%%.%df", member->attr.realt.dec);
                else
                    member->attr.realt.format = str_printf("%%.%df %s", member->attr.realt.dec, suffix);
                break;

            case ekDTYPE_ENUM:
            case ekDTYPE_STRING:
            case ekDTYPE_STRUCT:
            case ekDTYPE_BINARY:
            case ekDTYPE_CONTAINER:
                break;
            case ekDTYPE_UNKNOWN:
                cassert_default();
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

static ___INLINE StructMember *i_member(const DBind *bind, const uint32_t member_id)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_STRUCT);
    return arrst_get(bind->props.structp.members, member_id, StructMember);
}

/*---------------------------------------------------------------------------*/

const DBind *dbind_from_typename(const char_t *type, bool_t *is_ptr)
{
    bool_t is_pointer = FALSE;
    uint32_t alias_id = UINT32_MAX;
    const DBind *bind = i_dbind_from_typename(type, &is_pointer, &alias_id);
    ptr_assign(is_ptr, is_pointer);
    return bind;
}

/*---------------------------------------------------------------------------*/

dtype_t dbind_type(const DBind *bind)
{
    cassert_no_null(bind);
    return bind->type;
}

/*---------------------------------------------------------------------------*/

uint16_t dbind_size(const DBind *bind)
{
    cassert_no_null(bind);
    return bind->size;
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_typename(const DBind *bind)
{
    cassert_no_null(bind);
    return tc(bind->name);
}

/*---------------------------------------------------------------------------*/

uint32_t dbind_enum_count(const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    return arrst_size(bind->props.enump.members, EnumMember);
}

/*---------------------------------------------------------------------------*/

uint32_t dbind_enum_index(const DBind *bind, const enum_t value)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    arrst_foreach(emember, bind->props.enump.members, EnumMember)
        if (emember->value == value)
            return emember_i;
    arrst_end()
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

enum_t dbind_enum_index_value(const DBind *bind, const uint32_t index)
{
    const EnumMember *emember = NULL;
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
    return emember->value;
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_enum_alias(const DBind *bind, const uint32_t index)
{
    const EnumMember *emember = NULL;
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
    return tc(emember->alias);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_container_is_ptr(const DBind *bind)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    return bind->props.contp.store_pointers;
}

/*---------------------------------------------------------------------------*/

const DBind *dbind_container_type(const DBind *bind, const char_t *type)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    return i_inner_elem_bind(bind, type);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_is_basic_type(const dtype_t type)
{
    switch (type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
        return TRUE;
    case ekDTYPE_STRUCT:
    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
        return FALSE;
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

bool_t dbind_is_number_type(const dtype_t type)
{
    switch (type)
    {
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
        return TRUE;
    case ekDTYPE_BOOL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_STRUCT:
    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
        return FALSE;
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

const DBind *dbind_st_member(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return member->bind;
}

/*---------------------------------------------------------------------------*/

const DBind *dbind_st_ebind(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    if (member->bind->type == ekDTYPE_CONTAINER)
        return member->attr.containert.bind;
    return NULL;
}

/*---------------------------------------------------------------------------*/

uint32_t dbind_st_count(const DBind *stbind)
{
    cassert_no_null(stbind);
    cassert(stbind->type == ekDTYPE_STRUCT);
    return arrst_size(stbind->props.structp.members, StructMember);
}

/*---------------------------------------------------------------------------*/

uint32_t dbind_st_member_id(const DBind *stbind, const char_t *mname)
{
    cassert_no_null(stbind);
    cassert(stbind->type == ekDTYPE_STRUCT);
    arrst_foreach_const(member, stbind->props.structp.members, StructMember)
        if (str_equ(member->name, mname) == TRUE)
            return member_i;
    arrst_end()
    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

uint16_t dbind_st_offset(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return member->offset;
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_st_mname(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return tc(member->name);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_st_is_str_dptr(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    if (member->bind->type == ekDTYPE_STRUCT)
        return member->attr.structt.is_pointer;
    else
        return FALSE;
}

/*---------------------------------------------------------------------------*/

void dbind_st_int_range(const DBind *stbind, const uint32_t member_id, int64_t *min, int64_t *max)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_INT);
    ptr_assign(min, member->attr.intt.min);
    ptr_assign(max, member->attr.intt.max);
}

/*---------------------------------------------------------------------------*/

void dbind_st_real_range(const DBind *stbind, const uint32_t member_id, real64_t *min, real64_t *max)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_REAL);
    ptr_assign(min, member->attr.realt.min);
    ptr_assign(max, member->attr.realt.max);
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_st_real_format(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_REAL);
    return tc(member->attr.realt.format);
}

/*---------------------------------------------------------------------------*/

const void *dbind_st_binary_default(const DBind *stbind, const uint32_t member_id)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_BINARY);
    return member->attr.binaryt.def;
}

/*---------------------------------------------------------------------------*/

bool_t dbind_st_str_filter(const DBind *stbind, const uint32_t member_id, const char_t *src, char_t *dest, const uint32_t size)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    switch (member->bind->type)
    {
    case ekDTYPE_INT:
        _tfilter_number(src, dest, size, 0, member->bind->props.intp.is_signed);
        return TRUE;

    case ekDTYPE_REAL:
        _tfilter_number(src, dest, size, member->attr.realt.dec, (bool_t)(member->attr.realt.min < 0));
        return TRUE;

    case ekDTYPE_BOOL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        str_copy_c(dest, size, src);
        return FALSE;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_create_data(const DBind *bind, const DBind *ebind)
{
    return i_create(bind, ebind);
}

/*---------------------------------------------------------------------------*/

void dbind_init_data(const DBind *bind, byte_t *data)
{
    cassert_no_null(data);
    bmem_set_zero(data, bind->size);
    i_init_bind(data, bind);
}

/*---------------------------------------------------------------------------*/

void dbind_remove_data(byte_t *data, const DBind *bind)
{
    i_remove_data(data, bind);
}

/*---------------------------------------------------------------------------*/

void dbind_destroy_data(byte_t **data, const DBind *bind, const DBind *ebind)
{
    i_destroy_data(data, bind, ebind);
}

/*---------------------------------------------------------------------------*/

void dbind_st_store_field(const DBind *stbind, const uint32_t member_id, const byte_t *obj, byte_t *dest, const uint32_t dest_size)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert_no_null(obj);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
        cassert_unref(dest_size >= member->bind->size, dest_size);
        bmem_copy(dest, obj + member->offset, member->bind->size);
        break;

    case ekDTYPE_STRING:
    {
        const void *src_data = *dcast_const(obj + member->offset, void);
        void **dest_data = dcast(dest, void);

        cassert_unref(dest_size >= sizeofptr, dest_size);

        if (*dest_data != NULL)
            member->bind->props.stringp.func_destroy(dest_data);

        if (src_data != NULL)
        {
            const char_t *value = member->bind->props.stringp.func_get(src_data);
            *dest_data = member->bind->props.stringp.func_create(value);
        }
        else
        {
            *dest_data = NULL;
        }
        break;
    }

    case ekDTYPE_BINARY:
    {
        const void *src_data = *dcast_const(obj + member->offset, void);
        void **dest_data = dcast(dest, void);

        cassert_unref(dest_size >= sizeofptr, dest_size);

        if (*dest_data != NULL)
            member->bind->props.binaryp.func_destroy(dest_data);

        if (src_data != NULL)
            *dest_data = member->bind->props.binaryp.func_copy(src_data);
        else
            *dest_data = NULL;
        break;
    }

    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void dbind_st_restore_field(const DBind *stbind, const uint32_t member_id, byte_t *obj, const byte_t *src, const uint32_t src_size)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert_no_null(obj);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
        cassert_unref(src_size >= member->bind->size, src_size);
        bmem_copy(obj + member->offset, src, member->bind->size);
        break;

    case ekDTYPE_STRING:
    {
        const void *src_data = *dcast_const(src, void);
        void **dest_data = dcast(obj + member->offset, void);
        cassert_unref(src_size >= sizeofptr, src_size);

        if (*dest_data != NULL)
            member->bind->props.stringp.func_destroy(dest_data);

        if (src_data != NULL)
        {
            const char_t *value = member->bind->props.stringp.func_get(src_data);
            *dest_data = member->bind->props.stringp.func_create(value);
        }
        else
        {
            *dest_data = NULL;
        }
        break;
    }

    case ekDTYPE_BINARY:
    {
        const void *src_data = *dcast_const(src, void);
        void **dest_data = dcast(obj + member->offset, void);
        cassert_unref(src_size >= sizeofptr, src_size);

        if (*dest_data != NULL)
            member->bind->props.binaryp.func_destroy(dest_data);

        if (src_data != NULL)
            *dest_data = member->bind->props.binaryp.func_copy(src_data);
        else
            *dest_data = NULL;
        break;
    }

    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void dbind_st_remove_field(const DBind *stbind, const uint32_t member_id, byte_t *dest, const uint32_t dest_size)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
        break;

    case ekDTYPE_STRING:
        cassert_unref(dest_size >= member->bind->size, dest_size);
        if (*dcast(dest, void) != NULL)
            member->bind->props.stringp.func_destroy(dcast(dest, void));
        break;

    case ekDTYPE_BINARY:
        cassert_unref(dest_size >= member->bind->size, dest_size);
        if (*dcast(dest, void) != NULL)
            member->bind->props.binaryp.func_destroy(dcast(dest, void));
        break;

    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
    case ekDTYPE_UNKNOWN:
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

bool_t dbind_st_overlaps_field(const DBind *stbind, const uint32_t member_id, const byte_t *obj, const byte_t *data, const uint32_t data_size)
{
    StructMember *member = i_member(stbind, member_id);
    const byte_t *stmem = NULL;
    uint32_t stsize = UINT32_MAX;
    cassert_no_null(member);
    cassert_no_null(member->bind);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
    case ekDTYPE_INT:
    case ekDTYPE_REAL:
    case ekDTYPE_ENUM:
    case ekDTYPE_STRING:
    case ekDTYPE_BINARY:
    case ekDTYPE_CONTAINER:
        stmem = obj + member->offset;
        stsize = member->bind->size;
        break;

    case ekDTYPE_STRUCT:
        if (member->attr.structt.is_pointer == TRUE)
            stmem = *dcast(obj + member->offset, byte_t);
        else
            stmem = obj + member->offset;
        stsize = member->bind->size;
        break;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return bmem_overlaps(stmem, data, stsize, data_size);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_get_bool_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BOOL);
    cassert_no_null(data);
    return i_get_bool(data, bind->size);
}

/*---------------------------------------------------------------------------*/

bool_t dbind_st_get_bool_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_BOOL);
    cassert_no_null(obj);
    return i_get_bool(obj + member->offset, member->bind->size);
}

/*---------------------------------------------------------------------------*/

int64_t dbind_get_int_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_INT);
    cassert_no_null(data);
    return i_get_int(data, bind->size, bind->props.intp.is_signed);
}

/*---------------------------------------------------------------------------*/

int64_t dbind_st_get_int_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_INT);
    cassert_no_null(obj);
    return i_get_int(obj + member->offset, member->bind->size, member->bind->props.intp.is_signed);
}

/*---------------------------------------------------------------------------*/

real64_t dbind_get_real_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_REAL);
    cassert_no_null(data);
    return i_get_real(data, bind->size);
}

/*---------------------------------------------------------------------------*/

real64_t dbind_st_get_real_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_REAL);
    cassert_no_null(obj);
    return i_get_real(obj + member->offset, member->bind->size);
}

/*---------------------------------------------------------------------------*/

enum_t dbind_get_enum_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_ENUM);
    cassert_no_null(data);
    return i_get_enum(data, bind->size);
}

/*---------------------------------------------------------------------------*/

enum_t dbind_st_get_enum_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_ENUM);
    cassert_no_null(obj);
    return i_get_enum(obj + member->offset, member->bind->size);
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_get_str_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_STRING);
    cassert_no_null(data);
    return bind->props.stringp.func_get(data);
}

/*---------------------------------------------------------------------------*/

const char_t *dbind_st_get_str_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    void *str = NULL;
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_STRING);
    cassert_no_null(obj);
    str = *dcast(obj + member->offset, void);
    if (str != NULL)
        return member->bind->props.stringp.func_get(str);
    return NULL;
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_st_get_struct_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_STRUCT);
    cassert_no_null(obj);
    if (member->attr.structt.is_pointer == TRUE)
        return *dcast(obj + member->offset, byte_t);
    else
        return cast(obj + member->offset, byte_t);
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_st_create_struct_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    byte_t **sobj = dcast(obj + member->offset, byte_t);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_STRUCT);
    cassert_no_null(obj);
    cassert(*sobj == NULL);
    *sobj = dbind_create_data(member->bind, NULL);
    return *sobj;
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_st_get_container_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_CONTAINER);
    cassert_no_null(obj);
    return *dcast(obj + member->offset, byte_t);
}

/*---------------------------------------------------------------------------*/

const void *dbind_get_binary_value(const DBind *bind, const byte_t *data)
{
    cassert_no_null(bind);
    cassert_unref(bind->type == ekDTYPE_BINARY, bind);
    return cast_const(data, void);
}

/*---------------------------------------------------------------------------*/

const void *dbind_st_get_binary_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_BINARY);
    cassert_no_null(obj);
    return *dcast(obj + member->offset, void);
}

/*---------------------------------------------------------------------------*/

void dbind_write_binary_value(const DBind *bind, Stream *stm, const byte_t *data)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BINARY);
    cassert_no_null(data);
    bind->props.binaryp.func_write(stm, data);
}

/*---------------------------------------------------------------------------*/

void dbind_st_write_binary_value(const DBind *stbind, const uint32_t member_id, Stream *stm, const byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    void *data = NULL;
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(member->bind->type == ekDTYPE_BINARY);
    cassert_no_null(obj);
    data = *dcast(obj + member->offset, void);
    if (data != NULL)
        member->bind->props.binaryp.func_write(stm, data);
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_bool(byte_t *data, const uint16_t size, const bool_t value)
{
    bool_t current = i_get_bool(data, size);
    if (current != value)
    {
        i_set_bool(data, size, value);
        return ekBINDSET_OK;
    }

    return ekBINDSET_UNCHANGED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_int(byte_t *data, const uint16_t size, const bool_t is_signed, const int64_t value)
{
    int64_t current = i_get_int(data, size, is_signed);
    if (current != value)
    {
        i_set_int(data, size, is_signed, value);
        return ekBINDSET_OK;
    }

    return ekBINDSET_UNCHANGED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_real(byte_t *data, const uint16_t size, const real64_t value)
{
    real64_t current = i_get_real(data, size);
    if (bmath_absd(current - value) > i_EPSILON)
    {
        i_set_real(data, size, value);
        return ekBINDSET_OK;
    }

    return ekBINDSET_UNCHANGED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_enum(byte_t *data, const uint16_t size, const enum_t value)
{
    enum_t current = i_get_enum(data, size);
    if (current != value)
    {
        i_set_enum(data, size, value);
        return ekBINDSET_OK;
    }

    return ekBINDSET_UNCHANGED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_string(const DBind *bind, byte_t *data, const char_t *value)
{
    void **current = dcast(data, void);
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_STRING);

    if (value == NULL)
        value = "";

    if (*current != NULL)
    {
        const char_t *str = bind->props.stringp.func_get(*current);
        if (str_equ_c(str, value) == FALSE)
        {
            bind->props.stringp.func_destroy(current);
            *current = bind->props.stringp.func_create(value);
            return ekBINDSET_OK;
        }
        else
        {
            return ekBINDSET_UNCHANGED;
        }
    }
    else
    {
        *current = bind->props.stringp.func_create(value);
        return ekBINDSET_OK;
    }
}

/*---------------------------------------------------------------------------*/

static bindset_t i_update_binary(const DBind *bind, byte_t *data, const void *value)
{
    void **current = dcast(data, void);
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BINARY);
    if (*current != value)
    {
        if (*current != NULL)
        {
            bind->props.binaryp.func_destroy(current);
            *current = NULL;
        }

        if (value != NULL)
            *current = bind->props.binaryp.func_copy(value);

        return ekBINDSET_OK;
    }
    else
    {
        return ekBINDSET_UNCHANGED;
    }
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_null(const DBind *bind, const DBind *ebind, const bool_t is_str_dptr, byte_t *data)
{
    cassert_no_null(bind);
    unref(ebind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(data, bind->size, FALSE);

    case ekDTYPE_INT:
        return i_update_int(data, bind->size, bind->props.intp.is_signed, 0);

    case ekDTYPE_REAL:
        return i_update_real(data, bind->size, 0);

    case ekDTYPE_ENUM:
    {
        uint32_t index = 0;
        if (index < arrst_size(bind->props.enump.members, EnumMember))
        {
            const EnumMember *emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
            return i_update_enum(data, bind->size, emember->value);
        }
        else
        {
            return i_update_enum(data, bind->size, ENUM_MAX(enum_t));
        }
    }

    case ekDTYPE_STRING:
        if (*dcast(data, void) != NULL)
        {
            bind->props.stringp.func_destroy(dcast(data, void));
            return ekBINDSET_OK;
        }
        else
        {
            return ekBINDSET_UNCHANGED;
        }

    case ekDTYPE_STRUCT:
        if (is_str_dptr == TRUE)
            i_destroy_struct_data(dcast(data, byte_t), &bind->props.structp, tc(bind->name), bind->size);
        else
            i_remove_struct_data(data, &bind->props.structp);
        return ekBINDSET_OK;

    case ekDTYPE_CONTAINER:
        if (*dcast(data, byte_t) != NULL)
        {
            i_destroy_container(dcast(data, byte_t), bind, ebind);
            return ekBINDSET_OK;
        }
        else
        {
            return ekBINDSET_UNCHANGED;
        }

    case ekDTYPE_BINARY:
        if (*dcast(data, void) != NULL)
        {
            bind->props.binaryp.func_destroy(dcast(data, void));
            return ekBINDSET_OK;
        }
        else
        {
            return ekBINDSET_UNCHANGED;
        }

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_null(const DBind *stbind, const uint32_t member_id, byte_t *obj)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert_no_null(obj);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(obj + member->offset, member->bind->size, FALSE);

    case ekDTYPE_INT:
        return i_update_int(obj + member->offset, member->bind->size, member->bind->props.intp.is_signed, 0);

    case ekDTYPE_REAL:
        return i_update_real(obj + member->offset, member->bind->size, 0);

    case ekDTYPE_ENUM:
    {
        uint32_t index = 0;
        if (index < arrst_size(member->bind->props.enump.members, EnumMember))
        {
            const EnumMember *emember = arrst_get_const(member->bind->props.enump.members, index, EnumMember);
            return i_update_enum(obj + member->offset, member->bind->size, emember->value);
        }
        else
        {
            return i_update_enum(obj + member->offset, member->bind->size, ENUM_MAX(enum_t));
        }
    }

    case ekDTYPE_STRING:
        if (*dcast(obj + member->offset, void) != NULL)
            member->bind->props.stringp.func_destroy(dcast(obj + member->offset, void));
        return ekBINDSET_OK;

    case ekDTYPE_BINARY:
        if (*dcast(obj + member->offset, void) != NULL)
            member->bind->props.binaryp.func_destroy(dcast(obj + member->offset, void));
        return ekBINDSET_OK;

    case ekDTYPE_STRUCT:
        if (member->attr.structt.is_pointer == TRUE)
        {
            if (*dcast(obj + member->offset, byte_t) != NULL)
                i_destroy_struct_data(dcast(obj + member->offset, byte_t), &member->bind->props.structp, tc(member->bind->name), member->bind->size);
        }
        else
        {
            i_remove_struct_data(obj + member->offset, &member->bind->props.structp);
        }
        return ekBINDSET_OK;

    case ekDTYPE_CONTAINER:
        if (*dcast(obj + member->offset, byte_t) != NULL)
            i_destroy_container(dcast(obj + member->offset, byte_t), member->bind, member->attr.containert.bind);
        return ekBINDSET_OK;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_set_value_bool(const DBind *bind, byte_t *data, const bool_t value)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(data, bind->size, value);

    case ekDTYPE_INT:
        return i_update_int(data, bind->size, bind->props.intp.is_signed, value == TRUE ? 1 : 0);

    case ekDTYPE_REAL:
        return i_update_real(data, bind->size, value == TRUE ? 1 : 0);

    case ekDTYPE_ENUM:
    {
        uint32_t index = value == TRUE ? 1 : 0;
        if (index < arrst_size(bind->props.enump.members, EnumMember))
        {
            const EnumMember *emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
            return i_update_enum(data, bind->size, emember->value);
        }
        else
        {
            return i_update_enum(data, bind->size, ENUM_MAX(enum_t));
        }
    }

    case ekDTYPE_STRING:
        return i_update_string(bind, data, value == TRUE ? "true" : "false");

    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_bool(const DBind *bind, byte_t *data, const bool_t value)
{
    return i_set_value_bool(bind, data, value);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_bool(const DBind *stbind, const uint32_t member_id, byte_t *obj, const bool_t value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return i_set_value_bool(member->bind, obj + member->offset, value);
}

/*---------------------------------------------------------------------------*/

static bindset_t i_set_value_int(const DBind *bind, byte_t *data, const int64_t value, const StructMember *member)
{
    cassert_no_null(bind);
    cassert(member == NULL || member->bind == bind);

    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(data, bind->size, value == 0 ? FALSE : TRUE);

    case ekDTYPE_INT:
    {
        int64_t cvalue = value;
        if (member != NULL)
            cvalue = i_member_clamp_int(member, cvalue);
        return i_update_int(data, bind->size, bind->props.intp.is_signed, cvalue);
    }

    case ekDTYPE_REAL:
    {
        real64_t cvalue = (real64_t)value;
        if (member != NULL)
            cvalue = i_member_clamp_real(member, cvalue);
        return i_update_real(data, bind->size, cvalue);
    }

    case ekDTYPE_ENUM:
    {
        uint32_t index = (uint32_t)value;
        if (index < arrst_size(bind->props.enump.members, EnumMember))
        {
            const EnumMember *emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
            return i_update_enum(data, bind->size, emember->value);
        }
        else
        {
            return i_update_enum(data, bind->size, ENUM_MAX(enum_t));
        }
    }

    case ekDTYPE_STRING:
    {
        char_t str[32];
        bstd_sprintf(str, sizeof(str), "%" PRId64, value);
        return i_update_string(bind, data, str);
    }

    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_int(const DBind *bind, byte_t *data, const int64_t value)
{
    return i_set_value_int(bind, data, value, NULL);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_int(const DBind *stbind, const uint32_t member_id, byte_t *obj, const int64_t value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return i_set_value_int(member->bind, obj + member->offset, value, member);
}

/*---------------------------------------------------------------------------*/

static bindset_t i_set_value_real(const DBind *bind, byte_t *data, const real64_t value, const StructMember *member)
{
    cassert_no_null(bind);
    cassert(member == NULL || member->bind == bind);

    switch (bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(data, bind->size, bmath_absd(value) < i_EPSILON ? FALSE : TRUE);

    case ekDTYPE_INT:
    {
        int64_t cvalue = (int64_t)bmath_roundd(value);
        if (member != NULL)
            cvalue = i_member_clamp_int(member, cvalue);
        return i_update_int(data, bind->size, bind->props.intp.is_signed, cvalue);
    }

    case ekDTYPE_REAL:
    {
        real64_t cvalue = value;
        if (member != NULL)
            cvalue = i_member_clamp_real(member, cvalue);
        return i_update_real(data, bind->size, cvalue);
    }

    case ekDTYPE_ENUM:
    {
        uint32_t index = (uint32_t)bmath_roundd(value);
        if (index < arrst_size(bind->props.enump.members, EnumMember))
        {
            const EnumMember *emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
            return i_update_enum(data, bind->size, emember->value);
        }
        else
        {
            return i_update_enum(data, bind->size, ENUM_MAX(enum_t));
        }
    }

    case ekDTYPE_STRING:
    {
        char_t str[32];
        bstd_sprintf(str, sizeof(str), "%g", value);
        return i_update_string(bind, data, str);
    }

    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_real(const DBind *bind, byte_t *data, const real64_t value)
{
    return i_set_value_real(bind, data, value, NULL);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_real(const DBind *stbind, const uint32_t member_id, byte_t *obj, const real64_t value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return i_set_value_real(member->bind, obj + member->offset, value, member);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_norm(const DBind *stbind, const uint32_t member_id, byte_t *obj, const real64_t norm_value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    cassert(norm_value >= 0 && norm_value <= 1);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
        return i_update_bool(obj + member->offset, member->bind->size, norm_value > .5 ? TRUE : FALSE);

    case ekDTYPE_INT:
    {
        real64_t min = (real64_t)member->attr.intt.min;
        real64_t max = (real64_t)member->attr.intt.max;
        real64_t cvalue = min + (norm_value * (max - min));
        return i_update_int(obj + member->offset, member->bind->size, member->bind->props.intp.is_signed, (int64_t)cvalue);
    }

    case ekDTYPE_REAL:
    {
        real64_t min = member->attr.realt.min;
        real64_t max = member->attr.realt.max;
        real64_t cvalue = min + (norm_value * (max - min));
        return i_update_real(obj + member->offset, member->bind->size, cvalue);
    }

    case ekDTYPE_ENUM:
    {
        uint32_t count = arrst_size(member->bind->props.enump.members, EnumMember);
        uint32_t index = (uint32_t)bmath_roundd(norm_value * (count - 1));
        const EnumMember *emember = arrst_get_const(member->bind->props.enump.members, index, EnumMember);
        return i_update_enum(obj + member->offset, member->bind->size, emember->value);
    }

    case ekDTYPE_STRING:
    {
        char_t str[32];
        bstd_sprintf(str, sizeof(str), "%f", norm_value);
        return i_update_string(member->bind, obj + member->offset, str);
    }

    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_incr(const DBind *stbind, const uint32_t member_id, byte_t *obj, const bool_t positive_incr)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    switch (member->bind->type)
    {
    case ekDTYPE_BOOL:
    {
        bool_t cvalue = i_get_bool(obj + member->offset, member->bind->size);
        return i_update_bool(obj + member->offset, member->bind->size, !cvalue);
    }

    case ekDTYPE_INT:
    {
        int64_t cvalue = i_get_int(obj + member->offset, member->bind->size, member->bind->props.intp.is_signed);
        if (positive_incr == TRUE)
            cvalue += member->attr.intt.incr;
        else
            cvalue -= member->attr.intt.incr;
        cvalue = i_member_clamp_int(member, cvalue);
        return i_update_int(obj + member->offset, member->bind->size, member->bind->props.intp.is_signed, cvalue);
    }

    case ekDTYPE_REAL:
    {
        real64_t cvalue = i_get_real(obj + member->offset, member->bind->size);
        if (positive_incr == TRUE)
            cvalue += member->attr.realt.incr;
        else
            cvalue -= member->attr.realt.incr;
        cvalue = i_member_clamp_real(member, cvalue);
        return i_update_real(obj + member->offset, member->bind->size, cvalue);
    }

    case ekDTYPE_ENUM:
    {
        enum_t cvalue = i_get_enum(obj + member->offset, member->bind->size);
        uint32_t index = dbind_enum_index(member->bind, cvalue);
        uint32_t count = arrst_size(member->bind->props.enump.members, EnumMember);
        const EnumMember *emember = NULL;

        if (positive_incr == TRUE)
        {
            if (index == count - 1)
                index = 0;
            else
                index++;
        }
        else
        {
            if (index == 0)
                index = count - 1;
            else
                index--;
        }

        emember = arrst_get_const(member->bind->props.enump.members, index, EnumMember);
        return i_update_enum(obj + member->offset, member->bind->size, emember->value);
    }

    case ekDTYPE_STRING:
    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_set_value_str(const DBind *bind, byte_t *data, const char_t *value, const StructMember *member)
{
    cassert_no_null(bind);
    switch (bind->type)
    {
    case ekDTYPE_BOOL:
    {
        bool_t cvalue = str_equ_nocase(value, "true") || str_equ_nocase(value, "yes") || str_equ_nocase(value, "on") || str_equ_nocase(value, "y");
        return i_update_bool(data, bind->size, cvalue);
    }

    case ekDTYPE_INT:
    {
        bool_t error = FALSE;
        int64_t cvalue = str_to_i64(value, 10, &error);

        if (error == TRUE)
            cvalue = str_to_i64(value, 16, &error);

        if (error == FALSE)
        {
            if (member != NULL)
                cvalue = i_member_clamp_int(member, cvalue);
            return i_update_int(data, bind->size, bind->props.intp.is_signed, cvalue);
        }
        else
        {
            return ekBINDSET_NOT_ALLOWED;
        }
    }

    case ekDTYPE_REAL:
    {
        real64_t cvalue = str_to_r64(value, NULL);
        if (member != NULL)
            cvalue = i_member_clamp_real(member, cvalue);
        return i_update_real(data, bind->size, cvalue);
    }

    case ekDTYPE_ENUM:
    {
        const EnumMember *emember = NULL;
        arrst_foreach(imember, bind->props.enump.members, EnumMember)
            if (str_equ(imember->alias, value) == TRUE)
            {
                emember = imember;
                break;
            }
        arrst_end()

        if (emember == NULL)
        {
            bool_t err = FALSE;
            uint32_t index = (uint32_t)bmath_roundd(str_to_r64(value, &err));
            if (err == FALSE && index < arrst_size(bind->props.enump.members, EnumMember))
                emember = arrst_get_const(bind->props.enump.members, index, EnumMember);
        }

        if (emember != NULL)
            return i_update_enum(data, bind->size, emember->value);
        else
            return i_update_enum(data, bind->size, ENUM_MAX(enum_t));
    }

    case ekDTYPE_STRING:
        return i_update_string(bind, data, value);

    case ekDTYPE_BINARY:
    case ekDTYPE_STRUCT:
    case ekDTYPE_CONTAINER:
        return ekBINDSET_NOT_ALLOWED;

    case ekDTYPE_UNKNOWN:
        cassert_default();
    }

    return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_str(const DBind *bind, byte_t *data, const char_t *value)
{
    return i_set_value_str(bind, data, value, NULL);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_str(const DBind *stbind, const uint32_t member_id, byte_t *obj, const char_t *value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    return i_set_value_str(member->bind, obj + member->offset, value, member);
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_set_value_binary(const DBind *bind, byte_t *data, const void *value)
{
    cassert_no_null(bind);
    if (bind->type == ekDTYPE_BINARY)
        return i_update_binary(bind, data, value);
    else
        return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_set_value_binary(const DBind *stbind, const uint32_t member_id, byte_t *obj, const void *value)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    if (member->bind->type == ekDTYPE_BINARY)
        return i_update_binary(member->bind, obj + member->offset, value);
    else
        return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

static bindset_t i_create_binary(const DBind *bind, byte_t **data, const byte_t *bindata, const uint32_t binsize)
{
    bindset_t ret = ekBINDSET_NOT_ALLOWED;
    Stream *stm = NULL;
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_BINARY);
    stm = stm_from_block(bindata, binsize);
    if (stm != NULL)
    {
        void *obj = bind->props.binaryp.func_read(stm);
        if (obj != NULL)
        {
            void **dest = dcast(data, void);
            if (*dest != NULL)
                bind->props.binaryp.func_destroy(dest);
            *dest = obj;
            ret = ekBINDSET_OK;
        }
        stm_close(&stm);
    }
    return ret;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_create_value_binary(const DBind *bind, byte_t *data, const byte_t *bindata, const uint32_t binsize)
{
    cassert_no_null(bind);
    if (bind->type == ekDTYPE_BINARY)
        return i_create_binary(bind, dcast(data, byte_t), bindata, binsize);
    else
        return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

bindset_t dbind_st_create_value_binary(const DBind *stbind, const uint32_t member_id, byte_t *obj, const byte_t *bindata, const uint32_t binsize)
{
    StructMember *member = i_member(stbind, member_id);
    cassert_no_null(member);
    cassert_no_null(member->bind);
    if (member->bind->type == ekDTYPE_BINARY)
        return i_create_binary(member->bind, dcast(obj + member->offset, byte_t), bindata, binsize);
    else
        return ekBINDSET_NOT_ALLOWED;
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_container_create(const DBind *bind, const DBind *ebind)
{
    return i_create_container(bind, ebind);
}

/*---------------------------------------------------------------------------*/

uint32_t dbind_container_size(const DBind *bind, const byte_t *cont)
{
    cassert_no_null(bind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    return bind->props.contp.func_size(cont);
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_container_append(const DBind *bind, const DBind *ebind, byte_t *cont)
{
    cassert_no_null(bind);
    cassert_no_null(ebind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    return bind->props.contp.func_insert(cont, UINT32_MAX, 1, tc(ebind->name), bind->props.contp.store_pointers ? ebind->size : sizeofptr);
}

/*---------------------------------------------------------------------------*/

byte_t *dbind_container_get(const DBind *bind, const DBind *ebind, const uint32_t pos, byte_t *cont)
{
    byte_t *elem = NULL;
    cassert_no_null(bind);
    cassert_no_null(ebind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    elem = bind->props.contp.func_get(cont, pos, tc(ebind->name), bind->props.contp.store_pointers ? ebind->size : sizeofptr);
    if (bind->props.contp.store_pointers == TRUE)
        elem = *dcast(elem, byte_t);
    return elem;
}

/*---------------------------------------------------------------------------*/

const byte_t *dbind_container_cget(const DBind *bind, const DBind *ebind, const uint32_t pos, const byte_t *cont)
{
    return dbind_container_get(bind, ebind, pos, cast(cont, byte_t));
}

/*---------------------------------------------------------------------------*/

void dbind_container_delete(const DBind *bind, const DBind *ebind, const uint32_t pos, byte_t *cont)
{
    cassert_no_null(bind);
    cassert_no_null(ebind);
    cassert(bind->type == ekDTYPE_CONTAINER);
    bind->props.contp.func_delete(cont, pos, tc(ebind->name), bind->props.contp.store_pointers ? ebind->size : sizeofptr);
}
