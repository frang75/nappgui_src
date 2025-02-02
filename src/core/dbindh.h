/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dbindh.h
 *
 */

/*
 * Undocumented (hidden) API to use DBind database
 * obj: Memory address that represent an object (struct instance).
 * data: Memory address that represent an object field position (obj + field offset) or plain variable.
 *
 */

#include "coreh.hxx"

__EXTERN_C

/* Data type registration */

_core_api dbindst_t dbind_bool_imp(const char_t *type, const uint16_t size);

_core_api dbindst_t dbind_int_imp(const char_t *type, const uint16_t size, const bool_t is_signed);

_core_api dbindst_t dbind_real_imp(const char_t *type, const uint16_t size);

_core_api dbindst_t dbind_string_imp(const char_t *type, FPtr_str_create func_create, FPtr_destroy func_destroy, FPtr_str_get func_get, FPtr_read func_read, FPtr_write func_write, const char_t *def);

_core_api dbindst_t dbind_container_imp(const char_t *type, const bool_t store_pointers, FPtr_container_create func_create, FPtr_container_size func_size, FPtr_container_get func_get, FPtr_container_insert func_insert, FPtr_container_delete func_delete, FPtr_container_destroy func_destroy);

/* DBind info for registered types */

_core_api const DBind *dbind_from_typename(const char_t *type, bool_t *is_ptr);

_core_api dtype_t dbind_type(const DBind *bind);

_core_api uint16_t dbind_size(const DBind *bind);

_core_api const char_t *dbind_typename(const DBind *bind);

_core_api uint32_t dbind_enum_count(const DBind *bind);

_core_api uint32_t dbind_enum_index(const DBind *bind, const enum_t value);

_core_api enum_t dbind_enum_index_value(const DBind *bind, const uint32_t index);

_core_api const char_t *dbind_enum_alias(const DBind *bind, const uint32_t index);

_core_api bool_t dbind_container_is_ptr(const DBind *bind);

_core_api const DBind *dbind_container_type(const DBind *bind, const char_t *type);

_core_api bool_t dbind_is_basic_type(const dtype_t type);

_core_api bool_t dbind_is_number_type(const dtype_t type);

/* DBind info for registered struct members */

_core_api const DBind *dbind_st_member(const DBind *stbind, const uint32_t member_id);

_core_api const DBind *dbind_st_ebind(const DBind *stbind, const uint32_t member_id);

_core_api uint32_t dbind_st_count(const DBind *stbind);

_core_api uint32_t dbind_st_member_id(const DBind *stbind, const char_t *mname);

_core_api uint16_t dbind_st_offset(const DBind *stbind, const uint32_t member_id);

_core_api const char_t *dbind_st_mname(const DBind *stbind, const uint32_t member_id);

_core_api bool_t dbind_st_is_str_dptr(const DBind *stbind, const uint32_t member_id);

_core_api void dbind_st_int_range(const DBind *stbind, const uint32_t member_id, int64_t *min, int64_t *max);

_core_api void dbind_st_real_range(const DBind *stbind, const uint32_t member_id, real64_t *min, real64_t *max);

_core_api const char_t *dbind_st_real_format(const DBind *stbind, const uint32_t member_id);

_core_api const void *dbind_st_binary_default(const DBind *stbind, const uint32_t member_id);

_core_api bool_t dbind_st_str_filter(const DBind *stbind, const uint32_t member_id, const char_t *src, char_t *dest, const uint32_t size);

/* DBind object construct/destruct */

_core_api byte_t *dbind_create_data(const DBind *bind, const DBind *ebind);

_core_api void dbind_init_data(const DBind *bind, byte_t *data);

_core_api void dbind_remove_data(byte_t *data, const DBind *bind);

_core_api void dbind_destroy_data(byte_t **data, const DBind *bind, const DBind *ebind);

/* DBind object memory access: Copy struct member values */

_core_api void dbind_st_store_field(const DBind *stbind, const uint32_t member_id, const byte_t *obj, byte_t *dest, const uint32_t dest_size);

_core_api void dbind_st_restore_field(const DBind *stbind, const uint32_t member_id, byte_t *obj, const byte_t *src, const uint32_t src_size);

_core_api void dbind_st_remove_field(const DBind *stbind, const uint32_t member_id, byte_t *dest, const uint32_t dest_size);

_core_api bool_t dbind_st_overlaps_field(const DBind *stbind, const uint32_t member_id, const byte_t *obj, const byte_t *data, const uint32_t data_size);

/* DBind object memory access: Read values */

_core_api bool_t dbind_get_bool_value(const DBind *bind, const byte_t *data);

_core_api bool_t dbind_st_get_bool_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api int64_t dbind_get_int_value(const DBind *bind, const byte_t *data);

_core_api int64_t dbind_st_get_int_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api real64_t dbind_get_real_value(const DBind *bind, const byte_t *data);

_core_api real64_t dbind_st_get_real_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api enum_t dbind_get_enum_value(const DBind *bind, const byte_t *data);

_core_api enum_t dbind_st_get_enum_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api const char_t *dbind_get_str_value(const DBind *bind, const byte_t *data);

_core_api const char_t *dbind_st_get_str_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api byte_t *dbind_st_get_struct_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api byte_t *dbind_st_create_struct_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api byte_t *dbind_st_get_container_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api const void *dbind_get_binary_value(const DBind *bind, const byte_t *data);

