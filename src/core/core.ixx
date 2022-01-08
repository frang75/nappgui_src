/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: core.ixx
 *
 */

/* Core library */

#ifndef __CORE_IXX__
#define __CORE_IXX__

#include "core.hxx"

typedef enum _dtype_t
{
    ekDTYPE_BOOL,
    ekDTYPE_INT8,
    ekDTYPE_INT16,
    ekDTYPE_INT32,
    ekDTYPE_INT64,
    ekDTYPE_UINT8,
    ekDTYPE_UINT16,
    ekDTYPE_UINT32,
    ekDTYPE_UINT64,
    ekDTYPE_REAL32,
    ekDTYPE_REAL64,
    ekDTYPE_ENUM,
    ekDTYPE_STRING,
    ekDTYPE_STRING_PTR,
    ekDTYPE_ARRAY,
    ekDTYPE_ARRPTR,
    ekDTYPE_OBJECT,
    ekDTYPE_OBJECT_PTR,
    ekDTYPE_OBJECT_OPAQUE,
    ekDTYPE_UNKNOWN
} dtype_t;

typedef struct _object_t Object;
typedef struct _nfa_t NFA;
typedef struct _evassert_t EvAssert;
typedef struct _lexscn_t LexScn;
typedef struct _dbind_t DBind;
typedef struct _stbind_t StBind;
typedef struct _enumbind_t EnumBind;
typedef struct _enumvbind_t EnumVBind;

typedef void*(*FPtr_data)(const byte_t *data, const uint32_t size);
#define FUNC_CHECK_DATA(func, type)\
    (void)((type*(*)(const byte_t*, const uint32_t))func == func)

typedef Buffer*(*FPtr_buffer)(const void *obj);
#define FUNC_CHECK_BUFFER(func, type)\
    (void)((Buffer*(*)(const type*))func == func)

typedef void*(*FPtr_retain)(const void *item);
#define FUNC_CHECK_RETAIN(func, type)\
    (void)((type*(*)(const type*))func == func)

typedef void(*FPtr_release)(void **item);
#define FUNC_CHECK_RELEASE(func, type)\
    (void)((void(*)(type**))func == func)

typedef void*(*FPtr_create_from_data)(const byte_t *data, const uint32_t size);
#define FUNC_CHECK_CREATE_FROM_DATA(func, type)\
    (void)((type*(*)(const byte_t*, const uint32_t))func == func)

struct _object_t
{
    uint32_t retain_count;
};

struct _evassert_t
{
    uint32_t group;
    const char_t *caption;
    const char_t *detail;
    const char_t *file;
    uint32_t line;
};

DeclPt(NFA);

#endif
