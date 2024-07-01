/*
 * NAppGUI Cross-platform C SDK
 * 2015-2024 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: bstdimp.c
 *
 */

/* Basic standard functions */

#include "bstd.h"
#include "blib.h"
#include "bmem.h"
#include "cassert.h"
#include "ptr.h"
#include "unicode.h"

#if !defined(__WINDOWS__)
#error This file is for Windows system
#endif

#include "nowarn.hxx"
#include <Windows.h>
#include <stdio.h>
#include "warn.hxx"

/*---------------------------------------------------------------------------*/

uint32_t bstd_sprintf(char_t *str, const uint32_t size, const char_t *format, ...)
{
    int length;
    va_list args;
    va_start(args, format);
#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
    /* vsnprintf_s doesn't work for str = NULL (for counting chars only) */
    length = vsnprintf(cast(str, char), (size_t)size, cast_const(format, char), args);
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
    va_end(args);
    cassert(length >= 0);
    return (uint32_t)length;
}

/*---------------------------------------------------------------------------*/

uint32_t bstd_vsprintf(char_t *str, const uint32_t size, const char_t *format, va_list args)
{
    int length;
    cassert_no_null(format);
#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
    /* vsnprintf_s doesn't work for str = NULL (for counting chars only) */
    length = vsnprintf(cast(str, char), (size_t)size, cast_const(format, char), args);
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
    cassert(length >= 0);
    return (uint32_t)length;
}

/*---------------------------------------------------------------------------*/

uint32_t bstd_printf(const char_t *format, ...)
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    int length;
    char sbuffer[1024];
    char *dbuffer = NULL;
    char *buffer = NULL;
    DWORD lwsize;

    {
        va_list args;
        va_start(args, format);
#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
        /* vsnprintf_s doesn't work for str = NULL (for counting chars only) */
        length = vsnprintf(NULL, 0, format, args);
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
        va_end(args);
    }

    if (length < 1024)
    {
        buffer = sbuffer;
    }
    else
    {
        dbuffer = cast(bmem_malloc(length + 1), char);
        buffer = dbuffer;
    }

    {
        va_list args;
        va_start(args, format);
        length = vsprintf_s(buffer, length + 1, format, args);
        va_end(args);
    }

    WriteFile(handle, (LPCVOID)buffer, (DWORD)length, &lwsize, NULL);
    unref(lwsize);

