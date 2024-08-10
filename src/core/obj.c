/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: obj.c
 *
 */

/* Generic object utilities */

#include "objh.h"
#include "event.inl"
#include "event.h"
#include "heap.h"
#include <sewer/cassert.h>

/*---------------------------------------------------------------------------*/

void obj_init(Object *object)
{
    cassert_no_null(object);
    object->count = 0;
}

/*---------------------------------------------------------------------------*/

byte_t *obj_new_imp(const uint32_t size, const char_t *type)
{
    Object *object = cast(heap_malloc(size, type), Object);
    object->count = 0;
    return cast(object, byte_t);
}

/*---------------------------------------------------------------------------*/

byte_t *obj_new0_imp(const uint32_t size, const char_t *type)
{
    Object *object = cast(heap_calloc(size, type), Object);
    object->count = 0;
    return cast(object, byte_t);
}

/*---------------------------------------------------------------------------*/

void *obj_retain_imp(const void *object)
{
    cassert_no_null(object);
    cast(object, Object)->count += 1;
    return cast(object, void);
}

/*---------------------------------------------------------------------------*/

void *obj_retain_optional_imp(const void *object)
{
    if (object != NULL)
        cast(object, Object)->count += 1;
    return cast(object, void);
}

/*---------------------------------------------------------------------------*/

void obj_remove(Object *object)
{
    cassert_no_null(object);
    cassert(object->count == 0);
}

/*---------------------------------------------------------------------------*/

void obj_release_imp(void **object)
{
    cassert_no_null(object);
    cassert_no_null(*object);
    cassert((*dcast(object, Object))->count > 0);
    (*dcast(object, Object))->count -= 1;
    *object = NULL;
}

/*---------------------------------------------------------------------------*/

void obj_release_optional_imp(void **object)
{
    cassert_no_null(object);
    if (*object != NULL)
    {
        cassert((*dcast(object, Object))->count > 0);
        (*dcast(object, Object))->count -= 1;
        *object = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void obj_delete_imp(byte_t **object, const uint32_t size, const char_t *type)
{
    cassert_no_null(object);
    cassert_no_null(*object);
    cassert((*dcast(object, Object))->count == 0);
    heap_free(object, size, type);
}

/*---------------------------------------------------------------------------*/

Listener *obj_listener_imp(void *object, FPtr_event_handler func_event_handler)
{
    Listener *listener = listener_imp(object, func_event_handler);
    _listener_retain(listener, obj_retain_imp, obj_release_imp);
    return listener;
}
