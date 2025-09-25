/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrst.hpp
 * https://nappgui.com/en/core/arrst.html
 *
 */

/* Arrays */

#ifndef __ARRST_HPP__
#define __ARRST_HPP__

#include <sewer/bstd.h>
#include <sewer/nowarn.hxx>
#include <typeinfo>
#include <sewer/warn.hxx>

template < class type >
struct ArrSt
{
    static ArrSt< type > *create(void);

    static ArrSt< type > *copy(const ArrSt< type > *array, void (*func_copy)(type *, const type));

    static ArrSt< type > *read(Stream *stm, void (*func_read)(Stream *, type *));

    static void destroy(ArrSt< type > **array, void (*func_remove)(type *));

    static void destopt(ArrSt< type > **array, void (*func_remove)(type *));

    static void clear(ArrSt< type > *array, void (*func_remove)(type *));

    static void write(Stream *stm, const ArrSt< type > *array, void (*func_write)(Stream *, const type *));

    static uint32_t size(const ArrSt< type > *array);

    static type *get(ArrSt< type > *array, const uint32_t pos);

    static const type *get(const ArrSt< type > *array, const uint32_t pos);

    static type *first(ArrSt< type > *array);

    static const type *first(const ArrSt< type > *array);

    static type *last(ArrSt< type > *array);

    static const type *last(const ArrSt< type > *array);

    static type *all(ArrSt< type > *array);

    static const type *all(const ArrSt< type > *array);

    static type *nnew(ArrSt< type > *array);

    static type *new0(ArrSt< type > *array);

    static type *new_n(ArrSt< type > *array, const uint32_t n);

    static type *new_n0(ArrSt< type > *array, const uint32_t n);

    static type *prepend_n(ArrSt< type > *array, const uint32_t n);

    static type *insert_n(ArrSt< type > *array, const uint32_t pos, const uint32_t n);

    static type *insert_n0(ArrSt< type > *array, const uint32_t pos, const uint32_t n);

    static void append(ArrSt< type > *array, const type value);

    static void prepend(ArrSt< type > *array, const type value);

    static void insert(ArrSt< type > *array, const uint32_t pos, const type value);

    static void join(ArrSt< type > *dest, const ArrSt< type > *src, void (*func_copy)(type *, const type));

    static void ddelete(ArrSt< type > *array, const uint32_t pos, void (*func_remove)(type *));

    static void pop(ArrSt< type > *array, void (*func_remove)(type *));

    static void sort(ArrSt< type > *array, int (*func_compare)(const type *, const type *));

#if defined __ASSERTS__
    // Only for debugger inspector (non used)
    template < class ttype >
    struct TypeData
    {
        ttype elem[1024];
    };

    uint32_t reserved;
    uint32_t nsize;
    uint16_t elem_sizeof;
    TypeData< type > *content;
#endif
};

template < typename type, typename dtype >
struct ArrS2
{
    static ArrSt< type > *read_ex(Stream *stream, void (*func_read)(Stream *, type *, const dtype *), const dtype *data);

    static void write_ex(Stream *stream, const ArrSt< type > *array, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data);

    static void sort_ex(ArrSt< type > *array, int (*func_compare)(const type *, const type *, const dtype *), const dtype *data);

    static type *search(ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos);

    static const type *search(const ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos);

    static type *bsearch(ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos);

    static const type *bsearch(const ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos);
};

/*---------------------------------------------------------------------------*/

