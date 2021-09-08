/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: setpt.h
 * https://nappgui.com/en/core/setpt.html
 *
 */

/* Sets of pointers */

#include "rbtree.h"

#define setpt_create(func_compare, type)\
    ((void)FUNC_CHECK_COMPARE(func_compare, type),\
    (SetPt(type)*)rbtree_create_imp((FPtr_compare)func_compare, sizeof(type*), 0))

#define setpt_destroy(set, func_destroy, type)\
	((void)((set) == (SetPt(type)**)(set)),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
	rbtree_destroy_ptr_imp((RBTree**)(set), (FPtr_destroy)func_destroy, NULL))

#define setpt_size(set, type)\
	((void)((set) == (SetPt(type)*)(set)),\
	rbtree_size_imp((RBTree*)(set)))

#define setpt_get(set, key, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (void)((key) == (type*)(key)),\
    (type*)rbtree_get_imp((RBTree*)(set), (const void*)(key), TRUE))

#define setpt_insert(set, value, type)\
	((void)((set) == (SetPt(type)*)(set)),\
    rbtree_insert_ptr_imp((RBTree*)(set), (void*)(value)))

#define setpt_delete(set, key, func_destroy, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (void)((key) == (type*)(key)),\
    FUNC_CHECK_DESTROY(func_destroy, type),\
    rbtree_delete_ptr_imp((RBTree*)(set), (const void*)(key), (FPtr_destroy)func_destroy, NULL))

#define setpt_first(set, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (type*)rbtree_first_ptr_imp((RBTree*)(set)))

#define setpt_last(set, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (type*)rbtree_last_ptr_imp((RBTree*)(set)))

#define setpt_next(set, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (type*)rbtree_next_ptr_imp((RBTree*)(set)))

#define setpt_prev(set, type)\
    ((void)((set) == (SetPt(type)*)(set)),\
    (type*)rbtree_prev_ptr_imp((RBTree*)(set)))

#define setpt_foreach(elem, set, type)\
    {\
        register type *elem = NULL;\
        register uint32_t elem##_i, elem##_total;\
        elem = setpt_first((set), type);\
        elem##_i = 0;\
        elem##_total = setpt_size((set), type);\
        while (elem != NULL)\
        {

#define setpt_fornext(elem, set, type)\
            elem = setpt_next((set), type);\
            elem##_i += 1;\
            unref(elem##_total);\
        }\
    }
            
#define setpt_foreach_rev(elem, set, type)\
    {\
        register type *elem = NULL;\
        register uint32_t elem##_i, elem##_total;\
        elem = setpt_last((set), type);\
        elem##_total = setpt_size((set), type);\
        elem##_i = elem##_total - 1;\
        while (elem != NULL)\
        {

#define setpt_forprev(set, type)\
            elem = setpt_prev((set), type);\
            elem##_i -= 1;\
            unref(elem##_total);\
        }\
    }
