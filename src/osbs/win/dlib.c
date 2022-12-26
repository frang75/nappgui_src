/*
 * NAppGUI Cross-platform C SDK
 * 2015-2022 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: dlib.c
 *
 */

/* Dynamic library loading */

#include "dlib.h"
#include "osbs.inl"
#include "blib.h"
#include "bmem.h"
#include "cassert.h"
#include "unicode.h"

#if !defined(__WINDOWS__)
#error This file is for Windows system
#endif

#include "nowarn.hxx"
#include <Windows.h>
#include "warn.hxx"

/*---------------------------------------------------------------------------*/

DLib *dlib_open(const char_t *path, const char_t *libname)
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

    blib_strcat(pathname, sizeof(pathname), libname);

    num_bytes = unicode_convers(pathname, (char_t*)pathnamew, ekUTF8, ekUTF16, sizeof(pathnamew));
    if (num_bytes < sizeof(pathnamew))
    {
        HMODULE lib = LoadLibrary(pathnamew);
        if (lib != NULL)
        {
            _osbs_dlib_alloc();
            return (DLib*)lib;
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
    cassert_no_null(dlib);
    cassert_no_null(procname);
    return (FPtr_libproc)GetProcAddress((HMODULE)dlib, procname);
}

/*---------------------------------------------------------------------------*/

void* dlib_var_imp(DLib *dlib, const char_t *varname)
{
    cassert_no_null(dlib);
    cassert_no_null(varname);
#pragma warning(push, 0)
#pragma warning(disable:4064)
    return (void*)GetProcAddress((HMODULE)dlib, varname);
#pragma warning(pop)
}
