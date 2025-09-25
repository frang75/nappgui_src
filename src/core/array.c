/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: array.c
 *
 */

/* Array data structure */

#include "heap.h"
#include "stream.h"
#include "strings.h"
#include <sewer/blib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/ptr.h>

struct _array_t
{
    uint32_t nallocs;
    uint32_t elems;
    uint16_t esize;
    byte_t *data;
};

/*---------------------------------------------------------------------------*/

#define i_MINIMUN_ARRAY_SIZE 8

/*---------------------------------------------------------------------------*/

static Array *i_create_array(
    const uint32_t nallocs,
    const uint32_t elems,
    const uint16_t esize,
    byte_t **data,
    const char_t *type)
{
    Array *array = cast(heap_malloc(sizeof(Array), type), Array);
    array->nallocs = nallocs;
    array->elems = elems;
    array->esize = esize;
    array->data = ptr_dget_no_null(data, byte_t);
    return array;
}

/*---------------------------------------------------------------------------*/

static void i_destroy(Array **array, const char_t *type)
{
    cassert_no_null(array);
    cassert_no_null(*array);
    heap_free(&(*array)->data, (*array)->nallocs * (*array)->esize, "ArrayData");
    heap_free(dcast(array, byte_t), sizeof(Array), type);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_next_pow2(const uint32_t value)
{
    uint32_t v = value;
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

/*---------------------------------------------------------------------------*/

static Array *i_create_init_array(const uint32_t elems, const uint16_t esize, const char_t *type)
{
    uint32_t nallocs = 0;
    byte_t *data = NULL;

    cassert(esize > 0);
    nallocs = i_next_pow2(elems);
    if (nallocs < i_MINIMUN_ARRAY_SIZE)
        nallocs = i_MINIMUN_ARRAY_SIZE;

    data = heap_malloc(nallocs * esize, "ArrayData");
    return i_create_array(nallocs, elems, esize, &data, type);
}

/*---------------------------------------------------------------------------*/

Array *array_create(const uint16_t esize, const char_t *type)
{
    return i_create_init_array(0, esize, type);
}

/*---------------------------------------------------------------------------*/

Array *array_copy(const Array *array, FPtr_scopy func_copy, const char_t *type)
{
    byte_t *data = NULL;

    cassert_no_null(array);
    data = heap_malloc(array->nallocs * array->esize, "ArrayData");

    if (func_copy != NULL)
    {
        byte_t *dest = data;
        const byte_t *src = array->data;
        uint32_t i = 0;
        for (i = 0; i < array->elems; ++i, dest += array->esize, src += array->esize)
            func_copy(cast(dest, void), cast_const(src, void));
    }
    else
    {
        bmem_copy(data, array->data, array->elems * array->esize);
    }

    return i_create_array(array->nallocs, array->elems, array->esize, &data, type);
}

/*---------------------------------------------------------------------------*/

Array *array_copy_ptr(const Array *array, FPtr_copy func_copy, const char_t *type)
{
    byte_t *data = NULL;
    cassert_no_null(array);
    cassert_no_nullf(func_copy);
    cassert(array->esize == sizeofptr);
    data = heap_malloc(array->nallocs * array->esize, "ArrayData");
    if (func_copy != NULL)
    {
        uint32_t i;
        for (i = 0; i < array->elems; ++i)
        {
            void *elem = func_copy(*dcast(array->data + i * array->esize, void));
            *dcast(data + i * array->esize, void) = elem;
        }
    }
    else
    {
        bmem_copy(data, array->data, array->elems * array->esize);
    }

    return i_create_array(array->nallocs, array->elems, array->esize, &data, type);
}

/*---------------------------------------------------------------------------*/

static Array *i_read_array(Stream *stream, const uint16_t esize, FPtr_read func_read, FPtr_read_init func_read_init, const char_t *type)
{
    uint32_t elems = stm_read_u32(stream);
    Array *array = i_create_init_array(elems, esize, type);

    if (func_read != NULL)
    {
        uint32_t i;
        cassert(func_read_init == NULL);
        cassert(esize == sizeofptr);
        for (i = 0; i < elems; ++i)
        {
            bool_t nonull = stm_read_bool(stream);
            void *elem = NULL;
            if (nonull == TRUE)
                elem = func_read(stream);
            *dcast(array->data + i * esize, void) = elem;
        }
    }
    else
    {
        uint32_t i;
        cassert_no_nullf(func_read_init);
        for (i = 0; i < elems; ++i)
            func_read_init(stream, cast(array->data + i * esize, void));
    }

    return array;
}

/*---------------------------------------------------------------------------*/

static Array *i_read_array_ex(Stream *stream, const uint16_t esize, FPtr_read_ex func_read, FPtr_read_init_ex func_read_init, void *data, const char_t *type)
{
    uint32_t elems = stm_read_u32(stream);
    Array *array = i_create_init_array(elems, esize, type);

    if (func_read != NULL)
    {
        uint32_t i;
        cassert(func_read_init == NULL);
        cassert(esize == sizeofptr);
        for (i = 0; i < elems; ++i)
        {
            bool_t nonull = stm_read_bool(stream);
            void *elem = NULL;
            if (nonull == TRUE)
                elem = func_read(stream, data);
            *dcast(array->data + i * esize, void) = elem;
        }
    }
    else
    {
        uint32_t i;
        cassert_no_nullf(func_read_init);
        for (i = 0; i < elems; ++i)
            func_read_init(stream, cast(array->data + i * esize, void), data);
    }

    return array;
}

/*---------------------------------------------------------------------------*/

Array *array_read(Stream *stream, const uint16_t esize, FPtr_read_init func_read_init, const char_t *type)
{
    return i_read_array(stream, esize, NULL, func_read_init, type);
}

/*---------------------------------------------------------------------------*/

Array *array_read_ex(Stream *stream, const uint16_t esize, FPtr_read_init_ex func_read_init, void *data, const char_t *type)
{
    return i_read_array_ex(stream, esize, NULL, func_read_init, data, type);
}

/*---------------------------------------------------------------------------*/

Array *array_read_ptr(Stream *stream, FPtr_read func_read, const char_t *type)
{
    return i_read_array(stream, sizeofptr, func_read, NULL, type);
}

/*---------------------------------------------------------------------------*/

Array *array_read_ptr_ex(Stream *stream, FPtr_read_ex func_read, void *data, const char_t *type)
{
    return i_read_array_ex(stream, sizeofptr, func_read, NULL, data, type);
}

/*---------------------------------------------------------------------------*/

static void i_remove_elems(byte_t *data, const uint32_t esize, const uint32_t elems, FPtr_remove func_remove)
{
    uint32_t i;
    cassert_no_null(data);
    cassert_no_nullf(func_remove);
    for (i = 0; i < elems; ++i, data += esize)
        func_remove(data);
}

/*---------------------------------------------------------------------------*/

static void i_destroy_elems(void **data, const uint32_t elems, FPtr_destroy func_destroy)
{
    uint32_t i;
    cassert_no_nullf(func_destroy);
    for (i = 0; i < elems; ++i, ++data)
    {
        cassert_no_null(data);
        if (*data != NULL)
            func_destroy(data);
    }
}

/*---------------------------------------------------------------------------*/

void array_destroy(Array **array, FPtr_remove func_remove, const char_t *type)
{
    cassert_no_null(array);
    cassert_no_null(*array);
    if (func_remove != NULL)
        i_remove_elems((*array)->data, (*array)->esize, (*array)->elems, func_remove);
    i_destroy(array, type);
}

/*---------------------------------------------------------------------------*/

void array_destopt(Array **array, FPtr_remove func_remove, const char_t *type)
{
    if (array != NULL)
    {
        if (*array != NULL)
            array_destroy(array, func_remove, type);
    }
}

/*---------------------------------------------------------------------------*/

void array_destroy_ptr(Array **array, FPtr_destroy func_destroy, const char_t *type)
{
    cassert_no_null(array);
    cassert_no_null(*array);
    if (func_destroy != NULL)
        i_destroy_elems(dcast((*array)->data, void), (*array)->elems, func_destroy);
    i_destroy(array, type);
}

/*---------------------------------------------------------------------------*/

void array_destopt_ptr(Array **array, FPtr_destroy func_destroy, const char_t *type)
{
    if (array != NULL)
    {
        if (*array != NULL)
            array_destroy_ptr(array, func_destroy, type);
    }
}

/*---------------------------------------------------------------------------*/

static void i_clear(Array *array)
{
    cassert_no_null(array);
    if (array->nallocs != i_MINIMUN_ARRAY_SIZE)
    {
        uint32_t n_free_bytes = array->nallocs * array->esize;
        uint32_t n_alloc_bytes = i_MINIMUN_ARRAY_SIZE * array->esize;
        array->data = heap_realloc(array->data, n_free_bytes, n_alloc_bytes, "ArrayData");
        array->nallocs = i_MINIMUN_ARRAY_SIZE;
    }

    array->elems = 0;
}

/*---------------------------------------------------------------------------*/

void array_clear(Array *array, FPtr_remove func_remove)
{
    cassert_no_null(array);
    if (func_remove != NULL)
        i_remove_elems(array->data, array->esize, array->elems, func_remove);
    i_clear(array);
}

/*---------------------------------------------------------------------------*/

void array_clear_ptr(Array *array, FPtr_destroy func_destroy)
{
    cassert_no_null(array);
    if (func_destroy != NULL)
        i_destroy_elems(dcast(array->data, void), array->elems, func_destroy);
    i_clear(array);
}

/*---------------------------------------------------------------------------*/

void array_write(Stream *stream, const Array *array, FPtr_write func_write)
{
    const byte_t *elem = NULL;
    uint32_t i = 0;
    cassert_no_null(array);
    cassert_no_nullf(func_write);
    elem = array->data;
    stm_write_u32(stream, array->elems);
    for (i = 0; i < array->elems; ++i, elem += array->esize)
        func_write(stream, cast(elem, void));
}

/*---------------------------------------------------------------------------*/

void array_write_ex(Stream *stream, const Array *array, FPtr_write_ex func_write, const void *data)
{
    const byte_t *elem = NULL;
    uint32_t i = 0;
    cassert_no_null(array);
    cassert_no_nullf(func_write);
    elem = array->data;
    stm_write_u32(stream, array->elems);
    for (i = 0; i < array->elems; ++i, elem += array->esize)
        func_write(stream, cast(elem, void), data);
}

/*---------------------------------------------------------------------------*/

void array_write_ptr(Stream *stream, const Array *array, FPtr_write func_write)
{
    const byte_t **elem = NULL;
    uint32_t i = 0;
    cassert_no_null(array);
    cassert_no_nullf(func_write);
    cassert(array->esize == sizeofptr);
    elem = dcast_const(array->data, byte_t);
    stm_write_u32(stream, array->elems);
    for (i = 0; i < array->elems; ++i, ++elem)
    {
        if (*elem != NULL)
        {
            stm_write_bool(stream, TRUE);
            func_write(stream, *dcast_const(elem, void));
        }
        else
        {
            stm_write_bool(stream, FALSE);
        }
    }
}

/*---------------------------------------------------------------------------*/

void array_write_ptr_ex(Stream *stream, const Array *array, FPtr_write_ex func_write, const void *data)
{
    const byte_t **elem = NULL;
    uint32_t i = 0;
    cassert_no_null(array);
    cassert_no_nullf(func_write);
    cassert(array->esize == sizeofptr);
    elem = dcast_const(array->data, byte_t);
    stm_write_u32(stream, array->elems);
    for (i = 0; i < array->elems; ++i, ++elem)
    {
        if (*elem != NULL)
        {
            stm_write_bool(stream, TRUE);
            func_write(stream, *dcast_const(elem, void), data);
        }
        else
        {
            stm_write_bool(stream, FALSE);
        }
    }
}

/*---------------------------------------------------------------------------*/

uint32_t array_size(const Array *array)
{
    cassert_no_null(array);
    return array->elems;
}

/*---------------------------------------------------------------------------*/

uint32_t array_esize(const Array *array)
{
    cassert_no_null(array);
    return array->esize;
}

/*---------------------------------------------------------------------------*/

static void i_grow_array(
    uint32_t *nallocs,
    uint32_t *elems,
    byte_t **data,
    const uint32_t esize,
    const uint32_t elems_grown)
{
    uint32_t num_new_allocs;
    cassert_no_null(nallocs);
    cassert_no_null(elems);
    cassert_no_null(data);
    cassert(*nallocs >= *elems);
    *elems += elems_grown;
    num_new_allocs = i_next_pow2(*elems);
    if (num_new_allocs < i_MINIMUN_ARRAY_SIZE)
        num_new_allocs = i_MINIMUN_ARRAY_SIZE;

    if (num_new_allocs > *nallocs)
    {
        uint32_t n_free_bytes = *nallocs * esize;
        uint32_t n_alloc_bytes = num_new_allocs * esize;
        cassert(n_free_bytes < n_alloc_bytes);
        *data = heap_realloc(*data, n_free_bytes, n_alloc_bytes, "ArrayData");
        *nallocs = num_new_allocs;
    }
}

/*---------------------------------------------------------------------------*/

static void i_shrink_array(
    uint32_t *nallocs,
    uint32_t *elems,
    byte_t **data,
    const uint32_t esize,
    const uint32_t elems_shrunk)
{
    uint32_t num_new_allocs;
    cassert_no_null(nallocs);
    cassert_no_null(elems);
    cassert_no_null(data);
    cassert(*nallocs >= *elems);
    cassert(*elems >= elems_shrunk);
    *elems -= elems_shrunk;
    num_new_allocs = i_next_pow2(*elems);
    if (num_new_allocs < i_MINIMUN_ARRAY_SIZE)
        num_new_allocs = i_MINIMUN_ARRAY_SIZE;

    if (num_new_allocs < *nallocs)
    {
        uint32_t n_free_bytes = *nallocs * esize;
        uint32_t n_alloc_bytes = num_new_allocs * esize;
        cassert(n_free_bytes > n_alloc_bytes);
        *data = heap_realloc(*data, n_free_bytes, n_alloc_bytes, "ArrayData");
        *nallocs = num_new_allocs;
    }
}

/*---------------------------------------------------------------------------*/

byte_t *array_get(const Array *array, const uint32_t pos)
{
    cassert_no_null(array);
    cassert_msg(pos < array->elems, "Array invalid index");
    return array->data + pos * array->esize;
}

/*---------------------------------------------------------------------------*/

byte_t *array_get_last(const Array *array)
{
    cassert_no_null(array);
    cassert(array->elems > 0);
    return array->data + (array->elems - 1) * array->esize;
}

/*---------------------------------------------------------------------------*/

byte_t *array_all(const Array *array)
{
    cassert_no_null(array);
    return array->elems > 0 ? array->data : NULL;
}

/*---------------------------------------------------------------------------*/

byte_t *array_insert(Array *array, const uint32_t pos, const uint32_t n)
{
    uint32_t celem, cpos;
    cassert_no_null(array);
    celem = array->elems;
    cpos = (pos == UINT32_MAX) ? celem : pos;
    cassert_msg(cpos <= array->elems, "Array invalid index");
    cassert(n > 0);

    i_grow_array(&array->nallocs, &array->elems, &array->data, array->esize, n);

    if (cpos < celem)
        bmem_move(array->data + (cpos + n) * array->esize, array->data + cpos * array->esize, (celem - cpos) * array->esize);

    return array->data + cpos * array->esize;
}

/*---------------------------------------------------------------------------*/

byte_t *array_insert0(Array *array, const uint32_t pos, const uint32_t n)
{
    byte_t *data = array_insert(array, pos, n);
    bmem_set_zero(data, n * array->esize);
    return data;
}

/*---------------------------------------------------------------------------*/

void array_join(Array *dest, const Array *src, FPtr_scopy func_copy)
{
    cassert_no_null(dest);
    cassert_no_null(src);
    cassert(dest->esize == src->esize);

    if (src->elems > 0)
    {
        uint32_t celem = dest->elems;
        byte_t *bdest = NULL;
        const byte_t *bsrc = src->data;

        i_grow_array(&dest->nallocs, &dest->elems, &dest->data, dest->esize, src->elems);
        cassert(dest->elems == celem + src->elems);
        bdest = dest->data + (celem * dest->esize);

        if (func_copy != NULL)
        {
            uint32_t i = 0;
            for (i = 0; i < src->elems; ++i, bdest += dest->esize, bsrc += src->esize)
                func_copy(cast(bdest, void), cast_const(bsrc, void));
        }
        else
        {
            bmem_copy(bdest, bsrc, src->elems * src->esize);
        }
    }
}

/*---------------------------------------------------------------------------*/

void array_join_ptr(Array *dest, const Array *src, FPtr_copy func_copy)
{
    cassert_no_null(dest);
    cassert_no_null(src);
    cassert(dest->esize == src->esize);

    if (src->elems > 0)
    {
        uint32_t celem = dest->elems;
        byte_t *bdest = NULL;
        const byte_t *bsrc = src->data;

        i_grow_array(&dest->nallocs, &dest->elems, &dest->data, dest->esize, src->elems);
        cassert(dest->elems == celem + src->elems);
        bdest = dest->data + (celem * dest->esize);

        if (func_copy != NULL)
        {
            uint32_t i = 0;
            for (i = 0; i < src->elems; ++i, bdest += dest->esize, bsrc += src->esize)
            {
                void *elem = func_copy(*dcast_const(bsrc, void));
                *dcast(bdest, void) = elem;
            }
        }
        else
        {
            bmem_copy(bdest, bsrc, src->elems * src->esize);
        }
    }
}

/*---------------------------------------------------------------------------*/

static void i_delete_elems(uint32_t *nallocs, uint32_t *elems, byte_t **data, const uint32_t esize, const uint32_t pos, const uint32_t num_deletes)
{
    cassert_no_null(elems);
    cassert_no_null(data);
    cassert(num_deletes > 0);
    cassert(pos + num_deletes <= *elems);

    if (pos + num_deletes < *elems)
    {
        uint32_t elems_moved = *elems - (pos + num_deletes);
        bmem_move(PARAM(dest, *data + pos * esize), PARAM(src, *data + (pos + num_deletes) * esize), PARAM(num_bytes, elems_moved * esize));
    }

    i_shrink_array(nallocs, elems, data, esize, num_deletes);
}

/*---------------------------------------------------------------------------*/

void array_delete(Array *array, const uint32_t pos, const uint32_t n, FPtr_remove func_remove)
{
    cassert_no_null(array);
    cassert(n > 0);
    cassert(pos + n <= array->elems);
    if (func_remove != NULL)
    {
        byte_t *data = array->data + pos * array->esize;
        uint32_t i;
        for (i = 0; i < n; ++i)
        {
            func_remove(data);
            data += array->esize;
        }
    }

    i_delete_elems(&array->nallocs, &array->elems, &array->data, array->esize, pos, n);
}

/*---------------------------------------------------------------------------*/

void array_delete_ptr(Array *array, const uint32_t pos, const uint32_t n, FPtr_destroy func_destroy)
{
    cassert_no_null(array);
    cassert(n > 0);
    cassert(pos + n <= array->elems);
    cassert(array->esize == sizeofptr);
    if (func_destroy != NULL)
    {
        byte_t *data = array->data + pos * array->esize;
        uint32_t i;
        for (i = 0; i < n; ++i)
        {
            void **ldata = dcast(data, void);
            cassert_no_null(ldata);
            if (*ldata != NULL)
                func_destroy(ldata);
            data += array->esize;
        }
    }

    i_delete_elems(&array->nallocs, &array->elems, &array->data, array->esize, pos, n);
}

/*---------------------------------------------------------------------------*/

void array_pop(Array *array, FPtr_remove func_remove)
{
    cassert_no_null(array);
    cassert(array->elems > 0);
    if (func_remove != NULL)
    {
        byte_t *data = array->data + (array->elems - 1) * array->esize;
        func_remove(data);
    }

    i_delete_elems(&array->nallocs, &array->elems, &array->data, array->esize, array->elems - 1, 1);
}

/*---------------------------------------------------------------------------*/

void array_pop_ptr(Array *array, FPtr_destroy func_destroy)
{
    cassert_no_null(array);
    cassert(array->elems > 0);
    cassert(array->esize == sizeofptr);
    if (func_destroy != NULL)
    {
        byte_t *data = array->data + (array->elems - 1) * array->esize;
        void **ldata = dcast(data, void);
        cassert_no_null(ldata);
        if (*ldata != NULL)
            func_destroy(ldata);
    }

    i_delete_elems(&array->nallocs, &array->elems, &array->data, array->esize, array->elems - 1, 1);
}

/*---------------------------------------------------------------------------*/

void array_sort(Array *array, FPtr_compare func_compare)
{
    cassert_no_null(array);
    blib_qsort(array->data, array->elems, array->esize, func_compare);
}

/*---------------------------------------------------------------------------*/

void array_sort_ex(Array *array, FPtr_compare_ex func_compare, void *data)
{
    cassert_no_null(array);
    blib_qsort_ex(array->data, array->elems, array->esize, func_compare, cast_const(data, byte_t));
}

/*---------------------------------------------------------------------------*/

typedef struct i_compare_dptr
{
    FPtr_compare func_compare;
    FPtr_compare_ex func_compare_ex;
    void *data;
} i_CompareDPtr;

/*---------------------------------------------------------------------------*/

static int i_compare_ptr(const void **elem1, const void **elem2, i_CompareDPtr *cmp)
{
    cassert_no_null(elem1);
    cassert_no_null(elem2);
    cassert_no_null(cmp);
    if (cmp->func_compare != NULL)
        return cmp->func_compare(*elem1, *elem2);
    else
        return cmp->func_compare_ex(*elem1, *elem2, cmp->data);
}

/*---------------------------------------------------------------------------*/

void array_sort_ptr(Array *array, FPtr_compare func_compare)
{
    i_CompareDPtr cmp;
    cassert_no_null(array);
    cmp.func_compare = func_compare;
    cmp.func_compare_ex = NULL;
    cmp.data = NULL;
    blib_qsort_ex(array->data, array->elems, array->esize, (FPtr_compare_ex)i_compare_ptr, cast_const(&cmp, byte_t));
}

/*---------------------------------------------------------------------------*/

void array_sort_ptr_ex(Array *array, FPtr_compare_ex func_compare, void *data)
{
    i_CompareDPtr cmp;
    cassert_no_null(array);
    cmp.func_compare = NULL;
    cmp.func_compare_ex = func_compare;
    cmp.data = data;
    blib_qsort_ex(array->data, array->elems, array->esize, (FPtr_compare_ex)i_compare_ptr, cast_const(&cmp, byte_t));
}

/*---------------------------------------------------------------------------*/

uint32_t array_find_ptr(const Array *array, const void *elem)
{
    const void **data;
    uint32_t i;
    cassert_no_null(array);
    cassert(array->esize == sizeofptr);
    cassert_no_null(elem);
    data = dcast_const(array->data, void);
    for (i = 0; i < array->elems; ++i, ++data)
    {
        if (*data == elem)
            return i;
    }

    return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

byte_t *array_search(const Array *array, FPtr_compare func_compare, const void *key, uint32_t *pos)
{
    byte_t *data = NULL;
    uint32_t i, n, s;
    cassert_no_null(array);
    cassert_no_nullf(func_compare);
    data = array->data;
    n = array->elems;
    s = array->esize;
    for (i = 0; i < n; ++i, data += s)
    {
        if (func_compare(cast_const(data, void), key) == 0)
        {
            ptr_assign(pos, i);
            return array->data + i * array->esize;
        }
    }

    ptr_assign(pos, UINT32_MAX);
    return NULL;
}

/*---------------------------------------------------------------------------*/

byte_t *array_search_ptr(const Array *array, FPtr_compare func_compare, const void *key, uint32_t *pos)
{
    const void **data = NULL;
    uint32_t i;
    cassert_no_null(array);
    cassert_no_nullf(func_compare);
    data = dcast_const(array->data, void);
    for (i = 0; i < array->elems; ++i, ++data)
    {
        if (func_compare(*data, key) == 0)
        {
            ptr_assign(pos, i);
            return *dcast(data, byte_t);
        }
    }

    ptr_assign(pos, UINT32_MAX);
    return NULL;
}

/*---------------------------------------------------------------------------*/

byte_t *array_bsearch(const Array *array, FPtr_compare func_compare, const void *key, uint32_t *pos)
{
    uint32_t i;
    cassert_no_null(array);
    if (blib_bsearch(array->data, cast_const(key, byte_t), array->elems, array->esize, func_compare, &i) == TRUE)
    {
        ptr_assign(pos, i);
        return array->data + i * array->esize;
    }
    else
    {
        ptr_assign(pos, i);
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

typedef struct i_compare_key
{
    FPtr_compare func_compare;
} i_CompareKey;

/*---------------------------------------------------------------------------*/

static int i_compare_dkey(const void *elem, const void *key, i_CompareKey *cmp)
{
    return cmp->func_compare(*dcast(elem, void), key);
}

/*---------------------------------------------------------------------------*/

byte_t *array_bsearch_ptr(const Array *array, FPtr_compare func_compare, const void *key, uint32_t *pos)
{
    i_CompareKey cmp;
    uint32_t i;
    cassert_no_null(array);
    cmp.func_compare = func_compare;
    if (blib_bsearch_ex(array->data, cast_const(key, byte_t), array->elems, array->esize, (FPtr_compare_ex)i_compare_dkey, cast_const(&cmp, byte_t), &i) == TRUE)
    {
        byte_t **data = dcast(array->data, byte_t) + i;
        ptr_assign(pos, i);
        return *data;
    }
    else
    {
        ptr_assign(pos, i);
        return NULL;
    }
}
