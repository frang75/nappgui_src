/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: setpt.hxx
 * https://nappgui.com/en/core/setpt.html
 *
 */

/* Set macros for type checking at compile time */

#define SetPtDebug(type) \
    struct NodePt##type \
    { \
        uint32_t rb; \
        struct NodePt##type *left; \
        struct NodePt##type *right; \
        type *data; \
    }; \
\
    struct Set##Pt##type \
    { \
        uint32_t elems; \
        uint16_t esize; \
        uint16_t ksize; \
        struct NodePt##type *root; \
        FPtr_compare func_compare; \
    }

#define SetPtFuncs(type) \
    SetPt(type); \
\
    static __TYPECHECK SetPt(type) *setpt_##type##_create(FPtr_compare func_compare, const uint16_t esize, const char_t *key_type); \
    static SetPt(type) *setpt_##type##_create(FPtr_compare func_compare, const uint16_t esize, const char_t *key_type) \
    { \
        return cast(rbtree_create(func_compare, esize, 0, cast_const(SETPT #type, char_t), key_type), SetPt(type)); \
    } \
\
    static __TYPECHECK void setpt_##type##_destroy(struct Set##Pt##type **set, void(func_destroy)(type **)); \
    static void setpt_##type##_destroy(struct Set##Pt##type **set, void(func_destroy)(type **)) \
    { \
        rbtree_destroy_ptr(dcast(set, RBTree), (FPtr_destroy)func_destroy, NULL, cast_const(SETPT #type, char_t)); \
    } \
\
    static __TYPECHECK uint32_t setpt_##type##_size(const struct Set##Pt##type *set); \
    static uint32_t setpt_##type##_size(const struct Set##Pt##type *set) \
    { \
        return rbtree_size(cast_const(set, RBTree)); \
    } \
\
    static __TYPECHECK type *setpt_##type##_get(struct Set##Pt##type *set, const void *key, const char_t *ktype); \
    static type *setpt_##type##_get(struct Set##Pt##type *set, const void *key, const char_t *ktype) \
    { \
        return cast(rbtree_get(cast(set, RBTree), key, TRUE, ktype), type); \
    } \
\
    static __TYPECHECK const type *setpt_##type##_get_const(const struct Set##Pt##type *set, const void *key, const char_t *ktype); \
    static const type *setpt_##type##_get_const(const struct Set##Pt##type *set, const void *key, const char_t *ktype) \
    { \
        return cast_const(rbtree_get(cast(set, RBTree), key, TRUE, ktype), type); \
    } \
\
    static __TYPECHECK bool_t setpt_##type##_insert(struct Set##Pt##type *set, const void *key, type *ptr, const char_t *ktype); \
    static bool_t setpt_##type##_insert(struct Set##Pt##type *set, const void *key, type *ptr, const char_t *ktype) \
    { \
        return rbtree_insert_ptr(cast(set, RBTree), key, cast(ptr, void), ktype); \
    } \
\
    static __TYPECHECK bool_t setpt_##type##_delete(struct Set##Pt##type *set, const void *key, FPtr_destroy func_destroy, const char_t *ktype); \
    static bool_t setpt_##type##_delete(struct Set##Pt##type *set, const void *key, FPtr_destroy func_destroy, const char_t *ktype) \
    { \
        return rbtree_delete_ptr(cast(set, RBTree), key, func_destroy, NULL, ktype); \
    } \
\
    static __TYPECHECK type *setpt_##type##_first(struct Set##Pt##type *set); \
    static type *setpt_##type##_first(struct Set##Pt##type *set) \
    { \
        return cast(rbtree_first_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK const type *setpt_##type##_first_const(const struct Set##Pt##type *set); \
    static const type *setpt_##type##_first_const(const struct Set##Pt##type *set) \
    { \
        return cast_const(rbtree_first_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK type *setpt_##type##_last(struct Set##Pt##type *set); \
    static type *setpt_##type##_last(struct Set##Pt##type *set) \
    { \
        return cast(rbtree_last_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK const type *setpt_##type##_last_const(const struct Set##Pt##type *set); \
    static const type *setpt_##type##_last_const(const struct Set##Pt##type *set) \
    { \
        return cast_const(rbtree_last_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK type *setpt_##type##_next(struct Set##Pt##type *set); \
    static type *setpt_##type##_next(struct Set##Pt##type *set) \
    { \
        return cast(rbtree_next_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK const type *setpt_##type##_next_const(const struct Set##Pt##type *set); \
    static const type *setpt_##type##_next_const(const struct Set##Pt##type *set) \
    { \
        return cast_const(rbtree_next_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK type *setpt_##type##_prev(struct Set##Pt##type *set); \
    static type *setpt_##type##_prev(struct Set##Pt##type *set) \
    { \
        return cast(rbtree_prev_ptr(cast(set, RBTree)), type); \
    } \
\
    static __TYPECHECK const type *setpt_##type##_prev_const(const struct Set##Pt##type *set); \
    static const type *setpt_##type##_prev_const(const struct Set##Pt##type *set) \
    { \
        return cast_const(rbtree_prev_ptr(cast(set, RBTree)), type); \
    } \
\
    typedef struct _setptend##type##_t setptend##type
