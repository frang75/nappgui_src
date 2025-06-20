/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: encode.c
 *
 */

/* encode library */

#include "encode.h"
#include <core/core.h>
#include <osbs/log.h>
#include <sewer/blib.h>
#include <sewer/cassert.h>

static uint32_t i_NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

static void i_encode_atexit(void)
{
    if (i_NUM_USERS != 0)
        log_printf("Error! encode is not properly closed (%d)\n", i_NUM_USERS);
}

/*---------------------------------------------------------------------------*/

void encode_start(void)
{
    if (i_NUM_USERS == 0)
    {
        core_start();
        blib_atexit(i_encode_atexit);
    }

    i_NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void encode_finish(void)
{
    cassert(i_NUM_USERS > 0);
    if (i_NUM_USERS == 1)
    {
        core_finish();
    }

    i_NUM_USERS -= 1;
}
