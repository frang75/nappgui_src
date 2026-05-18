/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: respack.c
 *
 */

/* Resource Packages */

#include "respack.h"
#include "respackh.h"
#include "arrpt.h"
#include "arrst.h"
#include "buffer.h"
#include "heap.h"
#include "hfile.h"
#include "strings.h"
#include <osbs/bfile.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

#if defined(__ASSERTS__)
#include <sewer/bstd.h>
#endif

typedef struct i_resource_t i_Resource;
typedef struct i_parser_t i_Parser;

enum i_type_t
{
    i_ekTYPE_EMBEDDED = 0,
    i_ekTYPE_PACKED = 1
};

struct i_resource_t
{
#if defined(__ASSERTS__)
    const char_t *id;
#endif
    uint32_t type;
    const byte_t *data;
    uint32_t size;
    void *object;
    FPtr_destroy func_destroy;
};

struct _respack
{
    enum i_type_t type;
    String *name;
    Buffer *buffer;
    ArrSt(i_Resource) *resources;
};

struct i_parser_t
{
    const byte_t *cur;
    const byte_t *end;
};

DeclSt(i_Resource);

/*---------------------------------------------------------------------------*/

static void i_init_resource(
    i_Resource *resource,
    const uint32_t type,
    const byte_t *data,
    const uint32_t size,
    void **object)
{
    cassert_no_null(resource);
#if defined(__ASSERTS__)
    resource->id = NULL;
#endif
    resource->type = type;
    resource->data = data;
    resource->size = size;
    resource->object = ptr_dget(object, void);
    resource->func_destroy = NULL;
}

/*---------------------------------------------------------------------------*/

static void i_remove_resource(i_Resource *resource)
{
    cassert_no_null(resource);
    if (resource->object != NULL)
    {
        cassert_no_nullf(resource->func_destroy);
        resource->func_destroy(&resource->object);
    }
}

/*---------------------------------------------------------------------------*/

static ResPack *i_create_respack(const enum i_type_t type, String **name, Buffer **buffer, ArrSt(i_Resource) **resources)
{
    ResPack *pack = heap_new(ResPack);
    pack->type = type;
    pack->name = ptr_dget_no_null(name, String);
    pack->buffer = ptr_dget(buffer, Buffer);
    pack->resources = ptr_dget_no_null(resources, ArrSt(i_Resource));
    return pack;
}

/*---------------------------------------------------------------------------*/

#if defined(__ASSERTS__)
static bool_t i_dump_resources(const ArrSt(i_Resource) *resources, const bool_t dump)
{
    if (dump == TRUE)
    {
        arrst_foreach_const(res, resources, i_Resource)
            if (res->type == 1)
                bstd_printf("ResType: %d, ID: %s, Object:%p, Size:%d\n", res->type, res->id, res->object, res->size);
        arrst_end()
    }

    return TRUE;
}
#endif

/*---------------------------------------------------------------------------*/

void respack_destroy(ResPack **pack)
{
    if (pack == NULL || *pack == NULL)
        return;

    cassert(i_dump_resources((*pack)->resources, FALSE) == TRUE);
    str_destroy(&(*pack)->name);
    arrst_destroy(&(*pack)->resources, i_remove_resource, i_Resource);
    if ((*pack)->type == i_ekTYPE_PACKED)
        buffer_destroy(&(*pack)->buffer);
    else
        cassert((*pack)->buffer == NULL);
    heap_delete(pack, ResPack);
}

/*---------------------------------------------------------------------------*/

