/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmutex.c
 *
 */

/* Basic synchronization services */

#include "../osbs.inl"
#include "../bmutex.h"
#include <sewer/bmem.h>
#include <sewer/cassert.h>

#if !defined(__WINDOWS__)
#error This file is for Windows system
#endif

#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <sewer/warn.hxx>

/*---------------------------------------------------------------------------*/

Mutex *bmutex_create(void)
{
    SRWLOCK* mutex = cast(bmem_malloc(sizeof(SRWLOCK)), SRWLOCK);
    cassert_no_null(mutex);
    InitializeSRWLock(mutex);
    _osbs_mutex_alloc();
    return cast(mutex, Mutex);
}

/*---------------------------------------------------------------------------*/

void bmutex_close(Mutex **mutex)
{
    cassert_no_null(mutex);
    cassert_no_null(*mutex);
    bmem_free(*mutex);
    _osbs_mutex_dealloc();
    *mutex = NULL;
}

/*---------------------------------------------------------------------------*/

void bmutex_lock(Mutex *mutex)
{
    cassert_no_null(mutex);
    AcquireSRWLockExclusive(cast(mutex, SRWLOCK));
}

/*---------------------------------------------------------------------------*/

void bmutex_unlock(Mutex *mutex)
{
    cassert_no_null(mutex);
    ReleaseSRWLockExclusive(cast(mutex, SRWLOCK));
}
