/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dbind.inl
 *
 */

/* Data bindings */

#include "core.ixx"

__EXTERN_C

void _dbind_start(void);

void _dbind_finish(void);

void dbind_opaque_imp(const char_t *type, FPtr_data func_data, FPtr_buffer func_buffer, FPtr_copy func_copy, FPtr_read func_read, FPtr_write func_write, FPtr_destroy func_destroy);

dtype_t _dbind_type(const char_t *type, String **subtype, uint16_t *size);

uint16_t _dbind_struct_size(const char_t *type);

bool_t _dbind_struct(const char_t *type, uint16_t *size, uint32_t *nmembers);

bool_t _dbind_member_i(const char_t *type, const uint32_t i, const char_t **name, uint16_t *offset, dtype_t *mtype, const char_t **subtype);

DBind* _dbind_member(const char_t *type, const char_t *name);

dtype_t _dbind_member_type(const DBind *dbind);

uint16_t _dbind_member_offset(const DBind *dbind);

uint16_t _dbind_member_size(const DBind *dbind);

const char_t *_dbind_member_subtype(const DBind *dbind);

void _dbind_member_get(const DBind *dbind, uint16_t *offset, dtype_t *mtype, uint16_t *msize, const char_t **subtype);

const char_t *_dbind_real32_format(const DBind *dbind);

const char_t *_dbind_real64_format(const DBind *dbind);

void _dbind_real32_range(const DBind *dbind, real32_t *min, real32_t *max);

void _dbind_real64_range(const DBind *dbind, real64_t *min, real64_t *max);

void _dbind_int8_range(const DBind *dbind, int8_t *min, int8_t *max);

void _dbind_int16_range(const DBind *dbind, int16_t *min, int16_t *max);

void _dbind_int32_range(const DBind *dbind, int32_t *min, int32_t *max);

void _dbind_int64_range(const DBind *dbind, int64_t *min, int64_t *max);

void _dbind_uint8_range(const DBind *dbind, uint8_t *min, uint8_t *max);

void _dbind_uint16_range(const DBind *dbind, uint16_t *min, uint16_t *max);

void _dbind_uint32_range(const DBind *dbind, uint32_t *min, uint32_t *max);

void _dbind_uint64_range(const DBind *dbind, uint64_t *min, uint64_t *max);

StBind* _dbind_stbind(const char_t *type);

StBind* _dbind_get_stbind(const DBind *dbind);

const char_t* _dbind_stbind_type(const StBind *stbind);

uint16_t _dbind_stbind_size(const StBind *stbind);

uint32_t _dbind_stbind_members(const StBind *stbind);

void _dbind_stbind_member_i(const StBind *stbind, const uint32_t i, const char_t **name, uint16_t *offset, dtype_t *mtype, const char_t **subtype);

void _dbind_opaque(const char_t *type, const byte_t *data, const uint32_t size, void **obj);

void _dbind_opaque_upd(const char_t *type, void *new_obj, void **obj);

bool_t _dbind_filter_string(DBind *dbind, const char_t *src, char_t *dest, const uint32_t size);

real32_t _dbind_string_to_real32(DBind *dbind, const real32_t value, const char_t *src);

real64_t _dbind_string_to_real64(DBind *dbind, const real64_t value, const char_t *src);

int8_t _dbind_string_to_int8(DBind *dbind, const int8_t value, const char_t *src);

int16_t _dbind_string_to_int16(DBind *dbind, const int16_t value, const char_t *src);

int32_t _dbind_string_to_int32(DBind *dbind, const int32_t value, const char_t *src);

int64_t _dbind_string_to_int64(DBind *dbind, const int64_t value, const char_t *src);

uint8_t _dbind_string_to_uint8(DBind *dbind, const uint8_t value, const char_t *src);

uint16_t _dbind_string_to_uint16(DBind *dbind, const uint16_t value, const char_t *src);

uint32_t _dbind_string_to_uint32(DBind *dbind, const uint32_t value, const char_t *src);

uint64_t _dbind_string_to_uint64(DBind *dbind, const uint64_t value, const char_t *src);

real32_t _dbind_real32(DBind *dbind, const real32_t value);

real64_t _dbind_real64(DBind *dbind, const real64_t value);

int8_t _dbind_int8(const DBind *dbind, const int8_t value);

int16_t _dbind_int16(const DBind* dbind, const int16_t value);

int32_t _dbind_int32(const DBind* dbind, const int32_t value);

int64_t _dbind_int64(const DBind* dbind, const int64_t value);

uint8_t _dbind_uint8(const DBind* dbind, const uint8_t value);

uint16_t _dbind_uint16(const DBind* dbind, const uint16_t value);

uint32_t _dbind_uint32(const DBind* dbind, const uint32_t value);

uint64_t _dbind_uint64(const DBind* dbind, const uint64_t value);

real32_t _dbind_incr_real32(DBind *dbind, const real32_t value, const bool_t pos);

real64_t _dbind_incr_real64(DBind *dbind, const real64_t value, const bool_t pos);

int8_t _dbind_incr_int8(DBind *dbind, const int8_t value, const bool_t pos);

int16_t _dbind_incr_int16(DBind *dbind, const int16_t value, const bool_t pos);

int32_t _dbind_incr_int32(DBind *dbind, const int32_t value, const bool_t pos);

int64_t _dbind_incr_int64(DBind *dbind, const int64_t value, const bool_t pos);

uint8_t _dbind_incr_uint8(DBind *dbind, const uint8_t value, const bool_t pos);

uint16_t _dbind_incr_uint16(DBind *dbind, const uint16_t value, const bool_t pos);

uint32_t _dbind_incr_uint32(DBind *dbind, const uint32_t value, const bool_t pos);

uint64_t _dbind_incr_uint64(DBind *dbind, const uint64_t value, const bool_t pos);

void* _dbind_default_opaque(DBind *dbind);

bool_t _dbind_type_is_number(const dtype_t type);

bool_t _dbind_type_is_integer(const dtype_t type);

bool_t _dbind_is_basic_type(const DBind* dbind);

bool_t _dbind_is_number_type(const DBind *dbind);

bool_t _dbind_is_enum_type(const DBind *dbind);

uint32_t _dbind_enum_size(const DBind *dbind);

enum_t _dbind_enum_value(const DBind *dbind, const uint32_t index);

uint32_t _dbind_enum_index(const DBind *dbind, const enum_t value);

const char_t *_dbind_enum_alias(const DBind *dbind, const uint32_t index);

__END_C

#define dbind_opaque(type, func_data, func_buffer, func_copy, func_read, func_write, func_destroy)\
    (\
        FUNC_CHECK_DATA(func_data, type),\
        FUNC_CHECK_BUFFER(func_buffer, type),\
        FUNC_CHECK_COPY(func_copy, type),\
        FUNC_CHECK_READ(func_read, type),\
        FUNC_CHECK_WRITE(func_write, type),\
        FUNC_CHECK_DESTROY(func_destroy, type),\
        dbind_opaque_imp(\
                (const char_t*)#type,\
                (FPtr_data)func_data,\
                (FPtr_buffer)func_buffer,\
                (FPtr_copy)func_copy,\
                (FPtr_read)func_read,\
                (FPtr_write)func_write,\
                (FPtr_destroy)func_destroy)\
    )



