/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osbs.cpp
 *
 */

/* Operating System Basic Services */

#include "osbs.h"
#include "osbs.inl"
#include "sewer.h"
#include "bmath.h"
#include "bmem.h"
#include "bmem.inl"
#include "bmutex.h"
#include "log.inl"
#include "cassert.h"
#include "log.h"

/*---------------------------------------------------------------------------*/

class OsBs
{
public:
    static uint32_t NUM_USERS;
    Mutex *i_MUTEX;
    uint32_t num_barriers_alloc;
    uint32_t num_barriers_dealloc;
    uint32_t num_directories_opened;
    uint32_t num_directories_closed;
    uint32_t num_files_opened;
    uint32_t num_files_closed;
    uint32_t num_mutex_alloc;
    uint32_t num_mutex_dealloc;
    uint32_t num_procs_alloc;
    uint32_t num_procs_dealloc;
    uint32_t num_threads_alloc;
    uint32_t num_threads_dealloc;
    uint32_t num_sockets_alloc;
    uint32_t num_sockets_dealloc;

    ~OsBs()
    {
        if (NUM_USERS != 0)
        {
            //_osbs_finish();
            //log_printf("Error! osbs is not properly closed (%d)\n", NUM_USERS);
        }
    }
};

static OsBs i_OSBS;
uint32_t OsBs::NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

void _osbs_mutex(Mutex *mutex)
{
    i_OSBS.i_MUTEX = mutex;
}

/*---------------------------------------------------------------------------*/

void osbs_start(void)
{
    if (i_OSBS.NUM_USERS == 0)
    {
        cassert(sizeof(bool_t) == 1);
        cassert(sizeof(byte_t) == 1);
        cassert(sizeof(char_t) == 1);
        cassert(sizeof(uint8_t) == 1);
        cassert(sizeof(uint16_t) == 2);
        cassert(sizeof(uint32_t) == 4);
        cassert(sizeof(uint64_t) == 8);
        cassert(sizeof(enum_t) == 4);
        cassert(sizeof(real32_t) == 4);
        cassert(sizeof(real64_t) == 8);
        cassert((1 == 1) == TRUE);
        cassert((1 != 1) == FALSE);
        #if defined (__x86__)
        cassert(sizeof(void*) == 4);
        #elif defined (__x64__)
        cassert(sizeof(void*) == 8);
        #elif defined (__ARM__)
        cassert(sizeof(void*) == 4);
        #elif defined (__ARM64__)
        cassert(sizeof(void*) == 8);
        #endif

        bmem_zero(&i_OSBS, OsBs);
        i_OSBS.NUM_USERS = 1;
        sewer_start();
        _log_start();
        _bmem_start();
        _osbs_start_sockets();
    }
    else
    {
        i_OSBS.NUM_USERS += 1;
    }
}

/*---------------------------------------------------------------------------*/

void osbs_finish(void)
{
    cassert(i_OSBS.NUM_USERS > 0);
    if (i_OSBS.NUM_USERS == 1)
    {
        _osbs_finish_sockets();
        sewer_finish();
        _log_finish();
        _bmem_finish();

        i_OSBS.NUM_USERS = 0;

        if (i_OSBS.num_barriers_alloc != i_OSBS.num_barriers_dealloc)
            log_printf("Non-dealloc Barriers: %u/%u", i_OSBS.num_barriers_alloc, i_OSBS.num_barriers_dealloc);

        if (i_OSBS.num_directories_opened != i_OSBS.num_directories_closed)
            log_printf("Non-closed Directories: %u/%u", i_OSBS.num_directories_opened, i_OSBS.num_directories_closed);

        if (i_OSBS.num_files_opened != i_OSBS.num_files_closed)
            log_printf("Non-closed Files: %u/%u", i_OSBS.num_files_opened, i_OSBS.num_files_closed);

        if (i_OSBS.num_mutex_alloc != i_OSBS.num_mutex_dealloc)
            log_printf("Non-dealloc Mutex: %u/%u", i_OSBS.num_mutex_alloc, i_OSBS.num_mutex_dealloc);

        if (i_OSBS.num_procs_alloc != i_OSBS.num_procs_dealloc)
            log_printf("Non-dealloc Procs: %u/%u", i_OSBS.num_procs_alloc, i_OSBS.num_procs_dealloc);

        if (i_OSBS.num_threads_alloc != i_OSBS.num_threads_dealloc)
            log_printf("Non-dealloc Threads: %u/%u", i_OSBS.num_threads_alloc, i_OSBS.num_threads_dealloc);

        if (i_OSBS.num_sockets_alloc != i_OSBS.num_sockets_dealloc)
            log_printf("Non-closed Sockets: %u/%u", i_OSBS.num_sockets_alloc, i_OSBS.num_sockets_dealloc);
    }
    else
    {
        i_OSBS.NUM_USERS -= 1;
    }
}

/*---------------------------------------------------------------------------*/

static __INLINE void i_incr(uint32_t *value)
{
    //cassert(i_OSBS.NUM_USERS > 0);
    if (i_OSBS.i_MUTEX != NULL)
    {
        bmutex_lock(i_OSBS.i_MUTEX);
        *value += 1;
        bmutex_unlock(i_OSBS.i_MUTEX);
    }
    else
    {
        *value += 1;
    }
}

/*---------------------------------------------------------------------------*/

void _osbs_directory_alloc(void)
{
    i_incr(&i_OSBS.num_directories_opened);
}

/*---------------------------------------------------------------------------*/

void _osbs_file_alloc(void)
{
    i_incr(&i_OSBS.num_files_opened);
}

/*---------------------------------------------------------------------------*/

void _osbs_mutex_alloc(void)
{
    i_incr(&i_OSBS.num_mutex_alloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_proc_alloc(void)
{
    i_incr(&i_OSBS.num_procs_alloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_thread_alloc(void)
{
    i_incr(&i_OSBS.num_threads_alloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_socket_alloc(void)
{
    i_incr(&i_OSBS.num_sockets_alloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_directory_dealloc(void)
{
    i_incr(&i_OSBS.num_directories_closed);
}

/*---------------------------------------------------------------------------*/

void _osbs_file_dealloc(void)
{
    i_incr(&i_OSBS.num_files_closed);
}

/*---------------------------------------------------------------------------*/

void _osbs_mutex_dealloc(void)
{
    i_incr(&i_OSBS.num_mutex_dealloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_proc_dealloc(void)
{
    i_incr(&i_OSBS.num_procs_dealloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_thread_dealloc(void)
{
    i_incr(&i_OSBS.num_threads_dealloc);
}

/*---------------------------------------------------------------------------*/

void _osbs_socket_dealloc(void)
{
    i_incr(&i_OSBS.num_sockets_dealloc);
}

