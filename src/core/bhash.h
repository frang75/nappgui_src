/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bhash.h
 *
 */

/* Hash code from memory block */

#include "core.hxx"

__EXTERN_C

_core_api uint32_t bhash_from_block(const byte_t *data, const uint32_t size);

_core_api uint32_t bhash_append_uint32(const uint32_t hash, const uint32_t value);

_core_api uint32_t bhash_append_real32(const uint32_t hash, const real32_t value);

__END_C

#define bhash_from_object(data, type) \
    ((void)(cast(data, type) == (data)), \
     bhash_from_block(cast_const(data, byte_t), sizeof(type)))

#define bhash_append_enum(hash, value, type) \
    ((void)((enum type)value == value), \
     bhash_append_uint32(hash, (uint32_t)value))
