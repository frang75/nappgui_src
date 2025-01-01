/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: setst.hpp
 * https://nappgui.com/en/core/setst.html
 *
 */

/* Sets */

#ifndef __SETST_HPP__
#define __SETST_HPP__

#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include <typeinfo>
#include <sewer/warn.hxx>

template < class type >
struct SetSt
{
    static void destroy(SetSt< type > **set, void (*func_remove)(type *));

    static uint32_t size(const SetSt< type > *set);

    static type *first(SetSt< type > *set);

    static const type *first(const SetSt< type > *set);

    static type *last(SetSt< type > *set);

    static const type *last(const SetSt< type > *set);

    static type *next(SetSt< type > *set);

    static const type *next(const SetSt< type > *set);

    static type *prev(SetSt< type > *set);

    static const type *prev(const SetSt< type > *set);

#if defined __ASSERTS__
    // Only for debugger inspector (non used)
    template < class ttype >
    struct TypeNode
    {
        uint32_t rb;
        struct TypeNode< ttype > *left;
        struct TypeNode< ttype > *right;
        ttype data;
    };

    uint32_t elems;
    uint16_t esize;
    uint16_t ksize;
    TypeNode< type > *root;
    FPtr_compare func_compare;
#endif
};

template < typename type, typename dtype >
struct SetS2
{
    static SetSt< type > *create(int(func_compare)(const type *, const dtype *));

    static type *get(SetSt< type > *set, const dtype *key);

    static const type *get(const SetSt< type > *set, const dtype *key);

    static type *insert(SetSt< type > *set, const dtype *key);

    static bool_t ddelete(SetSt< type > *set, const dtype *key, void (*func_remove)(type *));
};

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
SetSt< type > *SetS2< type, dtype >::create(int(func_compare)(const type *, const dtype *))
{
    char_t ltype[64];
    bstd_sprintf(ltype, sizeof(ltype), "SetSt<%s>", typeid(type).name());
    return cast(rbtree_create((FPtr_compare)func_compare, (uint16_t)sizeof(type), 0, ltype, typeid(dtype).name()), SetSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void SetSt< type >::destroy(SetSt< type > **set, void (*func_remove)(type *))
{
    char_t ltype[64];
    bstd_sprintf(ltype, sizeof(ltype), "SetSt<%s>", typeid(type).name());
    rbtree_destroy(dcast(set, RBTree), (FPtr_remove)func_remove, NULL, ltype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t SetSt< type >::size(const SetSt< type > *set)
{
    return rbtree_size(cast_const(set, RBTree));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
type *SetS2< type, dtype >::get(SetSt< type > *set, const dtype *key)
{
    return cast(rbtree_get(cast_const(set, RBTree), cast_const(key, void), FALSE, typeid(dtype).name()), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
const type *SetS2< type, dtype >::get(const SetSt< type > *set, const dtype *key)
{
    return cast_const(rbtree_get(cast_const(set, RBTree), cast_const(key, void), FALSE, typeid(dtype).name()), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
type *SetS2< type, dtype >::insert(SetSt< type > *set, const dtype *key)
{
    return cast(rbtree_insert(cast(set, RBTree), cast_const(key, void), NULL, typeid(dtype).name()), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
bool_t SetS2< type, dtype >::ddelete(SetSt< type > *set, const dtype *key, void (*func_remove)(type *))
{
    return rbtree_delete(cast(set, RBTree), cast_const(key, void), (FPtr_remove)func_remove, NULL, typeid(dtype).name());
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetSt< type >::first(SetSt< type > *set)
{
    return cast(rbtree_first(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetSt< type >::first(const SetSt< type > *set)
{
    return cast_const(rbtree_first(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetSt< type >::last(SetSt< type > *set)
{
    return cast(rbtree_last(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetSt< type >::last(const SetSt< type > *set)
{
    return cast_const(rbtree_last(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetSt< type >::next(SetSt< type > *set)
{
    return cast(rbtree_next(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetSt< type >::next(const SetSt< type > *set)
{
    return cast_const(rbtree_next(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *SetSt< type >::prev(SetSt< type > *set)
{
    return cast(rbtree_prev(cast(set, RBTree)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *SetSt< type >::prev(const SetSt< type > *set)
{
    return cast_const(rbtree_prev(cast(set, RBTree)), type);
}

#endif
