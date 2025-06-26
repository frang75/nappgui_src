/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bthread.c
 *
 */

/* Basic threading services */

#include "../bthread.h"
#include "../osbs.inl"
#include <sewer/cassert.h>

#if !defined(__UNIX__)
#error This file is for Unix/Unix-like system
#endif

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#if defined __LINUX__
#include <sys/syscall.h>
long syscall(long number, ...);
#endif

int pthread_tryjoin_np(pthread_t thread, void **retval);

/*---------------------------------------------------------------------------*/

Thread *bthread_create_imp(uint32_t(func_thread_main)(void *), void *data)
{
    pthread_t *thread = cast(malloc(sizeof(pthread_t)), pthread_t);
#if defined(__GNUC__)
#if (__GNUC__ >= 8)
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
#endif
    int ret = pthread_create(thread, NULL, cast_func(func_thread_main, void *(*)(void *)), data);
#if defined(__GNUC__)
#if (__GNUC__ >= 8)
#pragma GCC diagnostic warning "-Wcast-function-type"
#endif
#endif

    if (ret != 0)
    {
        free(cast(thread, void));
        return NULL;
    }
    else
    {
        _osbs_thread_alloc();
        return cast(thread, Thread);
    }
}

/*---------------------------------------------------------------------------*/

int bthread_current_id(void)
{
#if defined __LINUX__
#ifdef SYS_gettid
    return (int)syscall(SYS_gettid);
#else
#error "SYS_gettid unavailable on this system"
    return 0;
#endif
#else
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (int)tid;
#endif
}

/*---------------------------------------------------------------------------*/

void bthread_close(Thread **thread)
{
    void *mem = NULL;
    cassert_no_null(thread);
    cassert_no_null(*thread);
    mem = *dcast(thread, void);
    free(mem);
    _osbs_thread_dealloc();
    *thread = NULL;
}

/*---------------------------------------------------------------------------*/

bool_t bthread_cancel(Thread *thread)
{
    int result = 0;
    cassert_no_null(thread);
    result = pthread_cancel(*cast(thread, pthread_t));
    return result == 0 ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*/

uint32_t bthread_wait(Thread *thread)
{
    void *value = NULL;
    int result = 0;
    cassert_no_null(thread);
    result = pthread_join(*cast(thread, pthread_t), &value);
    if (result == 0)
        return (uint32_t)(intptr_t)value;
    else
        return UINT32_MAX;
}

/*---------------------------------------------------------------------------*/

bool_t bthread_finish(Thread *thread, uint32_t *code)
{
#if defined(__LINUX__)
    void *value = NULL;
    int result = 0;
    cassert_no_null(thread);
    result = pthread_tryjoin_np(*cast(thread, pthread_t), &value);
    if (result == 0)
    {
        if (code != NULL)
            *code = (uint32_t)(intptr_t)value;
        return TRUE;
    }
    else
    {
        cassert(result == EBUSY);
        return FALSE;
    }
#elif defined(__MACOS__)
    unref(thread);
    cassert_msg(FALSE, "TODO: Not implemented");
    if (code != NULL)
        *code = 0;
    return FALSE;
#elif defined(__IOS__)
    unref(thread);
    cassert_msg(FALSE, "TODO: Not implemented");
    if (code != NULL)
        *code = 0;
    return FALSE;
#endif
}

/*---------------------------------------------------------------------------*/

void bthread_sleep(const uint32_t milliseconds)
{
    usleep(milliseconds * 1000);
}
