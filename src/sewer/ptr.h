/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: ptr.h
 * https://nappgui.com/en/sewer/ptr.html
 *
 */

/* Safety pointer manipulation */

#include "sewer.hxx"

__EXTERN_C

_sewer_api void *ptr_dget_imp(void **dptr);

_sewer_api void *ptr_dget_no_null_imp(void **dptr);

_sewer_api void ptr_destopt_imp(void **dptr, FPtr_destroy func_destroy);

_sewer_api void *ptr_copyopt_imp(void *ptr, FPtr_copy func_copy);

__END_C

#if defined(__ASSERTS__)

#define ptr_get(ptr, type) \
    ((void)(cast(ptr, type) == (ptr)), \
     cassert_no_null((ptr)), \
     *cast(ptr, type))

#else

#define ptr_get(ptr, type) \
    ((void)(cast(ptr, type) == (ptr)), \
     *(ptr))

#endif

#define ptr_dget(dptr, type) \
    ((void)(dcast(dptr, type) == (dptr)), \
     cast(ptr_dget_imp(dcast(dptr, void)), type))

#define ptr_dget_no_null(dptr, type) \
    ((void)(dcast(dptr, type) == (dptr)), \
     cast(ptr_dget_no_null_imp(dcast(dptr, void)), type))

#define ptr_assign(dest, src) \
    if ((dest) != NULL) \
    (*dest) = (src)

#define ptr_destopt(func_destroy, dptr, type) \
    ((void)(dcast(dptr, type) == (dptr)), \
     FUNC_CHECK_DESTROY(func_destroy, type), \
     ptr_destopt_imp(dcast(dptr, void), (FPtr_destroy)func_destroy))

#define ptr_copyopt(func_copy, ptr, type) \
    ((void)(cast(ptr, type) == (ptr)), \
     FUNC_CHECK_COPY(func_copy, type), \
     cast(ptr_copyopt_imp(dcast(ptr, void), (FPtr_copy)func_copy), type))