ResPack *respack_embedded(const char_t *name)
{
    String *lname = str_c(name);
    Buffer *buffer = NULL;
    ArrSt(i_Resource) *resources = arrst_create(i_Resource);
    return i_create_respack(i_ekTYPE_EMBEDDED, &lname, &buffer, &resources);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_remaining(const i_Parser *parser)
{
    cassert_no_null(parser);
    cassert(parser->end >= parser->cur);
    return (uint32_t)(parser->end - parser->cur);
}

/*---------------------------------------------------------------------------*/

static bool_t i_read_u32(i_Parser *parser, uint32_t *value)
{
    cassert_no_null(parser);
    cassert_no_null(value);
    if (i_remaining(parser) < sizeof32(uint32_t))
        return FALSE;

    bmem_copy(cast(value, byte_t), parser->cur, sizeof32(uint32_t));
    parser->cur += sizeof32(uint32_t);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_read_data(i_Parser *parser, const byte_t **data, const uint32_t size)
{
    cassert_no_null(parser);
    cassert_no_null(data);
    if (i_remaining(parser) < size)
        return FALSE;

    *data = parser->cur;
    parser->cur += size;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_valid_type(const uint32_t type)
{
    return (bool_t)(type <= 2);
}

/*---------------------------------------------------------------------------*/

static bool_t i_load_object(i_Parser *parser, const byte_t **data, uint32_t *size, const uint32_t type)
{
    uint32_t object_size;
    const byte_t *object_data = NULL;
    cassert_no_null(data);
    cassert_no_null(size);

    if (i_read_u32(parser, &object_size) == FALSE)
        return FALSE;

    /* String */
    if (type == 0)
    {
        uint32_t stored_size;
        if (object_size == UINT32_MAX)
            return FALSE;

        stored_size = object_size + 1;
        if (i_read_data(parser, &object_data, stored_size) == FALSE)
            return FALSE;

        if (object_data[object_size] != '\0')
            return FALSE;

        if (str_len_c(cast_const(object_data, char_t)) != object_size)
            return FALSE;

        *data = object_data;
        *size = 0;
    }
    /* Other file */
    else
    {
        if (type == 1 && object_size == 0)
            return FALSE;

        if (i_read_data(parser, &object_data, object_size) == FALSE)
            return FALSE;

        *data = object_data;
        *size = object_size;
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*/

static bool_t i_load_resource(i_Parser *parser, const uint32_t locale_code, const uint32_t num_locales, i_Resource *resource)
{
    uint32_t type;
    const byte_t *data = NULL;
    uint32_t size = 0;
    void *object = NULL;
    uint32_t i, num_localized;

    if (i_read_u32(parser, &type) == FALSE)
        return FALSE;

    if (i_valid_type(type) == FALSE)
        return FALSE;

    if (i_load_object(parser, &data, &size, type) == FALSE)
        return FALSE;

    if (i_read_u32(parser, &num_localized) == FALSE)
        return FALSE;

    if (num_localized > i_remaining(parser) / (type == 0 ? 9 : 8))
        return FALSE;

    for (i = 0; i < num_localized; ++i)
    {
        uint32_t lcode;
        if (i_read_u32(parser, &lcode) == FALSE)
            return FALSE;

        if (lcode >= num_locales)
            return FALSE;

        if (lcode == locale_code)
        {
            if (i_load_object(parser, &data, &size, type) == FALSE)
                return FALSE;
        }
        else
        {
            const byte_t *jump_data = NULL;
            uint32_t jump_size = 0;
            if (i_load_object(parser, &jump_data, &jump_size, type) == FALSE)
                return FALSE;
        }
    }

    i_init_resource(resource, type, data, size, &object);
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static Buffer *i_load_pack(ArrSt(i_Resource) *resources, const char_t *name, const char_t *locale)
{
    String *resfile = NULL;
    Buffer *buffer = NULL;

    {
        String *path;
        char_t pathname[512];
        bfile_dir_exec(pathname, 512);
        str_split_pathname(pathname, &path, NULL);
#if defined(__WINDOWS__)
        resfile = str_printf("%s%cres%c%s.res", tc(path), DIR_SEPARATOR, DIR_SEPARATOR, name);
#elif defined(__MACOS__)
        resfile = str_printf("%s%c..%cresources%c%s.res", tc(path), DIR_SEPARATOR, DIR_SEPARATOR, DIR_SEPARATOR, name);
#elif defined(__IOS__)
        cassert(FALSE);
        resfile = str_printf("%s%c..%cresources%c%s.res", tc(path), DIR_SEPARATOR, DIR_SEPARATOR, DIR_SEPARATOR, name);
#elif defined(__LINUX__)
        resfile = str_printf("%s%cres%c%s.res", tc(path), DIR_SEPARATOR, DIR_SEPARATOR, name);
#endif
        str_destroy(&path);
    }

    buffer = hfile_buffer(tc(resfile), NULL);
    if (buffer != NULL)
    {
        uint32_t locale_code = UINT32_MAX;
        i_Parser parser;
        bool_t ok = TRUE;
        uint32_t num_resources = 0;
        uint32_t i, num_locales = 0;

        parser.cur = buffer_data(buffer);
        parser.end = parser.cur + buffer_size(buffer);

        if (i_read_u32(&parser, &num_locales) == FALSE)
            ok = FALSE;

        if (ok == TRUE && i_remaining(&parser) < sizeof32(uint32_t))
            ok = FALSE;

        if (ok == TRUE && num_locales > (i_remaining(&parser) - sizeof32(uint32_t)) / 5)
            ok = FALSE;

        for (i = 0; ok == TRUE && i < num_locales; ++i)
        {
            uint32_t locale_size;
            const byte_t *locale_data = NULL;

            if (i_read_u32(&parser, &locale_size) == FALSE)
            {
                ok = FALSE;
                break;
            }

            if (locale_size == 0)
            {
                ok = FALSE;
                break;
            }

            if (i_read_data(&parser, &locale_data, locale_size) == FALSE)
            {
                ok = FALSE;
                break;
            }

            if (locale_data[locale_size - 1] != '\0')
            {
                ok = FALSE;
                break;
            }

            if (str_len_c(cast_const(locale_data, char_t)) + 1 != locale_size)
            {
                ok = FALSE;
                break;
            }

            if (locale_code == UINT32_MAX && locale != NULL)
            {
                const char_t *locale_name = cast_const(locale_data, char_t);
                if (str_equ_c(locale, locale_name) == TRUE)
                    locale_code = i;
            }
        }

        if (ok == TRUE && i_read_u32(&parser, &num_resources) == FALSE)
            ok = FALSE;

        if (ok == TRUE && num_resources > i_remaining(&parser) / 12)
            ok = FALSE;

        for (i = 0; ok == TRUE && i < num_resources; ++i)
        {
            i_Resource resource;
            if (i_load_resource(&parser, locale_code, num_locales, &resource) == FALSE)
            {
                ok = FALSE;
                break;
            }

            arrst_append(resources, resource, i_Resource);
        }

        if (ok == TRUE && parser.cur != parser.end)
            ok = FALSE;

        if (ok == FALSE)
        {
            arrst_clear(resources, i_remove_resource, i_Resource);
            buffer_destroy(&buffer);
        }
    }

    str_destroy(&resfile);
    return buffer;
}

/*---------------------------------------------------------------------------*/

ResPack *respack_packed(const char_t *name, const char_t *locale)
{
    String *lname = str_c(name);
    ArrSt(i_Resource) *resources = arrst_create(i_Resource);
    Buffer *buffer = i_load_pack(resources, name, locale);
    if (buffer == NULL)
    {
        arrst_destroy(&resources, i_remove_resource, i_Resource);
        str_destroy(&lname);
        return NULL;
    }

    return i_create_respack(i_ekTYPE_PACKED, &lname, &buffer, &resources);
}

/*---------------------------------------------------------------------------*/

void respack_add_msg(ResPack *pack, const char_t *msg)
{
    i_Resource *resource;
    void *object = NULL;
    cassert_no_null(pack);
    cassert(pack->type == i_ekTYPE_EMBEDDED);
    resource = arrst_new(pack->resources, i_Resource);
    i_init_resource(resource, 0, cast_const(msg, byte_t), UINT32_MAX, &object);
}

/*---------------------------------------------------------------------------*/

void respack_add_cdata(ResPack *pack, const uint32_t type, const byte_t *data, const uint32_t data_size)
{
    i_Resource *resource;
    void *object = NULL;
    cassert_no_null(pack);
    cassert(pack->type == i_ekTYPE_EMBEDDED);
    resource = arrst_new(pack->resources, i_Resource);
    i_init_resource(resource, type, data, data_size, &object);
}

/*---------------------------------------------------------------------------*/

static ___INLINE const char_t *i_magic(const ResId id)
{
    const char_t *magic;
    if (id == NULL)
        return NULL;

    magic = str_str(cast_const(id, char_t), "::");
    if (magic == NULL)
        return NULL;
    if (str_equ_cn(cast_const(id, char_t), "N23R3C75", (uint32_t)(magic - cast_const(id, char_t))) == FALSE)
        return NULL;
    if (str_str(magic + 2, "::") == NULL)
        return NULL;
    return magic + 2;
}

/*---------------------------------------------------------------------------*/

static bool_t i_parse_index(const char_t *str, uint32_t *index)
{
    uint32_t value = 0;
    bool_t has_digit = FALSE;
    cassert_no_null(index);

    if (str == NULL)
        return FALSE;

    while (*str != '\0')
    {
        uint32_t digit;
        if (*str < '0' || *str > '9')
            return FALSE;

        has_digit = TRUE;
        digit = (uint32_t)(*str - '0');
        if (value > (UINT32_MAX - digit) / 10)
            return FALSE;

        value = value * 10 + digit;
        str += 1;
    }

    if (has_digit == FALSE)
        return FALSE;

    *index = value;
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static ___INLINE bool_t i_index(ResId id, const String *name, uint32_t *index)
{
    const char_t *idr = i_magic(id);
    const char_t *packid = NULL;
    uint32_t name_size;
    if (index == NULL)
        return FALSE;

    *index = 0;

    if (idr == NULL || name == NULL)
        return FALSE;

    packid = str_str(idr, "::");
    if (packid == NULL)
        return FALSE;

    name_size = (uint32_t)(packid - idr);
    if (str_len(name) != name_size)
        return FALSE;

    if (str_equ_cn(tc(name), idr, name_size) == FALSE)
        return FALSE;

    return i_parse_index(packid + 2, index);
}

/*---------------------------------------------------------------------------*/

const char_t *respack_text(const ResPack *pack, const ResId id)
{
    i_Resource *resource = NULL;
    uint32_t index;
    if (pack == NULL || i_index(id, pack->name, &index) == FALSE)
        return NULL;

    if (index >= arrst_size(pack->resources, i_Resource))
        return NULL;

    resource = arrst_get(pack->resources, index, i_Resource);
    if (resource == NULL || resource->type != 0 || resource->data == NULL)
        return NULL;

    return cast_const(resource->data, char_t);
}

/*---------------------------------------------------------------------------*/

const byte_t *respack_file(const ResPack *pack, const ResId id, uint32_t *size)
{
    i_Resource *resource = NULL;
    uint32_t index;
    ptr_assign(size, 0);

    if (pack == NULL || i_index(id, pack->name, &index) == FALSE)
        return NULL;

    if (index >= arrst_size(pack->resources, i_Resource))
        return NULL;

    resource = arrst_get(pack->resources, index, i_Resource);
    if (resource == NULL || resource->type != 2 || resource->data == NULL)
        return NULL;

    ptr_assign(size, resource->size);
    return resource->data;
}

/*---------------------------------------------------------------------------*/

void *respack_object_imp(const ResPack *pack, const ResId id, FPtr_from_data func_create, FPtr_destroy func_destroy)
{
    i_Resource *resource = NULL;
    uint32_t index;
    if (pack == NULL || i_index(id, pack->name, &index) == FALSE)
        return NULL;

    if (index >= arrst_size(pack->resources, i_Resource))
        return NULL;

    resource = arrst_get(pack->resources, index, i_Resource);
    if (resource == NULL || resource->type == 0 || resource->data == NULL)
        return NULL;

    if (resource->object == NULL)
    {
        if (func_create == NULL || func_destroy == NULL)
            return NULL;

        resource->object = func_create(resource->data, resource->size);
        resource->func_destroy = func_destroy;
    }

    return resource->object;
}

/*---------------------------------------------------------------------------*/

static ___INLINE i_Resource *i_resource(const ArrPt(ResPack) *packs, const ResId id, bool_t *is_resid)
{
    const char_t *idr = i_magic(id);
    const char_t *packid = NULL;
    if (is_resid != NULL)
        *is_resid = (bool_t)(idr != NULL);

    if (idr == NULL || packs == NULL)
        return NULL;

    packid = str_str(idr, "::");
    if (packid != NULL)
    {
        uint32_t name_size = (uint32_t)(packid - idr);
        arrpt_foreach_const(pack, packs, ResPack)
            if (pack != NULL && str_len(pack->name) == name_size && str_cmp_cn(tc(pack->name), idr, name_size) == 0)
            {
                uint32_t idx;
                if (i_parse_index(packid + 2, &idx) == TRUE && idx < arrst_size(pack->resources, i_Resource))
                    return arrst_get(pack->resources, idx, i_Resource);
            }
        arrpt_end()
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/

const char_t *respack_atext(const ArrPt(ResPack) *packs, const ResId id, bool_t *is_resid)
{
    const i_Resource *resource = i_resource(packs, id, is_resid);
    if (resource != NULL)
    {
        if (resource->type == 0 && resource->data != NULL)
            return cast_const(resource->data, char_t);
    }

    return NULL;
}

/*---------------------------------------------------------------------------*/

const byte_t *respack_afile(const ArrPt(ResPack) *packs, const ResId id, uint32_t *size, bool_t *is_resid)
{
    i_Resource *resource = i_resource(packs, id, is_resid);
    if (resource != NULL)
    {
        if (resource->type == 2 && resource->data != NULL)
        {
            ptr_assign(size, resource->size);
            return resource->data;
        }
    }

    ptr_assign(size, 0);
    return NULL;
}

/*---------------------------------------------------------------------------*/

void *respack_aobj_imp(const ArrPt(ResPack) *packs, const ResId id, FPtr_from_data func_create, FPtr_destroy func_destroy, bool_t *is_resid)
{
    i_Resource *resource = i_resource(packs, id, is_resid);
    if (resource != NULL)
    {
        if (resource->type == 0 || resource->data == NULL)
            return NULL;

        if (resource->object == NULL)
        {
            if (func_create == NULL || func_destroy == NULL)
                return NULL;

#if defined(__ASSERTS__)
            resource->id = id;
#endif
            resource->object = func_create(resource->data, resource->size);
            resource->func_destroy = func_destroy;
        }

        return resource->object;
    }

    return NULL;
}