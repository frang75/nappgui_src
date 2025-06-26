/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dlib.c
 *
 */

/* Dynamic library loading */

#include "../osbs.inl"
#include "../dlib.h"
#include <sewer/blib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>
#include <sewer/unicode.h>

#if !defined(__WINDOWS__)
#error This file is for Windows system
#endif

#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <sewer/warn.hxx>

/*---------------------------------------------------------------------------*/

static DLib *i_open(const char_t *path, const char_t *prefix, const char_t *libname)
{
    char_t pathname[MAX_PATH + 1];
    WCHAR pathnamew[MAX_PATH + 1];
    uint32_t num_bytes = 0;

    pathname[0] = '\0';

    if (path != NULL)
    {
        blib_strcat(pathname, sizeof(pathname), path);
        blib_strcat(pathname, sizeof(pathname), "\\");
    }

    blib_strcat(pathname, sizeof(pathname), prefix);
    blib_strcat(pathname, sizeof(pathname), libname);

    num_bytes = unicode_convers(pathname, cast(pathnamew, char_t), ekUTF8, ekUTF16, sizeof(pathnamew));
    if (num_bytes < sizeof(pathnamew))
    {
        HMODULE lib = LoadLibrary(pathnamew);
        if (lib != NULL)
        {
            _osbs_dlib_alloc();
            return cast(lib, DLib);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

DLib *dlib_open(const char_t *path, const char_t *libname)
{
    DLib *lib = i_open(path, "", libname);
    /* MinGW generate DLLs with lib prefix */
    if (lib == NULL)
        lib = i_open(path, "lib", libname);
    return lib;
}

/*---------------------------------------------------------------------------*/

void dlib_close(DLib **dlib)
{
    BOOL ok = 0;
    cassert_no_null(dlib);
    cassert_no_null(*dlib);
    ok = FreeLibrary((HMODULE)*dlib);
    *dlib = NULL;
    _osbs_dlib_dealloc();
    cassert_unref(ok != 0, ok);
}

/*---------------------------------------------------------------------------*/

FPtr_libproc dlib_proc_imp(DLib *dlib, const char_t *procname)
{
    FARPROC func = NULL;
    cassert_no_null(dlib);
    cassert_no_null(procname);
    func = GetProcAddress((HMODULE)dlib, procname);
    return cast_func(func, FPtr_libproc);
}

/*---------------------------------------------------------------------------*/

void *dlib_var_imp(DLib *dlib, const char_t *varname)
{
    cassert_no_null(dlib);
    cassert_no_null(varname);
#if defined(_MSC_VER)
#pragma warning(push, 0)
#pragma warning(disable : 4064)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    return cast(GetProcAddress((HMODULE)dlib, varname), void);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic warning "-Wpedantic"
#endif
}