#if defined(_MSC_VER)
    if (length < 1024)
    {
        WCHAR wbuffer[1024];
        unicode_convers(cast_const(buffer, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, sizeof(wbuffer));
        OutputDebugString(wbuffer);
    }
    else
    {
        WCHAR *wbuffer = cast(bmem_malloc((length + 1) * sizeof(WCHAR)), WCHAR);
        unicode_convers(cast_const(buffer, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, (length + 1) * sizeof(WCHAR));
        OutputDebugString(wbuffer);
        bmem_free(cast(wbuffer, byte_t));
    }
#endif

    if (dbuffer != NULL)
        bmem_free(cast(dbuffer, byte_t));

    return (uint32_t)length;
}

/*---------------------------------------------------------------------------*/

uint32_t bstd_eprintf(const char_t *format, ...)
{
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    int length;
    char sbuffer[1024];
    char *dbuffer = NULL;
    char *buffer = NULL;
    DWORD lwsize;

    {
        va_list args;
        va_start(args, format);

#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#endif
        /* vsnprintf_s doesn't work for str = NULL (for counting chars only) */
        length = vsnprintf(NULL, 0, format, args);
#if defined(_MSC_VER)
#pragma warning(default : 4996)
#endif
        va_end(args);
    }

    if (length < 1024)
    {
        buffer = sbuffer;
    }
    else
    {
        dbuffer = cast(bmem_malloc(length + 1), char);
        buffer = dbuffer;
    }

    {
        va_list args;
        va_start(args, format);
        length = vsprintf_s(buffer, length + 1, format, args);
        va_end(args);
    }

    WriteFile(handle, (LPCVOID)buffer, (DWORD)length, &lwsize, NULL);
    unref(lwsize);

    if (length < 1024)
    {
        WCHAR wbuffer[1024];
        unicode_convers(cast_const(buffer, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, sizeof(wbuffer));
        OutputDebugString(wbuffer);
    }
    else
    {
        WCHAR *wbuffer = cast(bmem_malloc((length + 1) * sizeof(WCHAR)), WCHAR);
        unicode_convers(cast_const(buffer, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, (length + 1) * sizeof(WCHAR));
        OutputDebugString(wbuffer);
        bmem_free(cast(wbuffer, byte_t));
    }

    if (dbuffer != NULL)
        bmem_free(cast(dbuffer, byte_t));

    return (uint32_t)length;
}

/*---------------------------------------------------------------------------*/

uint32_t bstd_writef(const char_t *str)
{
    size_t size = strlen(str);
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD lwsize;
    WriteFile(handle, (LPCVOID)str, (DWORD)size, &lwsize, NULL);
    unref(lwsize);

    if (size < 1024)
    {
        WCHAR wbuffer[1024];
        unicode_convers(cast_const(str, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, sizeof(wbuffer));
        OutputDebugString(wbuffer);
    }
    else
    {
        WCHAR *wbuffer = cast(bmem_malloc((uint32_t)(size + 1) * sizeof(WCHAR)), WCHAR);
        unicode_convers(cast_const(str, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, (uint32_t)(size + 1) * sizeof(WCHAR));
        OutputDebugString(wbuffer);
        bmem_free(cast(wbuffer, byte_t));
    }

    return (uint32_t)size;
}

/*---------------------------------------------------------------------------*/

uint32_t bstd_ewritef(const char_t *str)
{
    size_t size = strlen(str);
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    DWORD lwsize;
    WriteFile(handle, (LPCVOID)str, (DWORD)size, &lwsize, NULL);
    unref(lwsize);
    return (uint32_t)size;
}

/*---------------------------------------------------------------------------*/

bool_t bstd_read(byte_t *data, const uint32_t size, uint32_t *rsize)
{
    DWORD lrsize;
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    BOOL ok = ReadFile(handle, (LPVOID)data, (DWORD)size, &lrsize, NULL);
    ptr_assign(rsize, (uint32_t)lrsize);
    return (bool_t)ok;
}

/*---------------------------------------------------------------------------*/

bool_t bstd_write(const byte_t *data, const uint32_t size, uint32_t *wsize)
{
    DWORD lwsize;
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    BOOL ok = WriteFile(handle, (LPCVOID)data, (DWORD)size, &lwsize, NULL);

    ptr_assign(wsize, (uint32_t)size);
    unref(ok);

#if defined __DEBUG__
    {
        unicode_t format = ENUM_MAX(unicode_t);
        if (unicode_valid_str_n(cast_const(data, char_t), size, ekUTF8) == TRUE)
            format = ekUTF8;

        if (format != ENUM_MAX(unicode_t))
        {
            if (size < 1024)
            {
                WCHAR wbuffer[1024];
                uint32_t n = unicode_convers_n(cast_const(data, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, size, sizeof(wbuffer));
                cassert(n % sizeof(WCHAR) == 0);
                OutputDebugString(wbuffer);
            }
            else
            {
                WCHAR *wbuffer = cast(bmem_malloc((size + 1) * sizeof(WCHAR)), WCHAR);
                unicode_convers(cast_const(data, char_t), cast(wbuffer, char_t), ekUTF8, ekUTF16, (size + 1) * sizeof(WCHAR));
                OutputDebugString(wbuffer);
                bmem_free(cast(wbuffer, byte_t));
            }
        }
    }
#endif

    return (bool_t)TRUE;
}

/*---------------------------------------------------------------------------*/

bool_t bstd_ewrite(const byte_t *data, const uint32_t size, uint32_t *wsize)
{
    DWORD lwsize;
    HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
    BOOL ok = WriteFile(handle, (LPCVOID)data, (DWORD)size, &lwsize, NULL);
    ptr_assign(wsize, (uint32_t)lwsize);
    return (bool_t)ok;
}

/*---------------------------------------------------------------------------*/

void blib_debug_break(void)
{
#if defined(_MSC_VER) && _MSC_VER > 1400
    if (IsDebuggerPresent() != 0)
        DebugBreak();
#else
    DebugBreak();
#endif
}
