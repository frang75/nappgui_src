/*
 * NAppGUI Cross-platform C SDK
 * 2015-2021 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: arrst.hpp
 *
 */

/* Arrays of structures */

#ifndef __ARRST_HPP__
#define __ARRST_HPP__

#include "array.h"
#include "bstd.h"
#include "nowarn.hxx"
#include <typeinfo>
#include "warn.hxx"

template<class type>
struct ArrSt
{
	static ArrSt<type>* create(void);

	static ArrSt<type>* copy(const ArrSt<type> *array, void(*func_copy)(type*, const type));

	static ArrSt<type>* read(Stream *stm, void(*func_read)(Stream*, type*));

	static void destroy(ArrSt<type> **array, void(*func_remove)(type*));

	static void clear(ArrSt<type> *array, void(*func_remove)(type*));

	static void write(Stream *stm, const ArrSt<type> *array, void(*func_write)(Stream*, const type*));

	static uint32_t size(const ArrSt<type> *array);
		
	static type* get(ArrSt<type> *array, const uint32_t pos);

	static const type* get(const ArrSt<type> *array, const uint32_t pos);

	static type* first(ArrSt<type> *array);

	static type* last(ArrSt<type> *array);

	static type* all(ArrSt<type> *array);

	static const type* all(const ArrSt<type> *array);

	static void grow(ArrSt<type> *array, const uint32_t n);

	static type* nnew(ArrSt<type> *array);

	static type* new0(ArrSt<type> *array);

	static type* new_n(ArrSt<type> *array, const uint32_t n);

	static type* new_n0(ArrSt<type> *array, const uint32_t n);

	static type* prepend_n(ArrSt<type> *array, const uint32_t n);

	static type* insert_n(ArrSt<type> *array, const uint32_t pos, const uint32_t n);

	static void append(ArrSt<type> *array, const type value);

	static void prepend(ArrSt<type> *array, const type value);

	static void insert(ArrSt<type> *array, const uint32_t pos, const type value);

	static void join(ArrSt<type> *dest, const ArrSt<type> *src, void(*func_copy)(type*, const type));

	static void ddelete(ArrSt<type> *array, const uint32_t pos, void(*func_remove)(type*));

	static void pop(ArrSt<type> *array, void(*func_remove)(type*));

	static void sort(ArrSt<type> *array, int(*func_compare)(const type*, const type*));

#if defined __ASSERTS__
	// Only for debuggers inspector (non used)
	template<class ttype>
	struct TypeData
	{
		ttype elem[1024];
	};

	uint32_t reserved;
    uint32_t nsize;
    uint16_t elem_sizeof;
    TypeData<type> *content;
#endif
};

template<typename type, typename dtype>
struct ArrS2
{
	static void sort_ex(ArrSt<type> *array, int(*func_compare)(const type*, const type*, const dtype*), const dtype *data);

	static type* search(const ArrSt<type> *array, int(*func_compare)(const type*, const type*), const dtype *key, uint32_t *pos);

	static type* bsearch(const ArrSt<type> *array, int(*func_compare)(const type*, const type*), const dtype *key, uint32_t *pos);
};

/*---------------------------------------------------------------------------*/

template<typename type> 
static const char_t* i_arrtype(void)
{
	static char_t dtype[64];
	bstd_sprintf(dtype, sizeof(dtype), "ArrSt<%s>", typeid(type).name());
	return dtype;
}

/*---------------------------------------------------------------------------*/

template<typename type> 
ArrSt<type>* ArrSt<type>::create(void)
{
	return (ArrSt<type>*)array_create_imp(sizeof(type), i_arrtype<type>());
}

/*---------------------------------------------------------------------------*/

template<typename type> 
ArrSt<type>* ArrSt<type>::copy(const ArrSt<type> *array, void(*func_copy)(type*, const type))
{
    return (ArrSt<type>*)array_copy_imp((Array*)array, (FPtr_scopy)func_copy, i_arrtype<type>());
}

/*---------------------------------------------------------------------------*/

