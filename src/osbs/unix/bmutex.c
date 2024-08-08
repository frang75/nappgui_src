/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bmutex.c
 *
 */

/* Basic synchronization services */

#include "bmutex.h"
#include "osbs.inl"
#include <sewer/cassert.h>

#if !defined(__UNIX__)
#error This file is for Unix/Unix-like system
#endif

#include <stdlib.h>
#include <pthread.h>

/*---------------------------------------------------------------------------*/

Mutex *bmutex_create(void)
{
    pthread_mutex_t *mutex = cast(malloc(sizeof(pthread_mutex_t)), pthread_mutex_t);
    int ret = pthread_mutex_init(mutex, NULL);
    cassert_unref(ret == 0, ret);
    _osbs_mutex_alloc();
    return cast(mutex, Mutex);
}

/*---------------------------------------------------------------------------*/

void bmutex_close(Mutex **mutex)
{
    void *mem = NULL;
    int ret;
    cassert_no_null(mutex);
    cassert_no_null(*mutex);
    mem = *dcast(mutex, void);
    ret = pthread_mutex_destroy(cast(*mutex, pthread_mutex_t));
    cassert_unref(ret == 0, ret);
    free(mem);
    _osbs_mutex_dealloc();
    *mutex = NULL;
}

/*---------------------------------------------------------------------------*/

void bmutex_lock(Mutex *mutex)
{
    int ret = 0;
    cassert_no_null(mutex);
    ret = pthread_mutex_lock(cast(mutex, pthread_mutex_t));
    cassert_unref(ret == 0, ret);
}

/*---------------------------------------------------------------------------*/

void bmutex_unlock(Mutex *mutex)
{
    int ret = 0;
    cassert_no_null(mutex);
    ret = pthread_mutex_unlock(cast(mutex, pthread_mutex_t));
    cassert_unref(ret == 0, ret);
}

/*---------------------------------------------------------------------------*/
