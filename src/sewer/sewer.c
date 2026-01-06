/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
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
#include "bstd.h"
#include "nowarn.hxx"
#include <stdlib.h>
#include "warn.hxx"

typedef struct _exit_node_t ExitNode;

struct _exit_node_t
{
    void (*func_exit)(void);
    ExitNode *next_node;
};

static ExitNode *i_ATEXIT = NULL;
static uint32_t i_NUM_USERS = 0;
static char_t i_VERSION[64];

/*---------------------------------------------------------------------------*/

static void i_atexit(void)
{
    ExitNode *node = i_ATEXIT;
    while (node != NULL)
    {
        ExitNode *next = node->next_node;
        node->func_exit();
        bmem_free(cast(node, byte_t));
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
        i_ATEXIT = cast(bmem_malloc(sizeof(ExitNode)), ExitNode);
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

uint32_t sewer_nappgui_major(void)
{
#if defined(NAPPGUI_MAJOR)
    return NAPPGUI_MAJOR;
#else
    return 0;
#endif
}

/*---------------------------------------------------------------------------*/

uint32_t sewer_nappgui_minor(void)
{
#if defined(NAPPGUI_MINOR)
    return NAPPGUI_MINOR;
#else
    return 0;
#endif
}

/*---------------------------------------------------------------------------*/

uint32_t sewer_nappgui_patch(void)
{
#if defined(NAPPGUI_PATCH)
    return NAPPGUI_PATCH;
#else
    return 0;
#endif
}

/*---------------------------------------------------------------------------*/

uint32_t sewer_nappgui_build(void)
{
#if defined(NAPPGUI_BUILD)
    return NAPPGUI_BUILD;
#else
    return 0;
#endif
}

/*---------------------------------------------------------------------------*/

const char_t *sewer_nappgui_version(const bool_t full)
{
    uint32_t major = sewer_nappgui_major();
    uint32_t minor = sewer_nappgui_minor();
    uint32_t patch = sewer_nappgui_patch();
    uint32_t build = sewer_nappgui_build();
    if (major > 0)
    {
        if (build > 0 && full == TRUE)
            bstd_sprintf(i_VERSION, sizeof(i_VERSION), "%d.%d.%d.%d", major, minor, patch, build);
        else
            bstd_sprintf(i_VERSION, sizeof(i_VERSION), "%d.%d.%d", major, minor, patch);
    }
    else
    {
        bstd_sprintf(i_VERSION, sizeof(i_VERSION), "0.0.0");
    }

    return i_VERSION;
}

/*---------------------------------------------------------------------------*/

void _sewer_atexit(void (*func)(void))
{
    ExitNode *next = i_ATEXIT;
    cassert(i_ATEXIT != NULL);
    i_ATEXIT = (ExitNode *)bmem_malloc(sizeof(ExitNode));
    i_ATEXIT->func_exit = func;
    i_ATEXIT->next_node = next;
}

/*#define SEWER_CHECK_COMPILER_WARNINGS*/
#if defined(SEWER_CHECK_COMPILER_WARNINGS)

/*
 * List of warnings that the compiler should detect
 *
 * Unused local variable
 * Unused static variable
 * Unused static function
 * Implicit function declaration (use a function not declared - missing include)
 * Missing declaration (define a global function without a previous declaration)
 * Format string is not a literal (printf)
 * printf wrong arguments
 * Incompatible pointer types
 * Signed/Unsigned comparison
 * float/integer conversion
 * Use variable without initialization
 * Control reaches end of non-void function
 * Enumeration value not handled in switch
 * Wrong pointer in function argument
 *
 * -Wenum-int-mismatch
 */

/* Unused static variable */
static uint32_t i_NON_USED_STATIC = 0;

typedef struct _str1_t Str1;
typedef struct _str2_t Str2;

struct _str1_t
{
    uint32_t data1;
};

struct _str2_t
{
    real32_t data1;
    real64_t data2;
};

/*---------------------------------------------------------------------------*/

/* Unused static function */
static uint32_t i_unused_static_function(void)
{
    uint32_t i = 0;
    uint32_t t = 0;
    for (i = 0; i < 100; ++i)
        t += i;
    return t;
}

/*---------------------------------------------------------------------------*/

/* Missing declaration */
uint32_t sewer_warns(void)
{
    uint32_t i;
    int32_t k = 0;
    /* Unused local variable */
    uint32_t j = 0;
    uint32_t l;
    const char_t *format = "%s";
    /* Use variable without initialization */
    for (; i < 100; ++i)
    {
        /* Format string is not a literal (printf) */
        bstd_printf(format, "Hello");

        /* printf wrong arguments */
        bstd_printf("%s %d", "Hello", "Hello");
        bstd_printf("%s %d", 3, 3);
        bstd_printf("%s %d", "Hello");
        bstd_printf("%s %d", 3);
    }

    /* Implicit function declaration */
    unicode_nchars("Hello", ekUTF8);

    /* Signed/Unsigned comparison */
    if (k < i)
    {
        bstd_printf("%s", "Bye");
    }

    /* Control reaches end of non-void function */
    if (i < 200)
        return 6;
}

/*---------------------------------------------------------------------------*/

uint32_t sewer_warns2(const unicode_t code);
uint32_t sewer_warns2(const unicode_t code)
{
    /* Enumeration value not handled in switch */
    switch (code)
    {
    case ekUTF8:
        return 1;
    case ekUTF16:
        return 2;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

uint32_t sewer_warns3(const Str1 *str);
uint32_t sewer_warns3(const Str1 *str)
{
    /* Incompatible pointer types */
    const Str2 *str2 = str;
    Str1 *str3 = str;
    return (uint32_t)str2 + (uint32_t)str3;
}

/*---------------------------------------------------------------------------*/

/* Wrong pointer in function argument */
uint32_t sewer_warns4(void);
uint32_t sewer_warns4(void)
{
    Str2 str;
    return sewer_warns3(&str);
}

#endif