_core_api const void *dbind_st_get_binary_value(const DBind *stbind, const uint32_t member_id, const byte_t *obj);

_core_api void dbind_write_binary_value(const DBind *bind, Stream *stm, const byte_t *data);

_core_api void dbind_st_write_binary_value(const DBind *stbind, const uint32_t member_id, Stream *stm, const byte_t *obj);

/* DBind object memory access: Write/convert values */

_core_api bindset_t dbind_set_value_null(const DBind *bind, const DBind *ebind, const bool_t is_str_dptr, byte_t *data);

_core_api bindset_t dbind_st_set_value_null(const DBind *stbind, const uint32_t member_id, byte_t *obj);

_core_api bindset_t dbind_set_value_bool(const DBind *bind, byte_t *data, const bool_t value);

_core_api bindset_t dbind_st_set_value_bool(const DBind *stbind, const uint32_t member_id, byte_t *obj, const bool_t value);

_core_api bindset_t dbind_set_value_int(const DBind *bind, byte_t *data, const int64_t value);

_core_api bindset_t dbind_st_set_value_int(const DBind *stbind, const uint32_t member_id, byte_t *obj, const int64_t value);

_core_api bindset_t dbind_set_value_real(const DBind *bind, byte_t *data, const real64_t value);

_core_api bindset_t dbind_st_set_value_real(const DBind *stbind, const uint32_t member_id, byte_t *obj, const real64_t value);

_core_api bindset_t dbind_st_set_value_norm(const DBind *stbind, const uint32_t member_id, byte_t *obj, const real64_t norm_value);

_core_api bindset_t dbind_st_set_value_incr(const DBind *stbind, const uint32_t member_id, byte_t *obj, const bool_t positive_incr);

_core_api bindset_t dbind_set_value_str(const DBind *bind, byte_t *data, const char_t *value);

_core_api bindset_t dbind_st_set_value_str(const DBind *stbind, const uint32_t member_id, byte_t *obj, const char_t *value);

_core_api bindset_t dbind_set_value_binary(const DBind *bind, byte_t *data, const void *value);

_core_api bindset_t dbind_st_set_value_binary(const DBind *stbind, const uint32_t member_id, byte_t *obj, const void *value);

_core_api bindset_t dbind_create_value_binary(const DBind *bind, byte_t *data, const byte_t *bindata, const uint32_t binsize);

_core_api bindset_t dbind_st_create_value_binary(const DBind *stbind, const uint32_t member_id, byte_t *obj, const byte_t *bindata, const uint32_t binsize);

/* DBind object memory access: Container operations */

_core_api byte_t *dbind_container_create(const DBind *bind, const DBind *ebind);

_core_api uint32_t dbind_container_size(const DBind *bind, const byte_t *cont);

_core_api byte_t *dbind_container_append(const DBind *bind, const DBind *ebind, byte_t *cont);

_core_api byte_t *dbind_container_get(const DBind *bind, const DBind *ebind, const uint32_t pos, byte_t *cont);

_core_api const byte_t *dbind_container_cget(const DBind *bind, const DBind *ebind, const uint32_t pos, const byte_t *cont);

_core_api void dbind_container_delete(const DBind *bind, const DBind *ebind, const uint32_t pos, byte_t *cont);

__END_C

#define dbind_bool(type) \
    ( \
        dbind_bool_imp( \
            cast_const(#type, char_t), \
            sizeof(type)))

#define dbind_int(type, is_signed) \
    ( \
        dbind_int_imp( \
            cast_const(#type, char_t), \
            sizeof(type), \
            is_signed))

#define dbind_real(type) \
    ( \
        dbind_real_imp( \
            cast_const(#type, char_t), \
            sizeof(type)))

#define dbind_string(type, func_create, func_destroy, func_get, func_read, func_write, def) \
    ( \
        FUNC_CHECK_STR_CREATE(func_create, type), \
        FUNC_CHECK_DESTROY(func_destroy, type), \
        FUNC_CHECK_STR_GET(func_get, type), \
        FUNC_CHECK_READ(func_read, type), \
        FUNC_CHECK_WRITE(func_write, type), \
        dbind_string_imp( \
            cast_const(#type, char_t), \
            (FPtr_str_create)func_create, \
            (FPtr_destroy)func_destroy, \
            (FPtr_str_get)func_get, \
            (FPtr_read)func_read, \
            (FPtr_write)func_write, \
            def))

#define dbind_container(type, store_pointers, func_create, func_size, func_get, func_insert, func_delete, func_destroy) \
    ( \
        FUNC_CHECK_CONTAINER_CREATE(func_create), \
        FUNC_CHECK_CONTAINER_SIZE(func_size), \
        FUNC_CHECK_CONTAINER_GET(func_get), \
        FUNC_CHECK_CONTAINER_INSERT(func_insert), \
        FUNC_CHECK_CONTAINER_DELETE(func_delete), \
        FUNC_CHECK_CONTAINER_DESTROY(func_destroy), \
        dbind_container_imp( \
            type, store_pointers, \
            (FPtr_container_create)func_create, \
            (FPtr_container_size)func_size, \
            (FPtr_container_get)func_get, \
            (FPtr_container_insert)func_insert, \
            (FPtr_container_delete)func_delete, \
            (FPtr_container_destroy)func_destroy))