template < typename type >
ArrSt< type > *ArrSt< type >::create(void)
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
    return cast(array_create(sizeof(type), dtype), ArrSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
ArrSt< type > *ArrSt< type >::copy(const ArrSt< type > *array, void (*func_copy)(type *, const type))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
    return cast(array_copy(cast_const(array, Array), (FPtr_scopy)func_copy, dtype), ArrSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
ArrSt< type > *ArrSt< type >::read(Stream *stm, void (*func_read)(Stream *, type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
    return cast(array_read(stm, sizeof(type), (FPtr_read_init)func_read, dtype), ArrSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::destroy(ArrSt< type > **array, void (*func_remove)(type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
    array_destroy(dcast(array, Array), (FPtr_remove)func_remove, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::destopt(ArrSt< type > **array, void (*func_remove)(type *))
{
    char_t dtype[64];
    bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
    array_destopt(dcast(array, Array), (FPtr_remove)func_remove, dtype);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::clear(ArrSt< type > *array, void (*func_remove)(type *))
{
    array_clear(cast(array, Array), (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::write(Stream *stm, const ArrSt< type > *array, void (*func_write)(Stream *, const type *))
{
    array_write(stm, cast_const(array, Array), (FPtr_write)func_write);
}

/*---------------------------------------------------------------------------*/

template < typename type >
uint32_t ArrSt< type >::size(const ArrSt< type > *array)
{
    return array_size(cast_const(array, Array));
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::get(ArrSt< type > *array, const uint32_t pos)
{
    return cast(array_get(cast_const(array, Array), pos), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *ArrSt< type >::get(const ArrSt< type > *array, const uint32_t pos)
{
    return cast_const(array_get(cast_const(array, Array), pos), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::first(ArrSt< type > *array)
{
    return cast(array_get(cast_const(array, Array), 0), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *ArrSt< type >::first(const ArrSt< type > *array)
{
    return cast_const(array_get(cast_const(array, Array), 0), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::last(ArrSt< type > *array)
{
    return cast(array_get_last(cast_const(array, Array)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *ArrSt< type >::last(const ArrSt< type > *array)
{
    return cast_const(array_get_last(cast_const(array, Array)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::all(ArrSt< type > *array)
{
    return cast(array_all(cast_const(array, Array)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
const type *ArrSt< type >::all(const ArrSt< type > *array)
{
    return cast(array_all(cast_const(array, Array)), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::nnew(ArrSt< type > *array)
{
    return cast(array_insert(cast(array, Array), UINT32_MAX, 1), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::new0(ArrSt< type > *array)
{
    return cast(array_insert0(cast(array, Array), UINT32_MAX, 1), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::new_n(ArrSt< type > *array, const uint32_t n)
{
    return cast(array_insert(cast(array, Array), UINT32_MAX, n), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::new_n0(ArrSt< type > *array, const uint32_t n)
{
    return cast(array_insert0(cast(array, Array), UINT32_MAX, n), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::prepend_n(ArrSt< type > *array, const uint32_t n)
{
    return cast(array_insert(cast(array, Array), 0, n), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::insert_n(ArrSt< type > *array, const uint32_t pos, const uint32_t n)
{
    return cast(array_insert(cast(array, Array), pos, n), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
type *ArrSt< type >::insert_n0(ArrSt< type > *array, const uint32_t pos, const uint32_t n)
{
    return cast(array_insert0(cast(array, Array), pos, n), type);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::append(ArrSt< type > *array, const type value)
{
    *cast(array_insert(cast(array, Array), UINT32_MAX, 1), type) = value;
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::prepend(ArrSt< type > *array, const type value)
{
    *cast(array_insert(cast(array, Array), 0, 1), type) = value;
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::insert(ArrSt< type > *array, const uint32_t pos, const type value)
{
    *cast(array_insert(cast(array, Array), pos, 1), type) = value;
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::join(ArrSt< type > *dest, const ArrSt< type > *src, void (*func_copy)(type *, const type))
{
    array_join(cast(dest, Array), cast_const(src, Array), (FPtr_scopy)func_copy);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::ddelete(ArrSt< type > *array, const uint32_t pos, void (*func_remove)(type *))
{
    array_delete(cast(array, Array), pos, 1, (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::pop(ArrSt< type > *array, void (*func_remove)(type *))
{
    array_pop(cast(array, Array), (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template < typename type >
void ArrSt< type >::sort(ArrSt< type > *array, int (*func_compare)(const type *, const type *))
{
    array_sort(cast(array, Array), (FPtr_compare)func_compare);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
ArrSt< type > *ArrS2< type, dtype >::read_ex(Stream *stream, void (*func_read)(Stream *, type *, const dtype *), const dtype *data)
{
    char_t ntype[64];
    bstd_sprintf(ntype, sizeof(ntype), "ArrSt<%s>", typeid(type).name());
    return cast(array_read_ex(stream, sizeof(type), (FPtr_read_init_ex)func_read, cast(data, void), ntype), ArrSt< type >);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
void ArrS2< type, dtype >::write_ex(Stream *stream, const ArrSt< type > *array, void (*func_write)(Stream *, const type *, const dtype *), const dtype *data)
{
    array_write_ex(stream, cast(array, Array), (FPtr_write_ex)func_write, cast(data, void));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
void ArrS2< type, dtype >::sort_ex(ArrSt< type > *array, int (*func_compare)(const type *, const type *, const dtype *), const dtype *data)
{
    array_sort_ex(cast(array, Array), (FPtr_compare_ex)func_compare, cast(data, void));
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
type *ArrS2< type, dtype >::search(ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos)
{
    return cast(array_search(cast(array, Array), (FPtr_compare)func_compare, cast_const(key, void), pos), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
const type *ArrS2< type, dtype >::search(const ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos)
{
    return cast_const(array_search(cast_const(array, Array), (FPtr_compare)func_compare, cast_const(key, void), pos), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
type *ArrS2< type, dtype >::bsearch(ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos)
{
    return cast(array_bsearch(cast(array, Array), (FPtr_compare)func_compare, cast_const(key, void), pos), type);
}

/*---------------------------------------------------------------------------*/

template < typename type, typename dtype >
const type *ArrS2< type, dtype >::bsearch(const ArrSt< type > *array, int (*func_compare)(const type *, const dtype *), const dtype *key, uint32_t *pos)
{
    return cast(array_bsearch(cast_const(array, Array), (FPtr_compare)func_compare, cast_const(key, void), pos), type);
}

#endif
