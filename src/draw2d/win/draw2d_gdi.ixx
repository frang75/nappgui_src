/*
 * NAppGUI Cross-platform C SDK
 * 2015-2025 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: draw2d_gdi.ixx
 *
 */

/* 2D drawing support */

#ifndef __OSDRAW_WIN2_IXX__
#define __OSDRAW_WIN2_IXX__

#include "../draw2d.hxx"
#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <sewer/warn.hxx>

#define STATIC_TEXT_SIZE 1024
typedef struct _wstring WString;

struct _measurestr_t
{
    HDC hdc;
};

struct _wstring
{
    uint32_t nchars;
    WCHAR statictext[STATIC_TEXT_SIZE];
    WCHAR *alloctext;
};

#endif
