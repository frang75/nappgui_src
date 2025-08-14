/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmem_win.c
 *
 */

/* Basic memory system */

#include "../bmem.h"
#include "../bmem.inl"
#include "../cassert.h"

#if !defined(__WINDOWS__)
#error This file is for Windows
#endif

#include "../nowarn.hxx"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#if defined(__MEMORY_AUDITOR__)
#include <stdlib.h>

/* <crtdbg.h> is a Microsoft Visual C++ specific header */
#if defined(_MSC_VER) && _MSC_VER > 1400
#if !defined(NAPPGUI_NO_CRTDBG)
#define _WITH_CRTDBG 1
#include <crtdbg.h>
#endif
#endif
#endif

#include "../warn.hxx"

#if defined(__MEMORY_SUBSYTEM_CHECKING__)

#define i_MAX 1000000
static void *i_MEM[i_MAX];
static uint32_t i_INDEX = 0;

/*---------------------------------------------------------------------------*/

static void i_mem_append(void *ptr)
{
    uint32_t i;

    for (i = 0; i < i_INDEX; ++i)
    {
        if (i_MEM[i] == NULL)
        {
            i_MEM[i] = ptr;
            return;
        }
    }

    i_MEM[i_INDEX] = ptr;
    i_INDEX++;
    cassert(i_INDEX < i_MAX);
}

/*---------------------------------------------------------------------------*/

static void i_mem_remove(void *ptr)
{
    uint32_t i;

    for (i = 0; i < i_INDEX; ++i)
    {
        if (i_MEM[i] == ptr)
        {
            i_MEM[i] = NULL;
            return;
        }
    }

    cassert(FALSE);
}

/*---------------------------------------------------------------------------*/

static bool_t i_mem_is_empty(void)
{
    uint32_t i;

    for (i = 0; i < i_INDEX; ++i)
    {
        if (i_MEM[i] != NULL)
            return FALSE;
    }

    return TRUE;
}

#endif

/*---------------------------------------------------------------------------*/

static HANDLE i_HEAP = NULL;

/*---------------------------------------------------------------------------*/

void _bmem_start(void)
{
    cassert_unref(i_HEAP == NULL, i_HEAP);
#if defined(__MEMORY_AUDITOR__)
#else
/*i_HEAP = HeapCreate(0, 0, 0);*/
#endif
}

/*---------------------------------------------------------------------------*/

void _bmem_finish(void)
{
#if defined(__MEMORY_SUBSYTEM_CHECKING__)
    cassert(i_mem_is_empty() == TRUE);
#endif
}

/*---------------------------------------------------------------------------*/

void _bmem_atexit(void)
{
#if defined(__MEMORY_AUDITOR__)
#if _WITH_CRTDBG
    _CrtDumpMemoryLeaks();
#endif
#else
    cassert(i_HEAP == NULL);
/*HeapDestroy(i_HEAP);*/
#endif
}

/*---------------------------------------------------------------------------*/

byte_t *bmem_aligned_malloc(const uint32_t size, const uint32_t align)
{
    void *mem = NULL;

#if defined(__MEMORY_AUDITOR__)
#if _WITH_CRTDBG
    mem = _aligned_malloc_dbg((size_t)size, (size_t)align, __FILE__, __LINE__);
#else
    mem = _aligned_malloc((size_t)size, (size_t)align);
#endif
#else
    /*mem = HeapAlloc(i_HEAP, 0, (SIZE_T)size);*/
    mem = _aligned_malloc((size_t)size, (size_t)align);
#endif

#if defined(__MEMORY_SUBSYTEM_CHECKING__)
    i_mem_append(mem);
#endif
    cassert((mem != NULL) && ((intptr_t)mem % align) == 0);
    return cast(mem, byte_t);
}

/*---------------------------------------------------------------------------*/

byte_t *bmem_aligned_realloc(byte_t *mem, const uint32_t size, const uint32_t new_size, const uint32_t align)
{
    void *new_mem = NULL;

    unref(size);

#if defined(__MEMORY_SUBSYTEM_CHECKING__)
    i_mem_remove(mem);
#endif

#if defined(__MEMORY_AUDITOR__)
#if _WITH_CRTDBG
    new_mem = _aligned_realloc_dbg(mem, (size_t)new_size, (size_t)align, __FILE__, __LINE__);
#else
    new_mem = _aligned_realloc(mem, (size_t)new_size, (size_t)align);
#endif
#else
    /* new_mem = HeapReAlloc(i_HEAP, 0, (LPVOID)mem, (SIZE_T)size); */
    new_mem = _aligned_realloc(mem, (size_t)new_size, (size_t)align);
    unref(size);
#endif

#if defined(__MEMORY_SUBSYTEM_CHECKING__)
    i_mem_append(new_mem);
#endif
    cassert((mem != NULL) && ((intptr_t)mem % align) == 0);
    return cast(new_mem, byte_t);
}

/*---------------------------------------------------------------------------*/

void bmem_free(byte_t *mem)
{
#if defined(__MEMORY_SUBSYTEM_CHECKING__)
    i_mem_remove(mem);
#endif

#if defined(__MEMORY_AUDITOR__)
#if _WITH_CRTDBG
    _aligned_free_dbg(mem);
#else
    _aligned_free(mem);
#endif
#else
    /*HeapFree(i_HEAP, 0, (LPVOID)mem);*/
    _aligned_free(mem);
#endif
}

/*---------------------------------------------------------------------------*/

void bmem_set1(byte_t *dest, const uint32_t size, const byte_t mask)
{
    cassert_no_null(dest);
    cassert(size > 0);
    memset(cast(dest, void), (int)mask, (size_t)size);
}

/*---------------------------------------------------------------------------*/

void bmem_set_zero(byte_t *mem, const uint32_t size)
{
    cassert_no_null(mem);
    cassert(size > 0);
    ZeroMemory(cast(mem, void), (size_t)size);
}
