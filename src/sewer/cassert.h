/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: cassert.h
 * https://nappgui.com/en/sewer/cassert.html
 *
 */

/* Code assertion */

#include "sewer.hxx"

__EXTERN_C

_sewer_api void cassert_set_func(void *data, FPtr_assert func_assert);

_sewer_api void cassert_imp(bool_t cond, const char_t *detail, const char_t *file, const uint32_t line);

_sewer_api void cassert_fatal_imp(bool_t cond, const char_t *detail, const char_t *file, const uint32_t line);

_sewer_api void cassert_no_null_imp(void *ptr, const char_t *detail, const char_t *file, const uint32_t line);

_sewer_api void cassert_no_nullf_imp(void (*ptr)(void), const char_t *detail, const char_t *file, const uint32_t line);

_sewer_api void cassert_default_imp(const char_t *file, const uint32_t line, const int32_t value);

__END_C

#if defined(__ASSERTS__)

#define cassert(cond) \
    cassert_imp((bool_t)(cond), #cond, __FILE__, __LINE__)

#define cassert_msg(cond, msg) \
    cassert_imp((bool_t)(cond), #msg, __FILE__, __LINE__)

#define cassert_fatal(cond) \
    cassert_fatal_imp((bool_t)(cond), #cond, __FILE__, __LINE__)

#define cassert_fatal_msg(cond, msg) \
    cassert_fatal_imp((bool_t)(cond), #msg, __FILE__, __LINE__)

#define cassert_no_null(ptr) \
    cassert_no_null_imp(cast(ptr, void), #ptr, __FILE__, __LINE__)

#define cassert_no_nullf(fptr) \
    if (fptr == NULL) \
    cassert_no_null_imp(NULL, #fptr, __FILE__, __LINE__)

#define cassert_default(value) \
    cassert_default_imp(__FILE__, __LINE__, (int32_t)value)

#define cassert_unref(cond, v) \
    cassert_imp((bool_t)(cond), #cond, __FILE__, __LINE__)

#else

#define cassert(cond) (void)(1 == 1)
#define cassert_msg(cond, msg) (void)(1 == 1)
#define cassert_fatal(cond) (void)(1 == 1)
#define cassert_fatal_msg(cond, msg) (void)(1 == 1)
#define cassert_no_null(ptr) unref(ptr)
#define cassert_no_nullf(fptr) unref(fptr)
#define cassert_default(value) unref(value)
#define cassert_unref(cond, v) unref(v)

#endif
