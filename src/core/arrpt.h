/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrpt.h
 * https://nappgui.com/en/core/arrpt.html
 *
 */

/* Arrays of pointers */

#include "array.h"

#define arrpt_create(type)\
    (ArrPt(type)*)array_create_imp(sizeof(type*), (const char_t*)(ARRPT#type))

#define arrpt_copy(array, func_copy, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_COPY(func_copy, type),\
    (ArrPt(type)*)array_copy_ptr_imp((Array*)(array), (FPtr_copy)(func_copy), (const char_t*)(ARRPT#type)))

#define arrpt_read(stream, func_read, type)\
    (FUNC_CHECK_READ(func_read, type),\
    (ArrPt(type)*)array_read_ptr_imp(stream, (FPtr_read)func_read, (const char_t*)(ARRPT#type)))

#define arrpt_destroy(array, func_destroy, type)\
    ((void)(array == (ArrPt(type)**)(array)),\
    array_destroy_ptr_imp((Array**)(array), (FPtr_destroy)func_destroy, (const char_t*)(ARRPT#type)))

#define arrpt_destopt(array, func_destroy, type)\
    ((void)(array == (ArrPt(type)**)(array)),\
    array_destopt_ptr_imp((Array**)(array), (FPtr_destroy)func_destroy, (const char_t*)(ARRPT#type)))

#define arrpt_clear(array, func_destroy, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    array_clear_ptr_imp((Array*)(array), (FPtr_destroy)func_destroy))

#define arrpt_write(stream, array, func_write, type)\
	((void)((array) == (const ArrPt(type)*)(array)),\
    FUNC_CHECK_WRITE(func_write, type),\
    array_write_ptr_imp(stream, (const Array*)(array), (FPtr_write)func_write))

#define arrpt_size(array, type)\
	((void)((array) == (ArrPt(type)*)(array)),\
	array_size_imp((Array*)(array)))

#define arrpt_get(array, pos, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    *(type**)array_get_imp((Array*)(array), (pos)))

#define arrpt_first(array, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    *(type**)array_get_imp((Array*)(array), (0)))

#define arrpt_last(array, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    *(type**)array_get_last_imp((Array*)(array)))

#define arrpt_all(array, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    (type**)array_all_imp((Array*)(array)))

#define arrpt_grow(array, n, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
	(void)array_insert_imp((Array*)(array), UINT32_MAX, (n)))

#define arrpt_append(array, value, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
	(*(type**)array_insert_imp((Array*)(array), UINT32_MAX, 1)) = (value))

#define arrpt_prepend(array, value, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
	(*(type**)array_insert_imp((Array*)(array), 0, 1)) = (value))

#define arrpt_insert(array, pos, value, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
	(*(type**)array_insert_imp((Array*)(array), (pos), 1)) = (type*)(value))

#define arrpt_join(dest, src, func_copy, type)\
    ((void)((dest) == (ArrPt(type)*)(dest)),\
    (void)((src) == (ArrPt(type)*)(src)),\
    FUNC_CHECK_COPY(func_copy, type),\
    array_join_ptr_imp((Array*)(dest), (const Array*)src, (FPtr_copy)func_copy))

#define arrpt_delete(array, pos, func_destroy, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    array_delete_ptr_imp((Array*)(array), (pos), 1, (FPtr_destroy)func_destroy))

#define arrpt_pop(array, func_destroy, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    array_pop_ptr_imp((Array*)(array), (FPtr_destroy)func_destroy))

#define arrpt_sort(array, func_compare, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_COMPARE(func_compare, type),\
    array_sort_ptr_imp((Array*)(array), (FPtr_compare)func_compare))

#define arrpt_sort_ex(array, func_compare, data, type, dtype)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    (void)((data) == (dtype*)(data)),\
    FUNC_CHECK_COMPARE_EX(func_compare, type, dtype),\
    array_sort_ptr_ex_imp((Array*)(array), (FPtr_compare_ex)func_compare, (void*)(data)))

#define arrpt_find(array, elem, type)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    (void)((type*)(elem) == (elem)),\
    array_find_ptr_imp((const Array*)(array), (const void*)(elem)))

#define arrpt_search(array, func_compare, key, pos, type, ktype)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype),\
    (void)((key) == (ktype*)(key)),\
    (type*)array_search_ptr_imp((const Array*)(array), (FPtr_compare)func_compare, (const void*)key, pos))

#define arrpt_bsearch(array, func_compare, key, pos, type, ktype)\
    ((void)((array) == (ArrPt(type)*)(array)),\
    FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype),\
    (void)((key) == (ktype*)(key)),\
    (type*)array_bsearch_ptr_imp((const Array*)(array), (FPtr_compare)func_compare, (const void*)key, pos))

#define arrpt_foreach(elem, array, type)\
    {\
        register type **elem##_buf = NULL;\
        register uint32_t elem##_i, elem##_total;\
        elem##_buf = arrpt_all((array), type);\
        elem##_total = arrpt_size((array), type);\
        for (elem##_i = 0; elem##_i < elem##_total; ++elem##_i, ++elem##_buf)\
        {\
            register type *elem = (*elem##_buf);

#define arrpt_foreach_rev(elem, array, type)\
    {\
        register type **elem##_buf = NULL;\
        register uint32_t i, elem##_i, elem##_total;\
        elem##_buf = arrpt_all((array), type);\
        elem##_total = arrpt_size((array), type);\
        elem##_buf += elem##_total - 1;\
        elem##_i = elem##_total - 1;\
        for (i = 0; i < elem##_total; ++i, --elem##_i, --elem##_buf)\
        {\
            register type *elem = (*elem##_buf);

#define arrpt_end()\
        }\
    }
