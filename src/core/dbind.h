/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dbind.h
 * https://nappgui.com/en/core/dbind.html
 *
 */

/* Data binding */

#include "core.hxx"

__EXTERN_C

_core_api dbindst_t dbind_imp(const char_t *type, const uint16_t size, const char_t *mname, const char_t *mtype, const uint16_t moffset, const uint16_t msize);

_core_api dbindst_t dbind_enum_imp(const char_t *type, const char_t *name, const enum_t value, const char_t *alias);

_core_api dbindst_t dbind_binary_imp(const char_t *type, FPtr_copy func_copy, FPtr_read func_read, FPtr_write func_write, FPtr_destroy func_destroy);

_core_api dbindst_t dbind_alias_imp(const char_t *type, const char_t *alias, const uint16_t type_size, const uint16_t alias_size);

_core_api dbindst_t dbind_unreg_imp(const char_t *type);

_core_api byte_t *dbind_create_imp(const char_t *type);

_core_api byte_t *dbind_copy_imp(const byte_t *obj, const char_t *type);

_core_api void dbind_init_imp(byte_t *obj, const char_t *type);

_core_api void dbind_remove_imp(byte_t *obj, const char_t *type);

_core_api void dbind_destroy_imp(byte_t **obj, const char_t *type);

_core_api void dbind_destopt_imp(byte_t **obj, const char_t *type);

_core_api int dbind_cmp_imp(const byte_t *obj1, const byte_t *obj2, const char_t *type);

_core_api bool_t dbind_equ_imp(const byte_t *obj1, const byte_t *obj2, const char_t *type);

_core_api byte_t *dbind_read_imp(Stream *stm, const char_t *type);

_core_api void dbind_write_imp(Stream *stm, const void *obj, const char_t *type);

_core_api void dbind_default_imp(const char_t *type, const char_t *mname, const byte_t *value);

_core_api void dbind_range_imp(const char_t *type, const char_t *mname, const byte_t *min, const byte_t *max);

_core_api void dbind_precision_imp(const char_t *type, const char_t *mname, const byte_t *prec);

_core_api void dbind_increment_imp(const char_t *type, const char_t *mname, const byte_t *incr);

_core_api void dbind_suffix_imp(const char_t *type, const char_t *mname, const char_t *suffix);

__END_C

#define dbind(type, mtype, mname) \
    ( \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype), \
        dbind_imp( \
            cast_const(#type, char_t), \
            (uint16_t)sizeof(type), \
            cast_const(#mname, char_t), \
            cast_const(#mtype, char_t), \
            (uint16_t)STRUCT_MEMBER_OFFSET(type, mname), \
            (uint16_t)STRUCT_MEMBER_SIZE(type, mname)))

#define dbind_enum(type, value, alias) \
    ( \
        (void)(((type)value) == value), \
        dbind_enum_imp( \
            cast_const(#type, char_t), \
            cast_const(#value, char_t), \
            (enum_t)value, \
            alias))

#define dbind_binary(type, func_copy, func_read, func_write, func_destroy) \
    ( \
        FUNC_CHECK_COPY(func_copy, type), \
        FUNC_CHECK_READ(func_read, type), \
        FUNC_CHECK_WRITE(func_write, type), \
        FUNC_CHECK_DESTROY(func_destroy, type), \
        dbind_binary_imp( \
            (const char_t *)#type, \
            (FPtr_copy)func_copy, \
            (FPtr_read)func_read, \
            (FPtr_write)func_write, \
            (FPtr_destroy)func_destroy))

#define dbind_alias(type, alias) \
    ( \
        dbind_alias_imp( \
            (const char_t *)#type, \
            (const char_t *)#alias, \
            (uint16_t)sizeof(type), \
            (uint16_t)sizeof(alias)))

#define dbind_unreg(type) \
    ( \
        dbind_unreg_imp( \
            (const char_t *)#type))

#define dbind_create(type) \
    cast(dbind_create_imp(cast_const(#type, char_t)), type)

#define dbind_copy(obj, type) \
    ((void)(obj == cast(obj, type)), \
     cast(dbind_copy_imp(cast_const(obj, byte_t), cast_const(#type, char_t)), type))

#define dbind_init(obj, type) \
    ((void)(obj == cast(obj, type)), \
     dbind_init_imp(cast(obj, byte_t), cast_const(#type, char_t)))

#define dbind_remove(obj, type) \
    ((void)(obj == cast(obj, type)), \
     dbind_remove_imp(cast(obj, byte_t), cast_const(#type, char_t)))

#define dbind_destroy(obj, type) \
    ((void)(obj == dcast(obj, type)), \
     dbind_destroy_imp(dcast(obj, byte_t), cast_const(#type, char_t)))

#define dbind_destopt(obj, type) \
    ((void)(obj == dcast(obj, type)), \
     dbind_destopt_imp(dcast(obj, byte_t), cast_const(#type, char_t)))

#define dbind_cmp(obj1, obj2, type) \
    ((void)(cast_const(obj1, type) == obj1), \
     (void)(cast_const(obj2, type) == obj2), \
     dbind_cmp_imp(cast_const(obj1, byte_t), cast_const(obj2, byte_t), cast_const(#type, char_t)))

#define dbind_equ(obj1, obj2, type) \
    ((void)(cast_const(obj1, type) == obj1), \
     (void)(cast_const(obj2, type) == obj2), \
     dbind_equ_imp(cast_const(obj1, byte_t), cast_const(obj2, byte_t), cast_const(#type, char_t)))

#define dbind_read(stm, type) \
    cast(dbind_read_imp(stm, cast_const(#type, char_t)), type)

#define dbind_write(stm, obj, type) \
    ((void)(cast_const(obj, type) == obj), \
     dbind_write_imp(stm, cast_const(obj, void), cast_const(#type, char_t)))

#define dbind_default(type, mtype, mname, value) \
    { \
        const mtype ___value = (const mtype)value; \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype); \
        dbind_default_imp( \
            cast_const(#type, char_t), \
            cast_const(#mname, char_t), \
            cast(&___value, byte_t)); \
    }

#define dbind_range(type, mtype, mname, min, max) \
    { \
        mtype ___min = min; \
        mtype ___max = max; \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype); \
        dbind_range_imp( \
            cast_const(#type, char_t), \
            cast_const(#mname, char_t), \
            cast(&___min, byte_t), \
            cast(&___max, byte_t)); \
    }

#define dbind_precision(type, mtype, mname, prec) \
    { \
        mtype ___prec = prec; \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype); \
        dbind_precision_imp( \
            cast_const(#type, char_t), \
            cast_const(#mname, char_t), \
            cast(&___prec, byte_t)); \
    }

#define dbind_increment(type, mtype, mname, incr) \
    { \
        mtype ___incr = incr; \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype); \
        dbind_increment_imp( \
            cast_const(#type, char_t), \
            cast_const(#mname, char_t), \
            cast(&___incr, byte_t)); \
    }

#define dbind_suffix(type, mtype, mname, suffix) \
    ( \
        CHECK_STRUCT_MEMBER_TYPE(type, mname, mtype), \
        dbind_suffix_imp( \
            cast_const(#type, char_t), \
            cast_const(#mname, char_t), \
            suffix))
