/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: objh.h
 *
 */

/* Undocumented (hidden) API about generic object utilities */

#include "core.hxx"

__EXTERN_C

_core_api void obj_init(Object *object);

_core_api byte_t *obj_new_imp(const uint32_t size, const char_t *type);

_core_api byte_t *obj_new0_imp(const uint32_t size, const char_t *type);

_core_api void *obj_retain_imp(const void *object);

_core_api void *obj_retain_optional_imp(const void *object);

_core_api void obj_remove(Object *object);

_core_api void obj_release_imp(void **object);

_core_api void obj_release_optional_imp(void **object);

_core_api void obj_delete_imp(byte_t **object, const uint32_t size, const char_t *type);

_core_api Listener *obj_listener_imp(void *object, FPtr_event_handler func_event_handler);

__END_C

#define obj_new(type) \
    cast(obj_new_imp(sizeof(type), #type), type)

#define obj_new0(type) \
    cast(obj_new0_imp(sizeof(type), #type), type)

#define obj_retain(object, type) \
    ((void)(cast(object, type) == (object)), \
     cast(obj_retain_imp(cast(object, void)), type))

#define obj_retain_optional(object, type) \
    ((void)(cast(object, type) == (object)), \
     cast(obj_retain_optional_imp(cast(object, void)), type))

#define obj_release(object, type) \
    ((void)(dcast(object, type) == (object)), \
     obj_release_imp(dcast(object, void)))

#define obj_release_optional(object, type) \
    ((void)(dcast(object, type) == (object)), \
     obj_release_optional_imp(dcast(object, void)))

#define obj_release_const_optional(object, type) \
    ((void)(dcast_const(object, type) == (object)), \
     obj_release_optional_imp(dcast(object, void)))

#define obj_delete(object, type) \
    ((void)(dcast(object, type) == (object)), \
     obj_delete_imp(dcast(object, byte_t), sizeof(type), cast_const(#type, char_t)))

#define obj_listener(object, func_event_handler, type) \
    ((void)(cast(object, type) == (object)), \
     FUNC_CHECK_EVENT_HANDLER(func_event_handler, type), \
     obj_listener_imp(cast(object, void), (FPtr_event_handler)(func_event_handler)))
