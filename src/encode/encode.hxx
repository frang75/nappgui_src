/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: encode.hxx
 * https://nappgui.com/en/encode/encode.html
 *
 */

/* encode library */

#ifndef __ENCODE_HXX__
#define __ENCODE_HXX__

#include <core/core.hxx>
#include "encode.hdf"

typedef struct _url_t Url;
typedef struct _json_t Json;
typedef struct _jsonopts_t JsonOpts;

struct _jsonopts_t
{
    ArrPt(String) *log;
};

#endif
