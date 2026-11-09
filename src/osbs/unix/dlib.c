/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://www.nappgui.com/en/legal/license.html
 *
 * File: dlib.c
 *
 */

/* Dynamic library loading */

#include "../dlib.h"
#include "../osbs.inl"
#include <sewer/blib.h>
#include <sewer/bmem.h>
#include <sewer/cassert.h>

#if defined(__LINUX__)
static const char_t *i_LIB_PREFIX = "lib";
static const char_t *i_LIB_SUFIX = ".so";
#elif defined(__MACOS__)
static const char_t *i_LIB_PREFIX = "lib";
static const char_t *i_LIB_SUFIX = ".dylib";
#else
#error Unknown platform
#endif

#include <dlfcn.h>

/*---------------------------------------------------------------------------*/

static void i_libname(char_t *pathname, const uint32_t size, const char_t *libname, const char_t *libvers)
{
    blib_strcat(pathname, size, i_LIB_PREFIX);
    blib_strcat(pathname, size, libname);
    if (libvers != NULL && libvers[0] != '\0')
    {
#if defined(__LINUX__)
        blib_strcat(pathname, size, i_LIB_SUFIX);
        blib_strcat(pathname, size, ".");
        blib_strcat(pathname, size, libvers);
#else
        blib_strcat(pathname, size, ".");
        blib_strcat(pathname, size, libvers);
        blib_strcat(pathname, size, i_LIB_SUFIX);
#endif
    }
    else
    {
        blib_strcat(pathname, size, i_LIB_SUFIX);
    }
}

/*---------------------------------------------------------------------------*/

DLib *dlib_open(const char_t *path, const char_t *libname, const char_t *libvers)
{
    uint32_t size = 0;
    char_t *pathname = NULL;
    void *lib = NULL;

    cassert_no_null(libname);
    size += 2; /* Backslash and null terminator */
    size += blib_strlen(libname);
    size += blib_strlen(i_LIB_PREFIX);
    size += blib_strlen(i_LIB_SUFIX);
    if (libvers != NULL)
        size += blib_strlen(libvers) + 1;

    if (path != NULL && path[0] != '\0')
    {
        uint32_t n = blib_strlen(path);
        size += n;
        pathname = cast(bmem_malloc(size), char_t);
        blib_strcpy(pathname, size, path);
        if (path[n - 1] != '/')
            blib_strcat(pathname, size, "/");
    }
    else
    {
        pathname = cast(bmem_malloc(size), char_t);
        pathname[0] = '\0';
    }

    i_libname(pathname, size, libname, libvers);

    lib = dlopen(pathname, RTLD_LAZY | RTLD_LOCAL);
    bmem_free(cast(pathname, byte_t));
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

/*---------------------------------------------------------------------------*/

void dlib_close(DLib **dlib)
{
    int ok = 0;
    cassert_no_null(dlib);
    cassert_no_null(*dlib);
    ok = dlclose(*dcast(dlib, void));
    *dlib = NULL;
    _osbs_dlib_dealloc();
    cassert_unref(ok == 0, ok);
}

/*---------------------------------------------------------------------------*/

FPtr_libproc dlib_proc_imp(DLib *dlib, const char_t *procname)
{
    void *func = NULL;
    FPtr_libproc *proc = NULL;
    cassert_no_null(dlib);
    cassert_no_null(procname);
    func = dlsym(cast(dlib, void), procname);
    proc = cast(&func, FPtr_libproc);
    return *proc;
}

/*---------------------------------------------------------------------------*/

void *dlib_var_imp(DLib *dlib, const char_t *varname)
{
    cassert_no_null(dlib);
    cassert_no_null(varname);
    return cast(dlsym(cast(dlib, void), varname), void);
}
