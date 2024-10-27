/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: json.h
 * https://nappgui.com/en/inet/json.html
 *
 */

/* Json parser */

#include "inet.hxx"

__EXTERN_C

_inet_api void *json_read_imp(Stream *stm, const JsonOpts *opts, const char_t *type);

_inet_api void json_write_imp(Stream *stm, const void *data, const JsonOpts *opts, const char_t *type);

_inet_api void json_destroy_imp(void **data, const char_t *type);

_inet_api void json_destopt_imp(void **data, const char_t *type);

__END_C

#define json_read(stm, opts, type) \
    cast(json_read_imp(stm, opts, cast_const(#type, char_t)), type)

#define json_write(stm, data, opts, type) \
    ((void)(cast_const(data, type) == data), \
     json_write_imp(stm, cast_const(data, void), opts, cast_const(#type, char_t)))

#define json_destroy(data, type) \
    ((void)(dcast(data, type) == data), \
     json_destroy_imp(dcast(data, void), cast_const(#type, char_t)))

#define json_destopt(data, type) \
    ((void)(dcast(data, type) == data), \
     json_destopt_imp(dcast(data, void), cast_const(#type, char_t)))
