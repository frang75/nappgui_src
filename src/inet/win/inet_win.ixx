/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: inet_win.ixx
 *
 */

/* Inet Windows */

#include "../inet.hxx"
#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <sewer/warn.hxx>

#define STATIC_TEXT_SIZE 1024
typedef struct _wstring WString;

struct _wstring
{
    uint32_t nchars;
    WCHAR statictext[STATIC_TEXT_SIZE];
    WCHAR *alloctext;
};
