/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrpt.h
 * https://nappgui.com/en/core/arrpt.html
 *
 */

/* Pointers arrays */

#define arrpt_create(type) \
    arrpt_##type##_create((uint16_t)sizeof(type *))

#define arrpt_copy(array, func_copy, type) \
    arrpt_##type##_copy(array, func_copy)

#define arrpt_read(stream, func_read, type) \
    arrpt_##type##_read(stream, func_read)

#define arrpt_read_ex(stream, func_read, data, type, dtype) \
    ((void)((data) == cast(data, dtype)), \
     FUNC_CHECK_READ_EX(func_read, type, dtype), \
     arrpt_##type##_read_ex(stream, (FPtr_read_ex)func_read, cast(data, void)))

#define arrpt_destroy(array, func_destroy, type) \
    arrpt_##type##_destroy(array, func_destroy)

#define arrpt_destopt(array, func_destroy, type) \
    arrpt_##type##_destopt(array, func_destroy)

#define arrpt_clear(array, func_destroy, type) \
    arrpt_##type##_clear(array, func_destroy)

#define arrpt_write(stream, array, func_write, type) \
    arrpt_##type##_write(stream, array, func_write)

#define arrpt_write_ex(stream, array, func_write, data, type, dtype) \
    ((void)((data) == cast(data, dtype)), \
     FUNC_CHECK_WRITE_EX(func_write, type, dtype), \
     arrpt_##type##_write_ex(stream, array, (FPtr_write_ex)func_write, cast(data, void)))

#define arrpt_size(array, type) \
    arrpt_##type##_size(array)

#define arrpt_get(array, pos, type) \
    arrpt_##type##_get(array, pos)

#define arrpt_get_const(array, pos, type) \
    arrpt_##type##_get_const(array, pos)

#define arrpt_first(array, type) \
    arrpt_##type##_first(array)

#define arrpt_first_const(array, type) \
    arrpt_##type##_first_const(array)

#define arrpt_last(array, type) \
    arrpt_##type##_last(array)

#define arrpt_last_const(array, type) \
    arrpt_##type##_last_const(array)

#define arrpt_all(array, type) \
    arrpt_##type##_all(array)

#define arrpt_all_const(array, type) \
    arrpt_##type##_all_const(array)

#define arrpt_append(array, value, type) \
    (*arrpt_##type##_insert(array, UINT32_MAX, 1)) = (value)

#define arrpt_prepend(array, value, type) \
    (*arrpt_##type##_insert(array, 0, 1)) = (value)

#define arrpt_insert(array, pos, value, type) \
    (*arrpt_##type##_insert(array, pos, 1)) = (value)

#define arrpt_insert_n(array, pos, n, type) \
    arrpt_##type##_insert(array, pos, n)

#define arrpt_join(dest, src, func_copy, type) \
    arrpt_##type##_join(dest, src, func_copy)

#define arrpt_delete(array, pos, func_destroy, type) \
    arrpt_##type##_delete(array, pos, func_destroy)

#define arrpt_pop(array, func_destroy, type) \
    arrpt_##type##_pop(array, func_destroy)

#define arrpt_sort(array, func_compare, type) \
    arrpt_##type##_sort(array, func_compare)

#define arrpt_sort_ex(array, func_compare, data, type, dtype) \
    ((void)((data) == cast(data, dtype)), \
     FUNC_CHECK_COMPARE_EX(func_compare, type, dtype), \
     arrpt_##type##_sort_ex(array, (FPtr_compare_ex)func_compare, cast(data, void)))

#define arrpt_find(array, elem, type) \
    arrpt_##type##_find(array, elem)

#define arrpt_search(array, func_compare, key, pos, type, ktype) \
    ((void)((key) == cast_const(key, ktype)), \
     FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype), \
     arrpt_##type##_search(array, (FPtr_compare)func_compare, cast_const(key, void), pos))

#define arrpt_search_const(array, func_compare, key, pos, type, ktype) \
    ((void)((key) == cast_const(key, ktype)), \
     FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype), \
     arrpt_##type##_search_const(array, (FPtr_compare)func_compare, cast_const(key, void), pos))

#define arrpt_bsearch(array, func_compare, key, pos, type, ktype) \
    ((void)((key) == cast_const(key, ktype)), \
     FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype), \
     arrpt_##type##_bsearch(array, (FPtr_compare)func_compare, cast_const(key, void), pos))

#define arrpt_bsearch_const(array, func_compare, key, pos, type, ktype) \
    ((void)((key) == cast_const(key, ktype)), \
     FUNC_CHECK_COMPARE_KEY(func_compare, type, ktype), \
     arrpt_##type##_bsearch_const(array, (FPtr_compare)func_compare, cast_const(key, void), pos))

#define arrpt_foreach(elem, array, type) \
    { \
        type **elem##_buf = arrpt_all(array, type); \
        uint32_t elem##_i, elem##_total = arrpt_size(array, type); \
        for (elem##_i = 0; elem##_i < elem##_total; ++elem##_i, ++elem##_buf) \
        { \
            type *elem = (*elem##_buf);

#define arrpt_foreach_const(elem, array, type) \
    { \
        const type **elem##_buf = arrpt_all_const(array, type); \
        uint32_t elem##_i, elem##_total = arrpt_size(array, type); \
        for (elem##_i = 0; elem##_i < elem##_total; ++elem##_i, ++elem##_buf) \
        { \
            const type *elem = (*elem##_buf);

#define arrpt_forback(elem, array, type) \
    { \
        type **elem##_buf = arrpt_all(array, type); \
        uint32_t i, elem##_i, elem##_total = arrpt_size(array, type); \
        elem##_buf += elem##_total - 1; \
        elem##_i = elem##_total - 1; \
        for (i = 0; i < elem##_total; ++i, --elem##_i, --elem##_buf) \
        { \
            type *elem = (*elem##_buf);

#define arrpt_forback_const(elem, array, type) \
    { \
        const type **elem##_buf = arrpt_all_const(array, type); \
        uint32_t i, elem##_i, elem##_total = arrpt_size(array, type); \
        elem##_buf += elem##_total - 1; \
        elem##_i = elem##_total - 1; \
        for (i = 0; i < elem##_total; ++i, --elem##_i, --elem##_buf) \
        { \
            const type *elem = (*elem##_buf);

#define arrpt_end() \
    } \
    }