template<typename type> 
ArrSt<type>* ArrSt<type>::read(Stream *stm, void(*func_read)(Stream*, type*))
{
	return (ArrSt<type>*)array_read_imp(stm, sizeof(type), (FPtr_read_init)func_read, i_arrtype<type>());
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::destroy(ArrSt<type> **array, void(*func_remove)(type*))
{
    array_destroy_imp((Array**)array, (FPtr_remove)func_remove, i_arrtype<type>());
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::clear(ArrSt<type> *array, void(*func_remove)(type*))
{
    array_clear_imp((Array*)array, (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::write(Stream *stm, const ArrSt<type> *array, void(*func_write)(Stream*, const type*))
{
    array_write_imp(stm, (const Array*)array, (FPtr_write)func_write);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
uint32_t ArrSt<type>::size(const ArrSt<type> *array)
{
	return array_size_imp((Array*)array);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::get(ArrSt<type> *array, const uint32_t pos)
{
    return (type*)array_get_imp((Array*)array, pos);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
const type* ArrSt<type>::get(const ArrSt<type> *array, const uint32_t pos)
{
    return (type*)array_get_imp((Array*)array, pos);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::first(ArrSt<type> *array)
{
    return (type*)array_get_imp((Array*)array, 0);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::last(ArrSt<type> *array)
{
    return (type*)array_get_last_imp((Array*)array);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::all(ArrSt<type> *array)
{
    return (type*)array_all_imp((Array*)array);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
const type* ArrSt<type>::all(const ArrSt<type> *array)
{
    return (type*)array_all_imp((Array*)array);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::grow(ArrSt<type> *array, const uint32_t n)
{
	array_insert_imp((Array*)array, UINT32_MAX, n);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::nnew(ArrSt<type> *array)
{
    return (type*)array_insert_imp((Array*)array, UINT32_MAX, 1);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::new0(ArrSt<type> *array)
{
    return (type*)array_insert0_imp((Array*)array, UINT32_MAX, 1);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::new_n(ArrSt<type> *array, const uint32_t n)
{
    return (type*)array_insert_imp((Array*)array, UINT32_MAX, n);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::new_n0(ArrSt<type> *array, const uint32_t n)
{
    return (type*)array_insert0_imp((Array*)array, UINT32_MAX, n);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::prepend_n(ArrSt<type> *array, const uint32_t n)
{
    return (type*)array_insert_imp((Array*)array, 0, n);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
type* ArrSt<type>::insert_n(ArrSt<type> *array, const uint32_t pos, const uint32_t n)
{
    return (type*)array_insert_imp((Array*)array, pos, n);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::append(ArrSt<type> *array, const type value)
{
	*(type*)array_insert_imp((Array*)array, UINT32_MAX, 1) = value;
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::prepend(ArrSt<type> *array, const type value)
{
	*(type*)array_insert_imp((Array*)array, 0, 1) = value;
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::insert(ArrSt<type> *array, const uint32_t pos, const type value)
{
	*(type*)array_insert_imp((Array*)array, pos, 1) = value;
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::join(ArrSt<type> *dest, const ArrSt<type> *src, void(*func_copy)(type*, const type))
{
    array_join_imp((Array*)(dest), (const Array*)src, (FPtr_scopy)func_copy);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::ddelete(ArrSt<type> *array, const uint32_t pos, void(*func_remove)(type*))
{
    array_delete_imp((Array*)array, pos, 1, (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::pop(ArrSt<type> *array, void(*func_remove)(type*))
{
    array_pop_imp((Array*)array, (FPtr_remove)func_remove);
}

/*---------------------------------------------------------------------------*/

template<typename type> 
void ArrSt<type>::sort(ArrSt<type> *array, int(*func_compare)(const type*, const type*))
{
    array_sort_imp((Array*)array, (FPtr_compare)func_compare);
}

/*---------------------------------------------------------------------------*/

template<typename type, typename dtype>
void ArrS2<type,dtype>::sort_ex(ArrSt<type> *array, int(*func_compare)(const type*, const type*, const dtype*), const dtype *data)
{
    array_sort_ex_imp((Array*)array, (FPtr_compare_ex)func_compare, (void*)data);
}

/*---------------------------------------------------------------------------*/

template<typename type, typename dtype>
type* ArrS2<type,dtype>::search(const ArrSt<type> *array, int(*func_compare)(const type*, const type*), const dtype *key, uint32_t *pos)
{
    return (type*)array_search_imp((const Array*)array, (FPtr_compare)func_compare, (const void*)key, pos);
}

/*---------------------------------------------------------------------------*/

template<typename type, typename dtype>
type* ArrS2<type,dtype>::bsearch(const ArrSt<type> *array, int(*func_compare)(const type*, const type*), const dtype *key, uint32_t *pos)
{
    return (type*)array_bsearch_imp((const Array*)array, (FPtr_compare)func_compare, (const void*)key, pos);
}

#endif
