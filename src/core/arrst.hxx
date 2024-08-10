/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrst.hxx
 * https://nappgui.com/en/core/arrst.html
 *
 */

/* Array macros for type checking at compile time */

#define ArrStDebug(type) \
    struct type##Data \
    { \
        type elem[1024]; \
    }; \
\
    struct Arr##St##type \
    { \
        uint32_t reserved; \
        uint32_t size; \
        uint16_t elem_sizeof; \
        struct type##Data *content; \
    }

#define ArrStFuncs(type) \
    ArrSt(type); \
\
    static __TYPECHECK ArrSt(type) *arrst_##type##_create(const uint16_t esize); \
    static ArrSt(type) *arrst_##type##_create(const uint16_t esize) \
    { \
        return cast(array_create(esize, cast_const(ARRST #type, char_t)), ArrSt(type)); \
    } \
\
    static __TYPECHECK ArrSt(type) *arrst_##type##_copy(const struct Arr##St##type *array, void(func_copy)(type *, const type *)); \
    static ArrSt(type) *arrst_##type##_copy(const struct Arr##St##type *array, void(func_copy)(type *, const type *)) \
    { \
        return cast(array_copy(cast_const(array, Array), (FPtr_scopy)func_copy, cast_const(ARRST #type, char_t)), ArrSt(type)); \
    } \
\
    static __TYPECHECK ArrSt(type) *arrst_##type##_read(Stream *stream, const uint16_t esize, void(func_read)(Stream *, type *)); \
    static ArrSt(type) *arrst_##type##_read(Stream *stream, const uint16_t esize, void(func_read)(Stream *, type *)) \
    { \
        return cast(array_read(stream, esize, (FPtr_read_init)func_read, cast_const(ARRST #type, char_t)), ArrSt(type)); \
    } \
\
    static __TYPECHECK void arrst_##type##_destroy(struct Arr##St##type **array, void(func_remove)(type *)); \
    static void arrst_##type##_destroy(struct Arr##St##type **array, void(func_remove)(type *)) \
    { \
        array_destroy(dcast(array, Array), (FPtr_remove)func_remove, cast_const(ARRST #type, char_t)); \
    } \
\
    static __TYPECHECK void arrst_##type##_destopt(struct Arr##St##type **array, void(func_remove)(type *)); \
    static void arrst_##type##_destopt(struct Arr##St##type **array, void(func_remove)(type *)) \
    { \
        array_destopt(dcast(array, Array), (FPtr_remove)func_remove, cast_const(ARRST #type, char_t)); \
    } \
\
    static __TYPECHECK void arrst_##type##_clear(struct Arr##St##type *array, void(func_remove)(type *)); \
    static void arrst_##type##_clear(struct Arr##St##type *array, void(func_remove)(type *)) \
    { \
        array_clear(cast(array, Array), (FPtr_remove)func_remove); \
    } \
\
    static __TYPECHECK void arrst_##type##_write(Stream *stream, const struct Arr##St##type *array, void(func_write)(Stream *, const type *)); \
    static void arrst_##type##_write(Stream *stream, const struct Arr##St##type *array, void(func_write)(Stream *, const type *)) \
    { \
        array_write(stream, cast_const(array, Array), (FPtr_write)func_write); \
    } \
\
    static __TYPECHECK uint32_t arrst_##type##_size(const struct Arr##St##type *array); \
    static uint32_t arrst_##type##_size(const struct Arr##St##type *array) \
    { \
        return array_size(cast_const(array, Array)); \
    } \
\
    static __TYPECHECK type *arrst_##type##_get(struct Arr##St##type *array, const uint32_t pos); \
    static type *arrst_##type##_get(struct Arr##St##type *array, const uint32_t pos) \
    { \
        return cast(array_get(cast_const(array, Array), pos), type); \
    } \
\
    static __TYPECHECK const type *arrst_##type##_get_const(const struct Arr##St##type *array, const uint32_t pos); \
    static const type *arrst_##type##_get_const(const struct Arr##St##type *array, const uint32_t pos) \
    { \
        return cast_const(array_get(cast_const(array, Array), pos), type); \
    } \
\
    static __TYPECHECK type *arrst_##type##_last(struct Arr##St##type *array); \
    static type *arrst_##type##_last(struct Arr##St##type *array) \
    { \
        return cast(array_get_last(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK const type *arrst_##type##_last_const(const struct Arr##St##type *array); \
    static const type *arrst_##type##_last_const(const struct Arr##St##type *array) \
    { \
        return cast_const(array_get_last(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK type *arrst_##type##_all(struct Arr##St##type *array); \
    static type *arrst_##type##_all(struct Arr##St##type *array) \
    { \
        return cast(array_all(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK const type *arrst_##type##_all_const(const struct Arr##St##type *array); \
    static const type *arrst_##type##_all_const(const struct Arr##St##type *array) \
    { \
        return cast_const(array_all(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK type *arrst_##type##_insert(struct Arr##St##type *array, const uint32_t pos, const uint32_t n); \
    static type *arrst_##type##_insert(struct Arr##St##type *array, const uint32_t pos, const uint32_t n) \
    { \
        return cast(array_insert(cast(array, Array), pos, n), type); \
    } \
\
    static __TYPECHECK type *arrst_##type##_insert0(struct Arr##St##type *array, const uint32_t pos, const uint32_t n); \
    static type *arrst_##type##_insert0(struct Arr##St##type *array, const uint32_t pos, const uint32_t n) \
    { \
        return cast(array_insert0(cast(array, Array), pos, n), type); \
    } \
\
    static __TYPECHECK void arrst_##type##_join(struct Arr##St##type *dest, const struct Arr##St##type *src, void(func_copy)(type *, const type *)); \
    static void arrst_##type##_join(struct Arr##St##type *dest, const struct Arr##St##type *src, void(func_copy)(type *, const type *)) \
    { \
        array_join(cast(dest, Array), cast_const(src, Array), (FPtr_scopy)func_copy); \
    } \
\
    static __TYPECHECK void arrst_##type##_delete(struct Arr##St##type *array, const uint32_t pos, void(func_remove)(type *)); \
    static void arrst_##type##_delete(struct Arr##St##type *array, const uint32_t pos, void(func_remove)(type *)) \
    { \
        array_delete(cast(array, Array), pos, 1, (FPtr_remove)func_remove); \
    } \
\
    static __TYPECHECK void arrst_##type##_pop(struct Arr##St##type *array, void(func_remove)(type *)); \
    static void arrst_##type##_pop(struct Arr##St##type *array, void(func_remove)(type *)) \
    { \
        array_pop(cast(array, Array), (FPtr_remove)func_remove); \
    } \
\
    static __TYPECHECK void arrst_##type##_sort(struct Arr##St##type *array, int(func_compare)(const type *, const type *)); \
    static void arrst_##type##_sort(struct Arr##St##type *array, int(func_compare)(const type *, const type *)) \
    { \
        array_sort(cast(array, Array), (FPtr_compare)func_compare); \
    } \
\
    static __TYPECHECK void arrst_##type##_sort_ex(struct Arr##St##type *array, FPtr_compare_ex func_compare, void *data); \
    static void arrst_##type##_sort_ex(struct Arr##St##type *array, FPtr_compare_ex func_compare, void *data) \
    { \
        array_sort_ex(cast(array, Array), func_compare, data); \
    } \
\
    static __TYPECHECK type *arrst_##type##_search(struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static type *arrst_##type##_search(struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast(array_search(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK const type *arrst_##type##_search_const(const struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static const type *arrst_##type##_search_const(const struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast_const(array_search(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK type *arrst_##type##_bsearch(struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static type *arrst_##type##_bsearch(struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast(array_bsearch(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK const type *arrst_##type##_bsearch_const(const struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static const type *arrst_##type##_bsearch_const(const struct Arr##St##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast_const(array_bsearch(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    typedef struct _arrstend##type##_t arrstend##type
