/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: sewer.c
 *
 */

/* Sewer library */

#include "sewer.h"
#include "sewer.inl"
#include "cassert.h"
#include "bmath.inl"
#include "bmem.h"
#include "bmem.inl"
#include "nowarn.hxx"
#include <stdlib.h>
#include "warn.hxx"

typedef struct _exit_node_t ExitNode;

struct _exit_node_t
{
    void(*func_exit)(void);
    ExitNode *next_node;
};

static ExitNode *i_ATEXIT = NULL;
static uint32_t i_NUM_USERS = 0;

/*---------------------------------------------------------------------------*/

static void i_atexit(void)
{
    ExitNode *node = i_ATEXIT;
    while (node != NULL)
    {
        ExitNode *next;
        next = node->next_node;
        node->func_exit();
        bmem_free((byte_t*)node);
        node = next;
    }

    _bmem_atexit();
}

/*---------------------------------------------------------------------------*/

static void i_sewer_atexit(void)
{
    cassert(i_NUM_USERS == 0);
}

/*---------------------------------------------------------------------------*/

void sewer_start(void)
{
    if (i_NUM_USERS == 0)
    {
        _bmem_start();
        cassert(i_ATEXIT == NULL);
        i_ATEXIT = (ExitNode*)bmem_malloc(sizeof(ExitNode)); 
        i_ATEXIT->func_exit = i_sewer_atexit;
        i_ATEXIT->next_node = NULL;
        atexit(i_atexit);
    }
    
    i_NUM_USERS += 1;
}

/*---------------------------------------------------------------------------*/

void sewer_finish(void)
{
    cassert(i_NUM_USERS > 0);
    if (i_NUM_USERS == 1)
    {
        _bmath_finish();
        _bmem_finish();
        i_NUM_USERS = 0;
    }
    else
    {
        i_NUM_USERS -= 1;
    }
}

/*---------------------------------------------------------------------------*/

void _sewer_atexit(void (*func)(void))
{
    ExitNode *next = i_ATEXIT;
    cassert(i_ATEXIT != NULL);
    i_ATEXIT = (ExitNode*)bmem_malloc(sizeof(ExitNode)); 
    i_ATEXIT->func_exit = func;
    i_ATEXIT->next_node = next;
}
