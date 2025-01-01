/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: coreh.hxx
 *
 */

/* Undocumented (hidden) Core library types */

#ifndef __COREH_HXX__
#define __COREH_HXX__

#include "core.hxx"

typedef enum _dtype_t
{
    ekDTYPE_BOOL,
    ekDTYPE_INT,
    ekDTYPE_REAL,
    ekDTYPE_ENUM,
    ekDTYPE_STRING,
    ekDTYPE_STRUCT,
    ekDTYPE_BINARY,
    ekDTYPE_CONTAINER,
    ekDTYPE_UNKNOWN
} dtype_t;

typedef enum _bindset_t
{
    ekBINDSET_OK,
    ekBINDSET_UNCHANGED,
    ekBINDSET_NOT_ALLOWED
} bindset_t;

typedef struct _dbind_t DBind;

typedef void *(*FPtr_str_create)(const char_t *str);
#define FUNC_CHECK_STR_CREATE(func, type) \
    (void)((type * (*)(const char_t *)) func == func)

typedef const char_t *(*FPtr_str_get)(const void *);
#define FUNC_CHECK_STR_GET(func, type) \
    (void)((const char_t *(*)(const type *))func == func)

typedef byte_t *(*FPtr_container_create)(const char_t *type, const uint16_t esize);
#define FUNC_CHECK_CONTAINER_CREATE(func) \
    (void)((byte_t * (*)(const char_t *, const uint16_t)) func == func)

typedef uint32_t (*FPtr_container_size)(const byte_t *cont);
#define FUNC_CHECK_CONTAINER_SIZE(func) \
    (void)((uint32_t(*)(const byte_t *))func == func)

typedef byte_t *(*FPtr_container_get)(byte_t *cont, const uint32_t pos, const char_t *type, const uint16_t esize);
#define FUNC_CHECK_CONTAINER_GET(func) \
    (void)((byte_t * (*)(byte_t *, const uint32_t, const char_t *, const uint16_t)) func == func)

typedef byte_t *(*FPtr_container_insert)(byte_t *cont, const uint32_t pos, const uint32_t n, const char_t *type, const uint16_t esize);
#define FUNC_CHECK_CONTAINER_INSERT(func) \
    (void)((byte_t * (*)(byte_t *, const uint32_t, const uint32_t, const char_t *, const uint16_t)) func == func)

typedef void (*FPtr_container_delete)(byte_t *cont, const uint32_t pos, const char_t *type, const uint16_t esize);
#define FUNC_CHECK_CONTAINER_DELETE(func) \
    (void)((void (*)(byte_t *, const uint32_t, const char_t *, const uint16_t))func == func)

typedef void (*FPtr_container_destroy)(byte_t **cont, const char_t *type);
#define FUNC_CHECK_CONTAINER_DESTROY(func) \
    (void)((void (*)(byte_t **, const char_t *))func == func)

typedef void *(*FPtr_from_data)(const byte_t *data, const uint32_t size);
#define FUNC_CHECK_FROM_DATA(func, type) \
    (void)((type * (*)(const byte_t *, const uint32_t)) func == func)

#endif
