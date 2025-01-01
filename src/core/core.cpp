/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: core.cpp
 *
 */

/* Core library */

#include "core.h"
#include "dbind.inl"
#include "dbindh.h"
#include "heap.inl"
#include "stream.inl"
#include "strings.h"
#include <osbs/osbs.h>
#include <osbs/bproc.h>
#include <osbs/log.h>
#include <sewer/bmem.h>
#include <sewer/blib.h>
#include <sewer/cassert.h>

static uint32_t i_NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

static void i_core_atexit(void)
{
    if (i_NUM_USERS != 0)
        log_printf("Error! core is not properly closed (%d)\n", i_NUM_USERS);
}

/*---------------------------------------------------------------------------*/

static void i_assert_to_log(void *item, const uint32_t group, const char_t *caption, const char_t *detail, const char_t *file, const uint32_t line)
{
    unref(item);
    unref(group);
    if (group == 0)
    {
        log_printf("FATAL ASSERT: %s (%s:%d): %s", caption, file, line, detail);
        bproc_exit(1000);
    }
    else
    {
        log_printf("ASSERT: %s (%s:%d): %s", caption, file, line, detail);
    }
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrpt_create(const char_t *type, const uint16_t esize)
{
    char_t atype[128] = ARRPT;
    unref(esize);
    str_cat_c(atype, sizeof(atype), type);
    return cast(array_create(sizeof(void *), atype), byte_t);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_arrpt_size(const byte_t *arrpt)
{
    return array_size(cast(arrpt, Array));
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrpt_get(byte_t *arrpt, const uint32_t pos, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_get(cast(arrpt, Array), pos);
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrpt_insert(byte_t *arrpt, const uint32_t pos, const uint32_t n, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_insert(cast(arrpt, Array), pos, n);
}

/*---------------------------------------------------------------------------*/

static void i_arrpt_delete(byte_t *arrpt, const uint32_t pos, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_delete(cast(arrpt, Array), pos, 1, NULL);
}

/*---------------------------------------------------------------------------*/

static void i_arrpt_destroy(byte_t **arrpt, const char_t *type)
{
    char_t atype[128] = ARRPT;
    str_cat_c(atype, sizeof(atype), type);
    array_destroy_ptr(dcast(arrpt, Array), NULL, atype);
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrst_create(const char_t *type, const uint16_t esize)
{
    char_t atype[128] = ARRST;
    str_cat_c(atype, sizeof(atype), type);
    return cast(array_create(esize, atype), byte_t);
}

/*---------------------------------------------------------------------------*/

static uint32_t i_arrst_size(const byte_t *arrst)
{
    return array_size(cast(arrst, Array));
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrst_get(byte_t *arrst, const uint32_t pos, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_get(cast(arrst, Array), pos);
}

/*---------------------------------------------------------------------------*/

static byte_t *i_arrst_insert(byte_t *arrpt, const uint32_t pos, const uint32_t n, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_insert(cast(arrpt, Array), pos, n);
}

/*---------------------------------------------------------------------------*/

static void i_arrst_delete(byte_t *arrpt, const uint32_t pos, const char_t *type, const uint16_t esize)
{
    unref(type);
    unref(esize);
    return array_delete(cast(arrpt, Array), pos, 1, NULL);
}

/*---------------------------------------------------------------------------*/

static void i_arrst_destroy(byte_t **arrst, const char_t *type)
{
    char_t atype[128] = ARRST;
    str_cat_c(atype, sizeof(atype), type);
    array_destroy_ptr(dcast(arrst, Array), NULL, atype);
}

/*---------------------------------------------------------------------------*/

void core_start(void)
{
    if (i_NUM_USERS == 0)
    {
        osbs_start();
        _heap_start();
        _stm_start();
        _dbind_start();
        cassert_set_func(NULL, i_assert_to_log);
        blib_atexit(i_core_atexit);

        /* Register DBind NAppGUI types */
        dbind_bool(bool_t);
        dbind_int(int8_t, TRUE);
        dbind_int(int16_t, TRUE);
        dbind_int(int32_t, TRUE);
        dbind_int(int64_t, TRUE);
        dbind_int(uint8_t, FALSE);
        dbind_int(uint16_t, FALSE);
        dbind_int(uint32_t, FALSE);
        dbind_int(uint64_t, FALSE);
        dbind_real(real32_t);
        dbind_real(real64_t);
        dbind_string(String, str_c, str_destroy, tc, str_read, str_write, "");
        dbind_container("ArrPt", TRUE, i_arrpt_create, i_arrpt_size, i_arrpt_get, i_arrpt_insert, i_arrpt_delete, i_arrpt_destroy);
        dbind_container("ArrSt", FALSE, i_arrst_create, i_arrst_size, i_arrst_get, i_arrst_insert, i_arrst_delete, i_arrst_destroy);
        i_NUM_USERS = 1;
#if defined(_MSC_VER)
        cassert(sizeof(EventHandler) == sizeofptr);
#else
        cassert(sizeof(EventHandler) == 2 * sizeofptr);
#endif
    }
    else
    {
        i_NUM_USERS += 1;
    }
}

/*---------------------------------------------------------------------------*/

void core_finish(void)
{
    cassert(i_NUM_USERS > 0);
    if (i_NUM_USERS == 1)
    {
        i_NUM_USERS = 0;
        _dbind_finish();
        _stm_finish();
        _heap_finish();
        osbs_finish();
    }
    else
    {
        i_NUM_USERS -= 1;
    }
}
