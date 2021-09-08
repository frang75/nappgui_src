/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: obj.c
 *
 */

/* Generic object utilities */

#include "obj.inl"
#include "event.inl"
#include "event.h"
#include "cassert.h"
#include "heap.h"

/*---------------------------------------------------------------------------*/

void obj_init(Object *object)
{
    cassert_no_null(object);
    object->retain_count = 0;
}

/*---------------------------------------------------------------------------*/

byte_t *obj_new_imp(const uint32_t size, const char_t *type)
{
    Object *object = (Object*)heap_malloc(size, type);
    object->retain_count = 0;
    return (byte_t*)object;
}

/*---------------------------------------------------------------------------*/

byte_t *obj_new0_imp(const uint32_t size, const char_t *type)
{
    Object *object = (Object*)heap_calloc(size, type);
    object->retain_count = 0;
    return (byte_t*)object;
}

/*---------------------------------------------------------------------------*/

void *obj_retain_imp(const void *object)
{
    cassert_no_null(object);
    ((Object*)object)->retain_count += 1;
    return (void*)object;
}

/*---------------------------------------------------------------------------*/

void *obj_retain_optional_imp(const void *object)
{    
    if (object != NULL)
        ((Object*)object)->retain_count += 1;
    return (void*)object;
}

/*---------------------------------------------------------------------------*/

uint32_t obj_retain_count_imp(const void *object)
{
    cassert_no_null(object);
    return ((Object*)object)->retain_count;
}

/*---------------------------------------------------------------------------*/

void obj_remove(Object *object)
{
    cassert_no_null(object);
    cassert(object->retain_count == 0);
}

/*---------------------------------------------------------------------------*/

void obj_release_imp(void **object)
{
    cassert_no_null(object);
    cassert_no_null(*object);
    cassert(((Object*)(*object))->retain_count > 0);
    ((Object*)(*object))->retain_count -= 1;
    *object = NULL;
}

/*---------------------------------------------------------------------------*/

void obj_release_optional_imp(void **object)
{
    cassert_no_null(object);
    if (*object != NULL)
    {
        cassert(((Object*)(*object))->retain_count > 0);
        ((Object*)(*object))->retain_count -= 1;
        *object = NULL;
    }
}

/*---------------------------------------------------------------------------*/

void obj_delete_imp(byte_t **object, const uint32_t size, const char_t *type)
{
    Object *obj = NULL;
    cassert_no_null(object);
    obj = (Object*)(*object);
    cassert_no_null(obj);
    cassert(obj->retain_count == 0);
    heap_free(object, size, type);
}

/*---------------------------------------------------------------------------*/

Listener *obj_listener_imp(void *object, FPtr_event_handler func_event_handler)
{
    Listener *listener = listener_imp(object, func_event_handler);
    listener_retain(listener, obj_retain_imp, obj_release_imp);
    return listener;
}

