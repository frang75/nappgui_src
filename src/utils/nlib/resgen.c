/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: resgen.c
 *
 */

/* Application resource packs generation */

#include "resgen.inl"
#include "nrclib.h"
#include "msgparser.inl"
#include "arrpt.h"
#include "arrst.h"
#include "bmem.h"
#include "bfile.h"
#include "buffer.h"
#include "cassert.h"
#include "date.h"
#include "event.h"
#include "heap.h"
#include "hfile.h"
#include "ptr.h"
#include "stream.h"
#include "strings.h"
#include "unicode.h"

typedef union i_object_t i_Object;
typedef struct i_local_t i_Local;
typedef struct i_resource_t i_Resource;

typedef enum _i_resource_type_t
{
    i_ekRESOURCE_TYPE_MESSAGE       = 0,
    i_ekRESOURCE_TYPE_IMAGE         = 1,
    i_ekRESOURCE_TYPE_FILE          = 2
} i_resource_type_t;

union i_object_t
{
    String *string;
    Buffer *file_data;
};

struct i_local_t
{
    uint32_t index;
    i_Object object;
};

struct i_resource_t
{
    uint32_t index;
    i_resource_type_t type;
    String *name;
    i_Object global;
    ArrSt(i_Local) *locals;
};

struct _resource_pack_t
{
    uint32_t local_index;
    uint32_t num_locals;
    ArrPt(String) *local_codes;
    ArrSt(i_Resource) *resources;
    ArrPt(String) *warnings;
    ArrPt(String) *errors;
};

DeclSt(i_Local);
DeclSt(i_Resource);

/*---------------------------------------------------------------------------*/

