/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: sinfo.m
 *
 */

/* Machine info */

#include "../bfile.h"
#include "../osbs.h"

#if !defined(__MACOS__)
#error This file is only for OSX
#endif

#include <sewer/nowarn.hxx>
#include <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>
#include <sewer/warn.hxx>

#include <sewer/cassert.h>
#include <sewer/unicode.h>

/*---------------------------------------------------------------------------*/

uint32_t bfile_dir_home(char_t *pathname, const uint32_t size)
{
    const char *home = getenv("HOME");
    return unicode_convers(cast_const(home, char_t), pathname, ekUTF8, ekUTF8, size);
}

/*---------------------------------------------------------------------------*/

uint32_t bfile_dir_data(char_t *pathname, const uint32_t size)
{
    uint32_t s;
    const char *home = getenv("HOME");
    s = unicode_convers(cast_const(home, char_t), pathname, ekUTF8, ekUTF8, size);
    if (s + 8 < size)
    {
        strcpy(pathname + s - 1, "/Library");
        s += 8;
    }
    return s;
}

/*---------------------------------------------------------------------------*/

uint32_t bfile_dir_exec(char_t *pathname, const uint32_t size)
{
    uint32_t lsize = size;
    if (_NSGetExecutablePath(cast(pathname, char), &lsize) != 0)
    {
        cassert_msg(FALSE, "Buffer too small");
    }

    return lsize;
}

/*---------------------------------------------------------------------------*/

macos_t osbs_macos(void)
{
#if defined(MAC_OS_VERSION_26_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_26_0
    return ekMACOS_TAHOE;
#elif defined(MAC_OS_VERSION_15_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_15_0
    return ekMACOS_SEQUOIA;
#elif defined(MAC_OS_VERSION_14_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_14_0
    return ekMACOS_SONOMA;
#elif defined(MAC_OS_VERSION_13_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_13_0
    return ekMACOS_VENTURA;
#elif defined(MAC_OS_VERSION_12_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_12_0
    return ekMACOS_MONTEREY;
#elif defined(MAC_OS_VERSION_11_0) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_11_0
    return ekMACOS_BIGSUR;
#elif defined(MAC_OS_X_VERSION_10_15) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
    return ekMACOS_CATALINA;
#elif defined(MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14
    return ekMACOS_MOJAVE;
#elif defined(MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
    return ekMACOS_HIGHSIERRA;
#elif defined(MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
    return ekMACOS_SIERRA;
#elif defined(MAC_OS_X_VERSION_10_11) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_11
    return ekMACOS_ELCAPITAN;
#elif defined(MAC_OS_X_VERSION_10_10) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
    return ekMACOS_YOSEMITE;
#elif defined(MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9
    return ekMACOS_MAVERICKS;
#elif defined(MAC_OS_X_VERSION_10_8) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_8
    return ekMACOS_MOUNTAIN_LION;
#elif defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
    return ekMACOS_LION;
#elif defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
    return ekMACOS_SNOW_LEOPARD;
#else
    return ekMACOS_LEGACY;
#endif
}
