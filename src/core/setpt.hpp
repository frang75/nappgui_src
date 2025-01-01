/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: setpt.hpp
 * https://nappgui.com/en/core/setpt.html
 *
 */

/* Pointer sets */

#ifndef __SETPT_HPP__
#define __SETPT_HPP__

#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include <typeinfo>
#include <sewer/warn.hxx>

template < class type >
struct SetPt
{
    static void destroy(SetPt< type > **set, void (*func_destroy)(type **));

    static uint32_t size(const SetPt< type > *set);

    static type *first(SetPt< type > *set);

    static const type *first(const SetPt< type > *set);

    static type *last(SetPt< type > *set);

    static const type *last(const SetPt< type > *set);

    static type *next(SetPt< type > *set);

    static const type *next(const SetPt< type > *set);

    static type *prev(SetPt< type > *set);

    static const type *prev(const SetPt< type > *set);

    // Only for debugger inspector (non used)
    template < class ttype >
    struct TypeNode
    {
        uint32_t rb;
        struct TypeNode< ttype > *left;
        struct TypeNode< ttype > *right;
        ttype *data;
    };

    uint32_t elems;
    uint16_t esize;
    uint16_t ksize;
    TypeNode< type > *root;
    FPtr_compare func_compare;
};

template < typename type, typename dtype >
struct SetP2
{
    static SetPt< type > *create(int(func_compare)(const type *, const dtype *));

    static type *get(SetPt< type > *set, const dtype *key);

    static const type *get(const SetPt< type > *set, const dtype *key);

    static bool_t insert(SetPt< type > *set, const dtype *key, type *ptr);

    static bool_t ddelete(SetPt< type > *set, const dtype *key, void (*func_destroy)(type **));
};

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
SetPt< type > *SetP2< type, dtype >::create(int(func_compare)(const type *, const dtype *))
{
    char_t ltype[64];
    bstd_sprintf(ltype, sizeof(ltype), "SetPt<%s>", typeid(type).name());
    return cast(rbtree_create((FPtr_compare)func_compare, (uint16_t)sizeof(type *), 0, ltype, typeid(dtype).name()), SetPt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void SetPt< type >::destroy(SetPt< type > **set, void (*func_destroy)(type **))
{
    char_t ltype[64];
    bstd_sprintf(ltype, sizeof(ltype), "SetPt<%s>", typeid(type).name());
    rbtree_destroy_ptr(dcast(set, RBTree), (FPtr_destroy)func_destroy, NULL, ltype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t SetPt< type >::size(const SetPt< type > *set)
{
    return rbtree_size(cast_const(set, RBTree));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
type *SetP2< type, dtype >::get(SetPt< type > *set, const dtype *key)
{
    return cast(rbtree_get(cast(set, RBTree), cast_const(key, void), TRUE, typeid(dtype).name()), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
const type *SetP2< type, dtype >::get(const SetPt< type > *set, const dtype *key)
{
    return cast_const(rbtree_get(cast_const(set, RBTree), cast_const(key, void), TRUE, typeid(dtype).name()), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
bool_t SetP2< type, dtype >::insert(SetPt< type > *set, const dtype *key, type *ptr)
{
    return rbtree_insert_ptr(cast(set, RBTree), cast_const(key, void), cast(ptr, void), typeid(dtype).name());
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
bool_t SetP2< type, dtype >::ddelete(SetPt< type > *set, const dtype *key, void (*func_destroy)(type **))
{
    return rbtree_delete_ptr(cast(set, RBTree), cast_const(key, void), (FPtr_destroy)func_destroy, NULL, typeid(dtype).name());
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetPt< type >::first(SetPt< type > *set)
{
    return cast(rbtree_first_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetPt< type >::first(const SetPt< type > *set)
{
    return cast_const(rbtree_first_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetPt< type >::last(SetPt< type > *set)
{
    return cast(rbtree_last_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetPt< type >::last(const SetPt< type > *set)
{
    return cast_const(rbtree_last_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetPt< type >::next(SetPt< type > *set)
{
    return cast(rbtree_next_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetPt< type >::next(const SetPt< type > *set)
{
    return cast_const(rbtree_next_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetPt< type >::prev(SetPt< type > *set)
{
    return cast(rbtree_prev_ptr(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetPt< type >::prev(const SetPt< type > *set)
{
    return cast_const(rbtree_prev_ptr(cast(set, RBTree)), type);
}

#endif
