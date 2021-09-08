/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: setst.h
 * https://nappgui.com/en/core/setst.html
 *
 */

/* Sets of structures */

#include "rbtree.h"

#define setst_create(func_compare, type)\
    ((void)FUNC_CHECK_COMPARE(func_compare, type),\
    (SetSt(type)*)rbtree_create_imp((FPtr_compare)func_compare, sizeof(type), 0))

#define setst_destroy(set, func_remove, type)\
	((void)((set) == (SetSt(type)**)(set)),\
    FUNC_CHECK_REMOVE(func_remove, type),\
	rbtree_destroy_imp((RBTree**)(set), (FPtr_remove)func_remove, NULL))

#define setst_size(set, type)\
	((void)((set) == (SetSt(type)*)(set)),\
	rbtree_size_imp((RBTree*)(set)))

#define setst_get(set, key, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (void)((key) == (type*)(key)),\
    (type*)rbtree_get_imp((RBTree*)(set), (const void*)(key), FALSE))

#define setst_insert(set, key, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (void)((key) == (type*)(key)),\
    (type*)rbtree_insert_imp((RBTree*)(set), (const void*)(key), NULL))

#define setst_delete(set, key, func_remove, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (void)((key) == (type*)(key)),\
    FUNC_CHECK_REMOVE(func_remove, type),\
    rbtree_delete_imp((RBTree*)(set), (const void*)(key), (FPtr_remove)func_remove, NULL))

#define setst_first(set, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (type*)rbtree_first_imp((RBTree*)(set)))

#define setst_last(set, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (type*)rbtree_last_imp((RBTree*)(set)))

#define setst_next(set, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (type*)rbtree_next_imp((RBTree*)(set)))

#define setst_prev(set, type)\
    ((void)((set) == (SetSt(type)*)(set)),\
    (type*)rbtree_prev_imp((RBTree*)(set)))

#define setst_foreach(elem, set, type)\
    {\
        register type *elem = NULL;\
        register uint32_t elem##_i, elem##_total;\
        elem = setst_first((set), type);\
        elem##_i = 0;\
        elem##_total = setst_size((set), type);\
        while (elem != NULL)\
        {

#define setst_fornext(elem, set, type)\
            elem = setst_next((set), type);\
            elem##_i += 1;\
            unref(elem##_total);\
        }\
    }
            
#define setst_foreach_rev(elem, set, type)\
    {\
        register type *elem = NULL;\
        register uint32_t elem##_i, elem##_total;\
        elem = setst_last((set), type);\
        elem##_total = setst_size((set), type);\
        elem##_i = elem##_total - 1;\
        while (elem != NULL)\
        {

#define setst_forprev(set, type)\
            elem = setst_prev((set), type);\
            elem##_i -= 1;\
            unref(elem##_total);\
        }\
    }