#define i_MAX_RESOURCE_PACK_SIZE    4194304
static const char_t i_HEX_CODE[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

/*---------------------------------------------------------------------------*/

static void i_init_object_string(i_Object *object, String **str)
{
    cassert_no_null(object);
    cassert(object->string == NULL);
    object->string = ptr_dget_no_null(str, String);
}

/*---------------------------------------------------------------------------*/

static void i_remove_object(i_Object *object, const i_resource_type_t type)
{
    cassert_no_null(object);
    switch (type)
    {
        case i_ekRESOURCE_TYPE_MESSAGE:
            if (object->string != NULL)
                str_destroy(&object->string);
            break;
        case i_ekRESOURCE_TYPE_IMAGE:
        case i_ekRESOURCE_TYPE_FILE:
            if (object->file_data != NULL)
                buffer_destroy(&object->file_data);
            break;
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

#if defined(__ASSERTS__)
static bool_t i_object_is_null(const i_Object *object, const i_resource_type_t type)
{
    cassert_no_null(object);
    switch (type)
    {
        case i_ekRESOURCE_TYPE_MESSAGE:
            return (bool_t)(object->string == NULL);
        case i_ekRESOURCE_TYPE_IMAGE:
        case i_ekRESOURCE_TYPE_FILE:
            return (bool_t)(object->file_data == NULL);
        cassert_default();
    }

    return FALSE;
}
#endif

/*---------------------------------------------------------------------------*/

static void i_init_local(i_Local *local, const uint32_t index, const i_Object *object)
{
    cassert_no_null(local);
    local->index = index;
    local->object = ptr_get(object, i_Object);
}

/*---------------------------------------------------------------------------*/

static void i_remove_local(i_Local *local, const i_resource_type_t type)
{
    cassert_no_null(local);
    i_remove_object(&local->object, type);
}

/*---------------------------------------------------------------------------*/

static void i_init_resource(i_Resource *resource, const uint32_t index, const i_resource_type_t type, String **name, i_Object *global, ArrSt(i_Local) **locals)
{
    cassert_no_null(resource);
    resource->index = index;
    resource->type = type;
    resource->name = ptr_dget_no_null(name, String);
    resource->global = ptr_get(global, i_Object);
    resource->locals = ptr_dget_no_null(locals, ArrSt(i_Local));
}

/*---------------------------------------------------------------------------*/

static void i_remove_resource(i_Resource *resource)
{
    cassert_no_null(resource);
    str_destroy(&resource->name);
    i_remove_object(&resource->global, resource->type);
    arrst_foreach(local, resource->locals, i_Local)
        i_remove_local(local, resource->type);
    arrst_end();
    arrst_destroy(&resource->locals, NULL, i_Local);
}

/*---------------------------------------------------------------------------*/

static ResourcePack *i_create_pack(const uint32_t local_index, const uint32_t num_locals, ArrPt(String) **local_codes, ArrSt(i_Resource) **resources, ArrPt(String) *warnings, ArrPt(String) *errors)
{
    ResourcePack *pack = heap_new(ResourcePack);
    pack->local_index = local_index;
    pack->num_locals = num_locals;
    pack->local_codes = ptr_dget_no_null(local_codes, ArrPt(String));
    pack->resources = ptr_dget_no_null(resources, ArrSt(i_Resource));
    pack->warnings = warnings;
    pack->errors = errors;
    return pack;
}

/*---------------------------------------------------------------------------*/

void resgen_destroy_pack(ResourcePack **pack)
{
    cassert_no_null(pack);
    cassert_no_null(*pack);
    arrpt_destroy(&(*pack)->local_codes, str_destroy, String);
    arrst_destroy(&(*pack)->resources, i_remove_resource, i_Resource);
    heap_delete(pack, ResourcePack);
}

/*---------------------------------------------------------------------------*/

static i_resource_type_t i_resource_type_by_file_extension(const char_t *file_extension)
{
    String *lowercase = str_c(file_extension);
    i_resource_type_t type = i_ekRESOURCE_TYPE_FILE;
    str_lower(lowercase);

    if (str_equ(lowercase, "png") == TRUE)
    {
        type = i_ekRESOURCE_TYPE_IMAGE;
    }
    else if (str_equ(lowercase, "jpg") == TRUE)
    {
        type = i_ekRESOURCE_TYPE_IMAGE;
    }
    else if (str_equ(lowercase, "gif") == TRUE)
    {
        type = i_ekRESOURCE_TYPE_IMAGE;
    }
    else if (str_equ(lowercase, "bmp") == TRUE)
    {
        type = i_ekRESOURCE_TYPE_IMAGE;
    }
    else if (str_equ(lowercase, "msg") == TRUE)
    {
        type = i_ekRESOURCE_TYPE_MESSAGE;
    }
    
    str_destroy(&lowercase);
    return type;
}

/*---------------------------------------------------------------------------*/

static bool_t i_file_is_resource(const char_t *filename)
{
    const char_t *file_extension = str_filext(filename);
    i_resource_type_t type = i_resource_type_by_file_extension(file_extension);
    return (bool_t)(type != ENUM_MAX(i_resource_type_t));
}

/*---------------------------------------------------------------------------*/

static i_Resource *i_resource_by_name(ArrSt(i_Resource) *resources, const char_t *name)
{
    arrst_foreach(resource, resources, i_Resource)
        if (str_equ(resource->name, name) == TRUE)
            return resource;
    arrst_end();
    return NULL;
}

/*---------------------------------------------------------------------------*/

static i_Resource *i_create_new_resource(ArrSt(i_Resource) *resources, const i_resource_type_t type, const char_t *name)
{
    if (i_resource_by_name(resources, name) == NULL)
    {
        i_Resource *resource;
        uint32_t index;
        String *lname = NULL;
        i_Object global;
        ArrSt(i_Local) *locals = NULL;
        resource = arrst_new(resources, i_Resource);
        index = arrst_size(resources, i_Resource);
        lname = str_c(name);
        bmem_zero(&global, i_Object);
        locals = arrst_create(i_Local);
        i_init_resource(resource, index, type, &lname, &global, &locals);
        return resource;
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

static i_Object *i_add_local(i_Resource *resource, const uint32_t index)
{
    cassert_no_null(resource);
    arrst_foreach(local, resource->locals, i_Local)
        cassert(local->index != index);
    arrst_end()
    
    {
        i_Local *local;
        i_Object object;
        local = arrst_new(resource->locals, i_Local);
        bmem_zero(&object, i_Object);
        i_init_local(local, index, &object);
    }
    
    {
        i_Local *local = arrst_last(resource->locals, i_Local);
        cassert_no_null(local);
        cassert(local->index == index);
        return &local->object;
    }
}

/*---------------------------------------------------------------------------*/

static void i_read_msgfile(ResourcePack *pack, const uint32_t local_index, const char_t *pathname, const byte_t *file_data, const uint32_t file_size)
{
    ArrPt(String) *ids = NULL;
    ArrPt(String) *texts = NULL;
    uint32_t num_errors = UINT32_MAX;
    cassert_no_null(pack);
    num_errors = arrpt_size(pack->errors, String);
    msgparser_process(pathname, (const char_t*)file_data, file_size, &ids, &texts, pack->errors);
    if (num_errors == arrpt_size(pack->errors, String))
    {
        uint32_t i, num_texts = arrpt_size(ids, String);
        cassert(num_texts == arrpt_size(texts, String));
        if (num_texts == 0)
        {
            String *warning = str_printf("Empty message file '%s'.", pathname);
            arrpt_append(pack->warnings, warning, String);
        }

        for (i = 0; i < num_texts; ++i)
        {
            String *id = arrpt_get(ids, i, String);
            String *text = arrpt_get(texts, i, String);
            i_Object *object = NULL;

            if (local_index == UINT32_MAX)
            {
                i_Resource *resource = i_create_new_resource(pack->resources, i_ekRESOURCE_TYPE_MESSAGE, tc(id));
                if (resource != NULL)
                {
                    object = &resource->global;
                }
                else
                {
                    String *error = str_printf("Duplicate resource id in '%s' (%s).", pathname, tc(id));
                    arrpt_append(pack->errors, error, String);
                    object = NULL;
                }
            }
            else
            {
                i_Resource *resource = i_resource_by_name(pack->resources, tc(id));
                if (resource != NULL)
                {
                    object = i_add_local(resource, local_index);
                    pack->num_locals += 1;
                }
                else
                {
                    String *warning = str_printf("Ignored localized text '%s' in '%s'. Global resource doesn't exists.", tc(id), pathname);
                    arrpt_append(pack->warnings, warning, String);
                    object = NULL;
                }
            }

            if (object != NULL)
            {
                String *str_text = str_copy(text);
                cassert(i_object_is_null(object, i_ekRESOURCE_TYPE_MESSAGE) == TRUE);
                i_init_object_string(object, &str_text);
            }
        }

        if (local_index != UINT32_MAX)
        {
            arrst_foreach(resource, pack->resources, i_Resource)
                if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
                {
                    bool_t with_local = FALSE;
                    arrst_foreach(local, resource->locals, i_Local)
                        if (local->index == local_index)
                        {
                            with_local = TRUE;
                            break;
                        }
                    arrst_end();

                    if (with_local == FALSE)
                    {
                        String *warning = str_printf("There is no localized version of the text '%s' in '%s'.", tc(resource->name), pathname);
                        arrpt_append(pack->warnings, warning, String);
                    }
                }
            arrst_end();
        }
    }

    arrpt_destroy(&ids, str_destroy, String);
    arrpt_destroy(&texts, str_destroy, String);
}

/*---------------------------------------------------------------------------*/

static void i_read_file(ResourcePack *pack, const uint32_t local_code, const char_t *pathname, Buffer **file_data)
{
    const char_t *file_extension = NULL;
    i_resource_type_t type;
    cassert_no_null(pack);
    cassert_no_null(file_data);
    cassert(*file_data != NULL);
    file_extension = str_filext(pathname);
    type = i_resource_type_by_file_extension(file_extension);
    switch (type)
    {
        case i_ekRESOURCE_TYPE_MESSAGE:
            i_read_msgfile(pack, local_code, pathname, buffer_data(*file_data), buffer_size(*file_data));
            break;

        case i_ekRESOURCE_TYPE_IMAGE:
        case i_ekRESOURCE_TYPE_FILE:
        {
            String *filename = NULL;
            i_Object *object = NULL;

            str_split_pathname(pathname, NULL, &filename);

            if (local_code == UINT32_MAX)
            {
                i_Resource *resource = i_create_new_resource(pack->resources, type, tc(filename));
                if (resource != NULL)
                {
                    object = &resource->global;
                }
                else
                {
                    String *error = str_printf("Duplicate resource id '%s' (%s).", pathname, tc(filename));
                    arrpt_append(pack->errors, error, String);
                    object = NULL;
                }
            }
            else
            {
                i_Resource *resource = i_resource_by_name(pack->resources, tc(filename));
                if (resource != NULL)
                {
                    object = i_add_local(resource, local_code);
                    pack->num_locals += 1;
                }
                else
                {
                    const String *local = arrpt_get(pack->local_codes, local_code, String);
                    String *warning = str_printf("Ignored localized file '%s' in '%s'. Global resource doesn't exists.", tc(filename), tc(local));
                    arrpt_append(pack->warnings, warning, String);
                    object = NULL;
                }
            }

            if (object != NULL)
            {
                cassert(i_object_is_null(object, type) == TRUE);
                object->file_data = *file_data;
                *file_data = NULL;
            }

            str_destroy(&filename);
            break;
        }

        cassert_default();
    }
    
    if (*file_data != NULL)
        buffer_destroy(file_data);
}

/*---------------------------------------------------------------------------*/

static void i_read_dir(ResourcePack *pack, Event *event)
{
    const EvFileDir *params = NULL;
    cassert_no_null(pack);
    params = event_params(event, EvFileDir);
    cassert_no_null(params);
    cassert(event_type(event) == ekEFILE);
    if (i_file_is_resource(params->pathname) == TRUE)
    {
        Buffer *file_data = hfile_buffer(params->pathname, NULL);
        if (file_data != NULL)
        {
            i_read_file(pack, pack->local_index, params->pathname, &file_data);
        }
        else
        {
            String *error = str_printf("Can't load resource file '%s'", params->pathname);
            arrpt_append(pack->errors, error, String);
        }
    }
    else
    {
        String *warning = str_printf("Ignored resource file '%s' (Unknown type).", params->pathname);
        arrpt_append(pack->warnings, warning, String);
    }
}

/*---------------------------------------------------------------------------*/

static void i_read_localdir(ResourcePack *pack, Event *event)
{
    cassert_no_null(pack);
    if (event_type(event) == ekEENTRY)
    {
        const EvFileDir *params = event_params(event, EvFileDir);
        const char_t *dirname = str_filename(params->pathname);
        String *local_code = str_c(dirname);
        bool_t loaded = FALSE;
        pack->num_locals = 0;
        pack->local_index = arrpt_size(pack->local_codes, String);
        arrpt_append(pack->local_codes, local_code, String);
        loaded = hfile_dir_loop(params->pathname, listener(pack, i_read_dir, ResourcePack), FALSE, FALSE, NULL);

        if (loaded == TRUE)
        {
            if (pack->num_locals == 0)
            {
                String *warning = str_printf("Localized directory '%s' is empty or has invalid resources.", params->pathname);
                arrpt_append(pack->warnings, warning, String);
                arrpt_delete(pack->local_codes, pack->local_index, str_destroy, String);
            }
        }
        else
        {
            String *error = str_printf("Error reading '%s' resource directory.", params->pathname);
            arrpt_append(pack->errors, error, String);
        }
    }
}

/*---------------------------------------------------------------------------*/

static int i_compare_resource(const i_Resource *res1, const i_Resource *res2)
{
    cassert_no_null(res1);
    cassert_no_null(res2);
    if (res1->type == res2->type)
    {
        if (res1->type == i_ekRESOURCE_TYPE_MESSAGE)
        {
            cassert(res1->index != res2->index);
            if (res1->index < res2->index)
                return -1;
            else
                return 1;
        }

        return str_cmp(res1->name, tc(res2->name));
    }

    if (res1->type < res2->type)
        return -1;
    else
        return 1;
}

/*---------------------------------------------------------------------------*/

ResourcePack *resgen_pack_read(const char_t *src_dir, ArrPt(String) *warnings, ArrPt(String) *errors)
{
    ArrPt(String) *local_codes = NULL;
    ArrSt(i_Resource) *resources = NULL;
    ResourcePack *pack = NULL;
    local_codes = arrpt_create(String);
    resources = arrst_create(i_Resource);
    pack = i_create_pack(UINT32_MAX, 0, &local_codes, &resources, warnings, errors);

    {
        bool_t loaded = hfile_dir_loop(src_dir, listener(pack, i_read_dir, ResourcePack), FALSE, FALSE, NULL);
        if (loaded == FALSE)
        {
            String *error = str_printf("Error reading '%s' resource directory.", src_dir);
            arrpt_append(pack->errors, error, String);
        }
    }

    {
        bool_t loaded = hfile_dir_loop(src_dir, listener(pack, i_read_localdir, ResourcePack), TRUE, FALSE, NULL);
        if (loaded == FALSE)
        {
            String *error = str_printf("Error reading '%s' subdirectories.", src_dir);
            arrpt_append(pack->errors, error, String);
        }
    }

    pack->warnings = NULL;
    pack->errors = NULL;
    arrst_sort(pack->resources, i_compare_resource, i_Resource);
    return pack;
}

/*---------------------------------------------------------------------------*/

static void i_stm_header(Stream *stream)
{
    stm_printf(stream, "/* Automatic generated by NAppGUI Resource Compiler (nrc-v%s) */\n\n", NAPPGUI_BUILD);
    //stm_writef(stream, "/* (support@nappgui.com) */\n\n");
}

/*---------------------------------------------------------------------------*/

static String *i_define_resname(const String *resource_name)
{
    String *name = str_copy(resource_name);
    str_subs(name, '.', '_');
    str_upper(name);
    return name;
}

/*---------------------------------------------------------------------------*/

static String *i_global_resname(const String *resource_name)
{
    String *name = str_printf("%s", tc(resource_name));
    str_subs(name, '.', '_');
    str_upper(name);
    return name;
}

/*---------------------------------------------------------------------------*/

static String *i_local_resname(const String *resource_name, const char_t *local_code)
{
    String *name = str_printf("%s_%s", tc(resource_name), local_code);
    str_subs(name, '.', '_');
    str_upper(name);
    return name;
}

/*---------------------------------------------------------------------------*/

void resgen_write_h_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors)
{
    String *pathname = str_printf("%s%c%s.h", dest_path, DIR_SEPARATOR, dest_file);
    Stream *stream = stm_to_file(tc(pathname), NULL);
    cassert_no_null(pack);
    if (stream != NULL)
    {
        bool_t with_texts = FALSE;
        bool_t with_files = FALSE;
        i_stm_header(stream);
        stm_writef(stream, "#include \"core.hxx\"\n\n");
        stm_writef(stream, "__EXTERN_C\n\n");

        arrst_foreach(resource, pack->resources, i_Resource)
            String *name = i_define_resname(resource->name);

            if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
            {
                if (with_texts == FALSE)
                {
                    stm_writef(stream, "/* Messages */\n");
                    with_texts = TRUE;
                }
            }
            else
            {
                if (with_files == FALSE)
                {
                    if (with_texts == TRUE)
                        stm_writef(stream, "\n/* Files */\n");

                    with_files = TRUE;
                }
            }

            stm_printf(stream, "extern ResId %s;\n", tc(name));
            str_destroy(&name);
        arrst_end();

        stm_writef(stream, "\n");
        stm_printf(stream, "ResPack *%s_respack(const char_t *locale);\n\n", dest_file);
        stm_writef(stream, "__END_C\n");
        stm_close(&stream);
    }
    else
    {
        String *error = str_printf("Error creating '%s' header file.", tc(pathname));
        arrpt_append(errors, error, String);
    }

    str_destroy(&pathname);
}

/*---------------------------------------------------------------------------*/

static void i_binary_to_ascii(Stream *stream, const byte_t *binary_code, const uint32_t size, const uint32_t num_bytes_per_row, const bool_t static_keyword, const char_t *variable_name)
{
    register uint32_t i, j;

    cassert(num_bytes_per_row > 0);

    if (static_keyword == TRUE)
        stm_printf(stream, "static const uint32_t %s_SIZE = %u;\n\n", variable_name, size);
    else
        stm_printf(stream, "const uint32_t %s_SIZE = %u;\n\n", variable_name, size);

    j = 0;

    if (static_keyword == TRUE)
        stm_printf(stream, "static const byte_t %s_DATA[] = {\n", variable_name);
    else
        stm_printf(stream, "const byte_t %s_DATA[] = {\n", variable_name);

    stm_writef(stream, "    ");

    for (i = 0; i < size; ++i)
    {
        char_t code1 = i_HEX_CODE[(binary_code[i] >> 4) & 0x0F];
        char_t code2 = i_HEX_CODE[binary_code[i] & 0x0F];

        stm_printf(stream, "0x%c%c", code1, code2);
        
        if (i < size - 1)
            stm_writef(stream, ",");

        j += 1;

        if (i < size - 1)
        {
            if (j == num_bytes_per_row)
            {
                stm_writef(stream, "\n    ");
                j = 0;
            }
            else
            {
                if (i < size - 1)
                    stm_writef(stream, " ");
            }
        }
    }

    stm_writef(stream, "};");
}

/*---------------------------------------------------------------------------*/

static bool_t i_write_local(Stream *stream, const i_Resource *resource, const char_t *local_code, const uint32_t local_index)
{
    cassert_no_null(resource);
    arrst_foreach(local, resource->locals, i_Local)
        if (local->index == local_index)
        {
            String *name = i_local_resname(resource->name, local_code);
            if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
                stm_printf(stream, "        respack_add_msg(pack, %s_TEXT);\n", tc(name));
            else
                stm_printf(stream, "        respack_add_cdata(pack, %d, %s_DATA, %s_SIZE);\n", resource->type, tc(name), tc(name));
            str_destroy(&name);
            return TRUE;
        }
    arrst_end();
    return FALSE;
}

/*---------------------------------------------------------------------------*/

static void i_write_global(Stream *stream, const i_Resource *resource)
{
    String *name = NULL;
    cassert_no_null(resource);
    name = i_global_resname(resource->name);
    if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
        stm_printf(stream, "        respack_add_msg(pack, %s_TEXT);\n", tc(name));
    else
        stm_printf(stream, "        respack_add_cdata(pack, %d, %s_DATA, %s_SIZE);\n", resource->type, tc(name), tc(name));
    str_destroy(&name);
}

/*---------------------------------------------------------------------------*/

static void i_write_message(Stream *stm, const char_t *msg)
{
    bool_t in_scape = FALSE;
    while (*msg != '\0')
    {
        uint32_t code = unicode_to_u32(msg, ekUTF8);
        if (code == '\"')
        {
            if (in_scape == TRUE)
            {
                stm_writef(stm, "\\");
                in_scape = FALSE;
            }

            stm_writef(stm, "\\\"");
        }
        else if (*msg == '\\')
        {
            if (in_scape == FALSE)
            {
                in_scape = TRUE;
            }
            else
            {
                stm_writef(stm, "\\\\");
                in_scape = FALSE;
            }
        }
        else if (*msg == 'n')
        {
            if (in_scape == TRUE)
            {
                stm_write_char(stm, '\n');
                in_scape = FALSE;
            }
            else
            {
                stm_write_char(stm, 'n');
            }
        }
        else
        {
            if (in_scape == TRUE)
            {
                stm_writef(stm, "\\");
                in_scape = FALSE;
            }

            stm_write_char(stm, code);
        }

        msg = unicode_next(msg, ekUTF8);
    }
}

/*---------------------------------------------------------------------------*/

void resgen_write_c_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors)
{
    String *pathname = str_printf("%s%c%s.c", dest_path, DIR_SEPARATOR, dest_file);
    Stream *stream = stm_to_file(tc(pathname), NULL);
    cassert_no_null(pack);
    if (stream != NULL)
    {
        bool_t with_texts = FALSE;
        bool_t with_files = FALSE;
        i_stm_header(stream);
        stm_printf(stream, "#include \"%s.h\"\n", dest_file);
        stm_writef(stream, "#include \"cassert.h\"\n");
        stm_writef(stream, "#include \"respackh.h\"\n");
        stm_writef(stream, "#include \"strings.h\"\n\n");
        stm_writef(stream, "/*---------------------------------------------------------------------------*/\n\n");

        /* Resource IDs */
        arrst_foreach(resource, pack->resources, i_Resource)
            String *name = i_define_resname(resource->name);
            stm_printf(stream, "ResId %s = \"N23R3C75::%s::%d\";\n", tc(name), dest_file, resource_i);
            str_destroy(&name);
        arrst_end();

        /* Resource serialized data */
        arrst_foreach(resource, pack->resources, i_Resource)
            String *name = i_global_resname(resource->name);
            if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
            {
                stm_printf(stream, "static const char_t *%s_TEXT = \"", tc(name));
                i_write_message(stream, tc(resource->global.string));
                stm_writef(stream, "\";\n");
                with_texts = TRUE;
            }
            else
            {
                if (with_files == FALSE)
                {
                    if (with_texts == TRUE)
                        stm_writef(stream, "\n/*---------------------------------------------------------------------------*/\n\n");
                    with_files = TRUE;
                }

                i_binary_to_ascii(stream, buffer_data(resource->global.file_data), buffer_size(resource->global.file_data), 50, TRUE, tc(name));
                stm_writef(stream, "\n\n/*---------------------------------------------------------------------------*/\n\n");
            }

            arrst_foreach(local, resource->locals, i_Local)
                const String *local_code = arrpt_get(pack->local_codes, local->index, String);
                String *local_name = i_local_resname(resource->name, tc(local_code));
                if (resource->type == i_ekRESOURCE_TYPE_MESSAGE)
                {
                    stm_printf(stream, "static const char_t *%s_TEXT = \"", tc(local_name));
                    i_write_message(stream, tc(local->object.string));
                    stm_writef(stream, "\";\n");
                }
                else
                {
                    i_binary_to_ascii(stream, buffer_data(local->object.file_data), buffer_size(local->object.file_data), 50, FALSE, tc(local_name));
                    stm_writef(stream, "\n\n/*---------------------------------------------------------------------------*/\n\n");
                }

                str_destroy(&local_name);
            arrst_end();

        str_destroy(&name);
        arrst_end();

        if (with_files == FALSE)
        {
            stm_writef(stream, "\n/*---------------------------------------------------------------------------*/\n\n");
            with_files = TRUE;
        }

        /* ResPack public interface */
        stm_printf(stream, "ResPack *%s_respack(const char_t *locale)\n", dest_file);
        stm_writef(stream, "{\n");
        stm_printf(stream, "    ResPack *pack = respack_embedded(\"%s\");\n", dest_file);

        arrpt_foreach(local_code, pack->local_codes, String)
        stm_printf(stream, "    if (str_equ_c(locale, \"%s\") == TRUE)\n", tc(local_code));
        stm_writef(stream, "    {\n");
        arrst_foreach(resource, pack->resources, i_Resource)
            if (i_write_local(stream, resource, tc(local_code), local_code_i) == FALSE)
                i_write_global(stream, resource);
        arrst_end();
        stm_writef(stream, "        return pack;\n");
        stm_writef(stream, "    }\n\n");
        arrpt_end();

        stm_writef(stream, "    /* Non-localized pack */\n");
        stm_writef(stream, "    {\n");
        stm_writef(stream, "        unref(locale);\n");
        arrst_foreach(resource, pack->resources, i_Resource)
            i_write_global(stream, resource);
        arrst_end();
        stm_writef(stream, "        return pack;\n");
        stm_writef(stream, "    }\n");
        stm_writef(stream, "}\n");

        stm_close(&stream);
    }
    else
    {
        String *error = str_printf("Error creating '%s' source file.", tc(pathname));
        arrpt_append(errors, error, String);
    }

    str_destroy(&pathname);
}

/*---------------------------------------------------------------------------*/

static void i_object_write(Stream *stream, const i_Object *object, const i_resource_type_t type)
{
    cassert_no_null(object);
    switch (type)
    {
        case i_ekRESOURCE_TYPE_MESSAGE:
        {
            uint32_t size = str_len(object->string);
            const byte_t *data = (byte_t*)tc(object->string);
            const byte_t end = 0;
            stm_write_u32(stream, size);
            stm_write(stream, data, size);
            stm_write(stream, &end, 1);
            break;
        }

        case i_ekRESOURCE_TYPE_IMAGE:
        case i_ekRESOURCE_TYPE_FILE:
        {
            uint32_t size = buffer_size(object->file_data);
            const byte_t *data = buffer_data(object->file_data);
            stm_write_u32(stream, size);
            stm_write(stream, data, size);
            break;
        }
        cassert_default();
    }
}

/*---------------------------------------------------------------------------*/

void resgen_write_packed_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors)
{
    String *pathname = str_printf("%s%c%s.res", dest_path, DIR_SEPARATOR, dest_file);
    Stream *stream = stm_to_file(tc(pathname), NULL);
    cassert_no_null(pack);
    if (stream != NULL)
    {
        register uint32_t num_locals = arrpt_size(pack->local_codes, String);
        register uint32_t num_res = arrst_size(pack->resources, i_Resource);

        /* Write localization codes */
        stm_write_u32(stream, num_locals);
        arrpt_foreach(local, pack->local_codes, String)
            str_write(stream, local);
        arrpt_end();

        /* Write resources */
        stm_write_u32(stream, num_res);
        arrst_foreach(resource, pack->resources, i_Resource)
            register uint32_t num_localized = arrst_size(resource->locals, i_Local);
            stm_write_u32(stream, resource->type);
            i_object_write(stream, &resource->global, resource->type);
            stm_write_u32(stream, num_localized);
            arrst_foreach(local, resource->locals, i_Local);
                stm_write_u32(stream, local->index);
                i_object_write(stream, &local->object, resource->type);
            arrst_end();
        arrst_end();

        stm_close(&stream);
    }
    else
    {
        String *error = str_printf("Error creating '%s' packed file.", tc(pathname));
        arrpt_append(errors, error, String);
    }

    str_destroy(&pathname);
}

/*---------------------------------------------------------------------------*/

void resgen_write_c_packed_file(const ResourcePack *pack, const char_t *dest_path, const char_t *dest_file, ArrPt(String) *errors)
{
    String *pathname = str_printf("%s%c%s.c", dest_path, DIR_SEPARATOR, dest_file);
    Stream *stream = stm_to_file(tc(pathname), NULL);
    cassert_no_null(pack);
    if (stream != NULL)
    {
        i_stm_header(stream);
        stm_printf(stream, "#include \"%s.h\"\n", dest_file);
        stm_writef(stream, "#include \"respackh.h\"\n\n");
        stm_writef(stream, "/*---------------------------------------------------------------------------*/\n\n");

        /* Resource IDs */
        arrst_foreach(resource, pack->resources, i_Resource)
            String *name = i_define_resname(resource->name);
            stm_printf(stream, "ResId %s = \"N23R3C75::%s::%d\";\n", tc(name), dest_file, resource_i);
            str_destroy(&name);
        arrst_end();

        stm_writef(stream, "\n/*---------------------------------------------------------------------------*/\n\n");

        stm_printf(stream, "ResPack *%s_respack(const char_t *locale)\n", dest_file);
        stm_writef(stream, "{\n");
        stm_printf(stream, "    return respack_packed(\"%s\", locale);\n", dest_file);
        stm_writef(stream, "}\n");
        stm_close(&stream);
    }
    else
    {
        String *error = str_printf("Error creating '%s' source file.", tc(pathname));
        arrpt_append(errors, error, String);
    }

    str_destroy(&pathname);
}
