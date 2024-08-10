/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrpt.hxx
 * https://nappgui.com/en/core/arrpt.html
 *
 */

/* Array macros for type checking at compile time */

#define ArrPtDebug(type) \
    struct type##PtData \
    { \
        type *elem[1024]; \
    }; \
\
    struct Arr##Pt##type \
    { \
        uint32_t reserved; \
        uint32_t size; \
        uint16_t elem_sizeof; \
        struct type##PtData *content; \
    }

#define ArrPtFuncs(type) \
    ArrPt(type); \
\
    static __TYPECHECK ArrPt(type) *arrpt_##type##_create(const uint16_t esize); \
    static ArrPt(type) *arrpt_##type##_create(const uint16_t esize) \
    { \
        return cast(array_create(esize, cast_const(ARRPT #type, char_t)), ArrPt(type)); \
    } \
\
    static __TYPECHECK ArrPt(type) *arrpt_##type##_copy(const struct Arr##Pt##type *array, type *(func_copy)(const type *)); \
    static ArrPt(type) *arrpt_##type##_copy(const struct Arr##Pt##type *array, type *(func_copy)(const type *)) \
    { \
        return cast(array_copy_ptr(cast_const(array, Array), (FPtr_copy)func_copy, cast_const(ARRPT #type, char_t)), ArrPt(type)); \
    } \
\
    static __TYPECHECK ArrPt(type) *arrpt_##type##_read(Stream *stream, type *(func_read)(Stream *)); \
    static ArrPt(type) *arrpt_##type##_read(Stream *stream, type *(func_read)(Stream *)) \
    { \
        return cast(array_read_ptr(stream, (FPtr_read)func_read, cast_const(ARRPT #type, char_t)), ArrPt(type)); \
    } \
\
    static __TYPECHECK void arrpt_##type##_destroy(struct Arr##Pt##type **array, void(func_destroy)(type **)); \
    static void arrpt_##type##_destroy(struct Arr##Pt##type **array, void(func_destroy)(type **)) \
    { \
        array_destroy_ptr(dcast(array, Array), (FPtr_destroy)func_destroy, cast_const(ARRPT #type, char_t)); \
    } \
\
    static __TYPECHECK void arrpt_##type##_destopt(struct Arr##Pt##type **array, void(func_destroy)(type **)); \
    static void arrpt_##type##_destopt(struct Arr##Pt##type **array, void(func_destroy)(type **)) \
    { \
        array_destopt_ptr(dcast(array, Array), (FPtr_destroy)func_destroy, cast_const(ARRPT #type, char_t)); \
    } \
\
    static __TYPECHECK void arrpt_##type##_clear(struct Arr##Pt##type *array, void(func_destroy)(type **)); \
    static void arrpt_##type##_clear(struct Arr##Pt##type *array, void(func_destroy)(type **)) \
    { \
        array_clear_ptr(cast(array, Array), (FPtr_destroy)func_destroy); \
    } \
\
    static __TYPECHECK void arrpt_##type##_write(Stream *stream, const struct Arr##Pt##type *array, void(func_write)(Stream *, const type *)); \
    static void arrpt_##type##_write(Stream *stream, const struct Arr##Pt##type *array, void(func_write)(Stream *, const type *)) \
    { \
        array_write_ptr(stream, cast_const(array, Array), (FPtr_write)func_write); \
    } \
\
    static __TYPECHECK uint32_t arrpt_##type##_size(const struct Arr##Pt##type *array); \
    static uint32_t arrpt_##type##_size(const struct Arr##Pt##type *array) \
    { \
        return array_size(cast_const(array, Array)); \
    } \
\
    static __TYPECHECK type *arrpt_##type##_get(struct Arr##Pt##type *array, const uint32_t pos); \
    static type *arrpt_##type##_get(struct Arr##Pt##type *array, const uint32_t pos) \
    { \
        return *dcast(array_get(cast_const(array, Array), pos), type); \
    } \
\
    static __TYPECHECK const type *arrpt_##type##_get_const(const struct Arr##Pt##type *array, const uint32_t pos); \
    static const type *arrpt_##type##_get_const(const struct Arr##Pt##type *array, const uint32_t pos) \
    { \
        return *dcast_const(array_get(cast_const(array, Array), pos), type); \
    } \
\
    static __TYPECHECK type *arrpt_##type##_first(struct Arr##Pt##type *array); \
    static type *arrpt_##type##_first(struct Arr##Pt##type *array) \
    { \
        return *dcast(array_get(cast_const(array, Array), 0), type); \
    } \
\
    static __TYPECHECK const type *arrpt_##type##_first_const(const struct Arr##Pt##type *array); \
    static const type *arrpt_##type##_first_const(const struct Arr##Pt##type *array) \
    { \
        return *dcast_const(array_get(cast_const(array, Array), 0), type); \
    } \
\
    static __TYPECHECK type *arrpt_##type##_last(struct Arr##Pt##type *array); \
    static type *arrpt_##type##_last(struct Arr##Pt##type *array) \
    { \
        return *dcast(array_get_last(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK const type *arrpt_##type##_last_const(const struct Arr##Pt##type *array); \
    static const type *arrpt_##type##_last_const(const struct Arr##Pt##type *array) \
    { \
        return *dcast_const(array_get_last(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK type **arrpt_##type##_all(struct Arr##Pt##type *array); \
    static type **arrpt_##type##_all(struct Arr##Pt##type *array) \
    { \
        return dcast(array_all(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK const type **arrpt_##type##_all_const(const struct Arr##Pt##type *array); \
    static const type **arrpt_##type##_all_const(const struct Arr##Pt##type *array) \
    { \
        return dcast_const(array_all(cast_const(array, Array)), type); \
    } \
\
    static __TYPECHECK type **arrpt_##type##_insert(struct Arr##Pt##type *array, const uint32_t pos, const uint32_t n); \
    static type **arrpt_##type##_insert(struct Arr##Pt##type *array, const uint32_t pos, const uint32_t n) \
    { \
        return dcast(array_insert(cast(array, Array), pos, n), type); \
    } \
\
    static __TYPECHECK void arrpt_##type##_join(struct Arr##Pt##type *dest, const struct Arr##Pt##type *src, type *(func_copy)(const type *)); \
    static void arrpt_##type##_join(struct Arr##Pt##type *dest, const struct Arr##Pt##type *src, type *(func_copy)(const type *)) \
    { \
        array_join_ptr(cast(dest, Array), cast_const(src, Array), (FPtr_copy)func_copy); \
    } \
\
    static __TYPECHECK void arrpt_##type##_delete(struct Arr##Pt##type *array, const uint32_t pos, void(func_destroy)(type **)); \
    static void arrpt_##type##_delete(struct Arr##Pt##type *array, const uint32_t pos, void(func_destroy)(type **)) \
    { \
        array_delete_ptr(cast(array, Array), pos, 1, (FPtr_destroy)func_destroy); \
    } \
\
    static __TYPECHECK void arrpt_##type##_pop(struct Arr##Pt##type *array, void(func_destroy)(type **)); \
    static void arrpt_##type##_pop(struct Arr##Pt##type *array, void(func_destroy)(type **)) \
    { \
        array_pop_ptr(cast(array, Array), (FPtr_destroy)func_destroy); \
    } \
\
    static __TYPECHECK void arrpt_##type##_sort(struct Arr##Pt##type *array, int(func_compare)(const type *, const type *)); \
    static void arrpt_##type##_sort(struct Arr##Pt##type *array, int(func_compare)(const type *, const type *)) \
    { \
        array_sort_ptr(cast(array, Array), (FPtr_compare)func_compare); \
    } \
\
    static __TYPECHECK void arrpt_##type##_sort_ex(struct Arr##Pt##type *array, FPtr_compare_ex func_compare, void *data); \
    static void arrpt_##type##_sort_ex(struct Arr##Pt##type *array, FPtr_compare_ex func_compare, void *data) \
    { \
        array_sort_ptr_ex(cast(array, Array), func_compare, data); \
    } \
\
    static __TYPECHECK uint32_t arrpt_##type##_find(const struct Arr##Pt##type *array, const type *elem); \
    static uint32_t arrpt_##type##_find(const struct Arr##Pt##type *array, const type *elem) \
    { \
        return array_find_ptr(cast_const(array, Array), cast_const(elem, void)); \
    } \
\
    static __TYPECHECK type *arrpt_##type##_search(struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static type *arrpt_##type##_search(struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast(array_search_ptr(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK const type *arrpt_##type##_search_const(const struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static const type *arrpt_##type##_search_const(const struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast_const(array_search_ptr(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK type *arrpt_##type##_bsearch(struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static type *arrpt_##type##_bsearch(struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast(array_bsearch_ptr(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    static __TYPECHECK const type *arrpt_##type##_bsearch_const(const struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos); \
    static const type *arrpt_##type##_bsearch_const(const struct Arr##Pt##type *array, FPtr_compare func_compare, const void *key, uint32_t *pos) \
    { \
        return cast_const(array_bsearch_ptr(cast_const(array, Array), func_compare, key, pos), type); \
    } \
\
    typedef struct _arrptend##type##_t arrptend##type
